package config

import (
	"encoding/json"
	"errors"
	"os"

	"admiral/internal/broker"
)

type Config struct {
	Services []broker.Service `json:"services"`
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
