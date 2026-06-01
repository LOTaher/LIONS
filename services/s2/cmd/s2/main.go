package main

import (
	"fmt"
	"net"
	"os"
	"os/signal"
	"s2/internal/config"
	"syscall"
	"time"

	"liblmp"
	"lmp"
)

const (
	ADMIRAL_IP   = "100.113.240.39"
	ADMIRAL_PORT = 5321
)

const LIONS_LOGO = "[LIONS //]"
const LIONS_LOGO_COLORED = LIONS_COLOR + LIONS_LOGO + COLOR_RESET

const (
	LIONS_COLOR          = "\x1b[38;5;220m"
	SERVICE_COLOR_GIBSON = "\x1b[38;5;34m"
	COLOR_RESET          = "\x1b[0m"
)

func main() {
	fmt.Printf("%s Starting S2 - Scheduling Service - Version 2\n", LIONS_LOGO_COLORED)

	done := make(chan bool, 1)

	config, err := config.Read()
	if err != nil {
		panic(err)
	}

	var schedule = make(map[string]lmp.LmpPacket)

	for _, entry := range config.Entries {
		admiralHeader := []byte{byte(entry.Destination), byte(liblmp.S2)}
		packet := lmp.LmpPacket{
			Version:       0x02,
			Type:          lmp.LmpTypeSend,
			Arg:           lmp.LmpArgSend,
			Payload:       append(admiralHeader, []byte(entry.Payload)...),
			PayloadLength: len(entry.Payload) + 2,
		}
		schedule[entry.Time] = packet
	}

	conn, err := net.Dial("tcp", fmt.Sprintf("%s:%d", ADMIRAL_IP, ADMIRAL_PORT))
	if err != nil {
		panic(err)
	}

	liblmp.SendHandshake(conn, int(liblmp.S2))

	go func() {
		sigchan := make(chan os.Signal, 1)
		signal.Notify(sigchan, os.Interrupt, syscall.SIGTERM)
		<-sigchan

		done <- true
	}()

	for {
		if <-done {
			fmt.Printf("%s Closing S2 - Scheduling Service - Version 2\n", LIONS_LOGO_COLORED)
			os.Exit(0)
		}

		currentTime := time.Now().Format(time.TimeOnly)
		fmt.Printf("\r%s === %s ===", LIONS_LOGO_COLORED, currentTime)

		if packet, ok := schedule[currentTime]; ok {
			if err := liblmp.SendPacket(conn, &packet); err != nil {
				fmt.Printf("%s Could not send scheduled packet. Admiral connection broken\n", LIONS_LOGO_COLORED)
			}
		}

		time.Sleep(time.Second)
	}
}
