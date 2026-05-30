package admiral

import (
	"errors"
	"fmt"
	"os"
	"os/signal"
	"syscall"

	"admiral/internal/broker"
	"admiral/internal/config"
	"admiral/internal/logger"
)

type Application interface {
	Start() error
	Close() error
}

type Admiral struct {
	Config *config.Config
	Broker *broker.Broker
}

func New(config *config.Config) (Application, error) {
	broker := broker.New(config.Services, config.StrictMode)

	return &Admiral{
		Config: config,
		Broker: &broker,
	}, nil
}

func (a *Admiral) Start() error {
	done := make(chan bool, 1)
	errchan := make(chan error, 1)

	// NOTE(laith): 2 listeners to handling shutdowns:
	// 1. interruption, behavior should be to close all connections admiral has.
	// 2. actual program exits
	go func() {
		sigch := make(chan os.Signal, 1)
		signal.Notify(sigch, os.Interrupt, syscall.SIGTERM)
		<-sigch

		done <- true
	}()

	go func() {
		if err := a.Broker.Start(); err != nil {
			errchan <- err
		}

		done <- true
	}()

	<-done

	if err := a.Close(); err != nil {
		return errors.New("error closing application: " + err.Error())
	}

	// NOTE(laith): non blocking channel read
	select {
	case err := <-errchan:
		return err
	default:
		return nil
	}
}

func (a *Admiral) Close() error {
	fmt.Printf("%s Closing Admiral - Message Broker - Version 2\n", logger.LIONS_LOGO_COLORED)
	// Broker.CloseAllConnections

	return nil
}
