package config

import (
	"encoding/json"
	"errors"
	"os"
)

type Entry struct {
	Destination int    `json:"destination"`
	Payload     string `json:"payload"`
	Time        string `json:"time"`
	Note        string `json:"note"`
}

type Config struct {
	Entries []Entry `json:"entries"`
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
