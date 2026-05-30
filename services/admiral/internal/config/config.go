package config

import (
	"encoding/json"
	"errors"
	"os"

	"admiral/internal/service"
)

type Config struct {
	Services   []service.Service `json:"services"`   // Array of services
	StrictMode bool              `json:"strictMode"` // Do you want admiral to be strict in verifying the IPs and ports of the connection
}

func Read() (Config, error) {
	var config Config

	content, err := os.ReadFile("config.json")
	if err != nil {
		return config, errors.New("error reading configuration file: " + err.Error())
	}

	err = json.Unmarshal(content, &config)
	if err != nil {
		return config, errors.New("error unmarshalling json: " + err.Error())
	}

	return config, nil
}
