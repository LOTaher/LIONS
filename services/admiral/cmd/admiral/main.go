package main

import (
	"admiral/internal/admiral"
	"admiral/internal/config"
	"os"
)

func main() {
	config, err := config.Read()
	if err != nil {
		panic(err)
	}

	admiral, err := admiral.New(&config)
	if err != nil {
		panic(err)
	}

	if err := admiral.Start(); err != nil {
		os.Exit(1)
	}
}
