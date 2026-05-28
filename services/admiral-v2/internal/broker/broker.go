package broker

import (
	"errors"
	"fmt"
	"io"
	"liblmp"
	"lmp"
	"net"

	"admiral/internal/logger"
	"admiral/internal/message"
	"admiral/internal/service"
)

type Connection struct {
	service   service.Service
	appConn   net.Conn
	sonarConn net.Conn
}

type Broker struct {
	Services    map[int]service.Service
	Connections []Connection
	strictMode  bool
	messages    chan message.Message
}

func New(configServices []service.Service, strictMode bool) Broker {
	services := service.CreateServiceMap(configServices)
	return Broker{
		Services:   services,
		strictMode: strictMode,
	}
}

func (b *Broker) Start() error {
	// Listen for all oncoming connections
	listener, err := net.Listen("tcp", ":5321")
	if err != nil {
		return errors.New("error creating tcp listener: " + err.Error())
	}

	fmt.Printf("%s Starting admiral\n", logger.LIONS_LOGO_COLORED)

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

		logger.Log(b.Services[service.Id], "CONNECTED", logger.Info)
		connection := Connection{
			appConn:   conn,
			sonarConn: nil,
			service:   service,
		}

		b.Connections = append(b.Connections, connection)

		go b.serveConnection(connection)
	}
}

func (b *Broker) serveConnection(conn Connection) {
	for {
		packet, err := liblmp.ReadPacket(conn.appConn)
		if err != nil {
			if err == io.EOF {
				logger.Log(conn.service, "DISCONNECTED", logger.Warn)
				conn.appConn.Close()
				break
			} else {
				logger.Log(conn.service, "sent bad packet: "+err.Error(), logger.Error)
				continue
			}
		}

		var sendPacket lmp.LmpPacket
		destinationId := packet.Payload[0]
		fmt.Println("destination ID ", destinationId)

		// Removing the admiral header
		packet.Payload = packet.Payload[2:]
		packet.PayloadLength = packet.PayloadLength - 2

		// ---- Handle all packet type cases ----
		switch packet.Type {
		case lmp.LmpTypePing:
			sendPacket = lmp.LmpPacket{
				Version: 0x02,
				Type:    lmp.LmpTypePing,
				Arg:     lmp.LmpArgPing,
				Flags:   lmp.LmpFlagsNone,
				Payload: []byte{lmp.LmpPayloadEmpty},
			}
		case lmp.LmpTypeSend:
			sendPacket = lmp.LmpPacket{
				Version: 0x02,
				Type:    lmp.LmpTypeSend,
				Arg:     lmp.LmpArgSend,
				Flags:   lmp.LmpFlagsNone,
				Payload: packet.Payload,
			}
		case lmp.LmpTypeTerm:
			// TODO
		}

		b.getServiceConnection(int(destinationId))
		if err := liblmp.SendPacket(conn.appConn, &sendPacket); err != nil {
			// TODO
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
	// is the service already connected with admiral? use that connection
	for _, conn := range b.Connections {
		if conn.service.Id == id {
			return conn.appConn, nil
		}
	}

	// does the service need to connect with admiral
	connection, err := net.Dial("tcp", fmt.Sprintf("%s:%d", b.Services[id].Hostname, b.Services[id].Port))
	if err != nil {
		return nil, errors.New("could not create connection with service")
	}

	return connection, nil
}
