package broker

import "net"

type Service struct {
	Id       string `json:"id"`
	Name     string `json:"name"`
	Hostname string `json:"hostname"`
	IpAddr   string `json:"ip"`
	Port     string `json:"port"`
	Color    string `json:"color"`
}

type Connection struct {
	service   Service
	appConn   net.TCPConn
	sonarConn net.TCPConn
}

type Broker struct {
	Services    []Service
	Connections []Connection
}

func New(services []Service) Broker {
	return Broker{
		Services: services,
	}
}

func (b *Broker) Start() {
	// Listen for all oncoming connections
}

func (b *Broker) CloseAllConnections()
