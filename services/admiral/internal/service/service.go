package service

type Service struct {
	Id       int    `json:"id"`
	Name     string `json:"name"`
	Hostname string `json:"hostname"`
	IpAddr   string `json:"ip"`
	Port     int    `json:"port"`
	Color    string `json:"color"`
}

func CreateServiceMap(services []Service) map[int]Service {
	var serviceMap = map[int]Service{}
	for _, service := range services {
		serviceMap[service.Id] = service
	}

	return serviceMap
}
