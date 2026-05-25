package logger

import (
	"admiral/internal/broker"
	"fmt"
)

const LIONS_LOGO = "[LIONS //]"

const (
	LIONS_COLOR = "\x1b[38;5;220m"

	TYPE_COLOR_INFO  = "\x1b[38;5;27m"
	TYPE_COLOR_WARN  = "\x1b[38;5;208m"
	TYPE_COLOR_ERROR = "\x1b[38;5;124m"

	// SERVICE_COLOR_ADMIRAL   = "\x1b[38;5;48m"
	// SERVICE_COLOR_RECEPTION = "\x1b[38;5;93m"
	// SERVICE_COLOR_S2        = "\x1b[38;5;201m"
	// SERVICE_COLOR_GIBSON    = "\x1b[38;5;34m"
	// SERVICE_COLOR_LAITT     = "\x1b[38;5;226m"
	// SERVICE_COLOR_LIGHTCTL  = "\x1b[38;5;213m"

	COLOR_RESET = "\x1b[0m"
)

type LogType int

const (
	Info LogType = iota
	Warn
	Error
)

// [LIONS] (hostname | service) timestamp: message
func Log(sender broker.Service, message string, level LogType) {
	fmt.Printf("%s[%s]%s %s(%s | %s)%s _timestamp_: %s\n", LIONS_COLOR, LIONS_LOGO, COLOR_RESET, sender.Color, sender.Hostname, sender.Name, COLOR_RESET, message)
}
