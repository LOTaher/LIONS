package broker

import (
	"errors"
	"fmt"
	"io"
	"liblmp"
	"lmp"
	"net"
	"slices"

	"admiral/internal/logger"
	// "admiral/internal/message"
	"admiral/internal/service"
)

type Connection struct {
	id        uint16
	service   service.Service
	appConn   net.Conn
	sonarConn net.Conn
}

type EventType int

const (
	CONNECTED EventType = iota
	DISCONNECTED
)

type Event struct {
	connection Connection
	eventType  EventType
}

type Broker struct {
	Services    map[int]service.Service
	Connections []Connection
	strictMode  bool
	events      chan Event
}

func New(configServices []service.Service, strictMode bool) Broker {
	services := service.CreateServiceMap(configServices)
	return Broker{
		Services:   services,
		strictMode: strictMode,
		events:     make(chan Event, 8),
	}
}

func (b *Broker) Start() error {
	// Listen for all oncoming connections
	listener, err := net.Listen("tcp", ":5321")
	if err != nil {
		return errors.New("error creating tcp listener: " + err.Error())
	}

	fmt.Printf("%s Starting Admiral - Message Broker - Version 2\n", logger.LIONS_LOGO_COLORED)

	go b.handleEvents()

	for {
		conn, err := listener.Accept()
		if err != nil {
			logger.Log(b.Services[0], "unable to accept connection", logger.Error)
			continue
		}

		service, err := b.handshake(conn)
		if err != nil {
			logger.Log(b.Services[0], "unable to handshake the connection: "+err.Error(), logger.Error)
			conn.Close()
			continue
		}

		connection := Connection{
			appConn:   conn,
			sonarConn: nil,
			service:   service,
		}
		b.events <- Event{connection, CONNECTED}
	}
}

func (b *Broker) handleEvents() {
	for {
		event := <-b.events

		switch event.eventType {
		case CONNECTED:
			logger.Log(b.Services[event.connection.service.Id], "CONNECTED", logger.Info)
			b.Connections = append(b.Connections, event.connection)
			go b.serveConnection(event.connection)
		case DISCONNECTED:
			logger.Log(event.connection.service, "DISCONNECTED", logger.Warn)
			for idx, brokerConn := range b.Connections {
				if event.connection.service.Id == brokerConn.service.Id {
					b.Connections = slices.Delete(b.Connections, idx, idx+1)
					break
				}
			}
			event.connection.appConn.Close()
		}
	}
}

func (b *Broker) serveConnection(conn Connection) {
	for {
		packet, err := liblmp.ReadPacket(conn.appConn)
		if err != nil {
			if err == io.EOF {
				b.events <- Event{conn, DISCONNECTED}
				break
			} else {
				logger.Log(conn.service, "sent bad packet: "+err.Error(), logger.Error)
				continue
			}
		}

		var sendPacket lmp.LmpPacket
		destinationId := packet.Payload[0]

		// Removing the admiral header
		packet.Payload = packet.Payload[2:]
		packet.PayloadLength = packet.PayloadLength - 2

		// ---- Handle all packet type cases ----
		switch packet.Type {
		case lmp.LmpTypePing:
			sendPacket = lmp.LmpPacket{
				Version:       0x02,
				Type:          lmp.LmpTypePing,
				Arg:           lmp.LmpArgPing,
				Flags:         lmp.LmpFlagsNone,
				Payload:       []byte{lmp.LmpPayloadEmpty},
				PayloadLength: 1,
			}
		case lmp.LmpTypeSend:
			sendPacket = lmp.LmpPacket{
				Version:       0x02,
				Type:          lmp.LmpTypeSend,
				Arg:           lmp.LmpArgSend,
				Flags:         lmp.LmpFlagsNone,
				Payload:       packet.Payload,
				PayloadLength: packet.PayloadLength,
			}
		case lmp.LmpTypeTerm:
			// TODO
		}

		serviceConn, err := b.getServiceConnection(int(destinationId))
		if err != nil {
			logger.Log(conn.service, fmt.Sprintf("BAD SERVICE: %s", err.Error()), logger.Error)
			continue
		}

		if err := liblmp.SendPacket(serviceConn, &sendPacket); err != nil {
			logger.Log(conn.service, fmt.Sprintf("BAD PACKET: %s", err.Error()), logger.Error)
			continue
		}

		logMessage := fmt.Sprintf("Type 0x0%x Arg 0x0%x -> %s", packet.Type, packet.Arg, logger.BuildServiceString(b.Services[int(destinationId)]))
		logger.Log(b.Services[conn.service.Id], logMessage, logger.Info)

	}
}

func (b *Broker) handshake(conn net.Conn) (service.Service, error) {
	initPacket, err := liblmp.ReadPacket(conn)
	if err != nil {
		return service.Service{}, errors.New("could not read init packet")
	}

	if initPacket.Type != lmp.LmpTypeInit || initPacket.Arg != lmp.LmpArgInitInit {
		conn.Close()
		return service.Service{}, errors.New("incorrect init packet structure")
	}

	sendPacket, err := liblmp.ReadPacket(conn)
	if err != nil {
		return service.Service{}, errors.New("could not read send packet")
	}

	if sendPacket.Type != lmp.LmpTypeSend || sendPacket.Arg != lmp.LmpArgSend {
		conn.Close()
		return service.Service{}, errors.New("incorrect send packet structure")
	}

	acceptPacket := lmp.LmpPacket{
		Version:       0x02,
		Type:          lmp.LmpTypeInit,
		Arg:           lmp.LmpArgInitAccept,
		Flags:         lmp.LmpFlagsNone,
		Payload:       []byte{lmp.LmpPayloadEmpty},
		PayloadLength: 1,
	}

	if err := liblmp.SendPacket(conn, &acceptPacket); err != nil {
		conn.Close()
		return service.Service{}, errors.New("unable to send accept packet")
	}

	destinationId := sendPacket.Payload[0]
	destinationIsAdmiral := false
	senderId := sendPacket.Payload[1]
	senderIsValid := false

	destinationService, ok := b.Services[int(destinationId)]
	if ok {
		if destinationService.Name == "admiral" {
			destinationIsAdmiral = true
		}
	}

	senderService, ok := b.Services[int(senderId)]
	if ok {
		if b.strictMode {
			addr := conn.RemoteAddr().(*net.TCPAddr)
			ip := addr.IP.String()
			port := addr.Port

			if senderService.IpAddr == ip || senderService.Port == port {
				senderIsValid = true
			}
		} else {
			senderIsValid = true
		}
	}

	if !destinationIsAdmiral {
		return service.Service{}, errors.New("payload destination is not admiral")
	}

	if !senderIsValid {
		return service.Service{}, errors.New("payload sender is invalid")
	}

	return senderService, nil
}

func (b *Broker) getServiceConnection(id int) (net.Conn, error) {
	// is it a valid service
	if _, ok := b.Services[id]; !ok {
		return nil, errors.New(fmt.Sprintf("service with id %d does not exist", id))
	}

	// is the service already connected with admiral? use that connection
	for _, conn := range b.Connections {
		if conn.service.Id == id {
			return conn.appConn, nil
		}
	}

	// does the service need to connect with admiral
	connection, err := net.Dial("tcp", fmt.Sprintf("%s:%d", b.Services[id].IpAddr, b.Services[id].Port))
	if err != nil {
		return nil, errors.New("could not create connection with service")
	}

	return connection, nil
}
