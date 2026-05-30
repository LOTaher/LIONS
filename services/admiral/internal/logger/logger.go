package logger

import (
	"fmt"
	"time"

	"admiral/internal/service"
)

const LIONS_LOGO = "[LIONS //]"
const LIONS_LOGO_COLORED = LIONS_COLOR + LIONS_LOGO + COLOR_RESET

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
func Log(sender service.Service, message string, level LogType) {
	var logColor string
	switch level {
	case Info:
		logColor = TYPE_COLOR_INFO
	case Warn:
		logColor = TYPE_COLOR_WARN
	case Error:
		logColor = TYPE_COLOR_ERROR
	}

	fmt.Printf("%s%s%s %s(%s | %s)%s %s: %s\n", logColor, LIONS_LOGO, COLOR_RESET, sender.Color, sender.Hostname, sender.Name, COLOR_RESET, time.Now().Format(time.TimeOnly), message)
}

func BuildServiceString(service service.Service) string {
	return fmt.Sprintf("%s(%s | %s)%s", service.Color, service.Hostname, service.Name, COLOR_RESET)
}
