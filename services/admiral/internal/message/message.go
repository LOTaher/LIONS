package message

import (
	"admiral/internal/service"
)

var START_ID = 0

type Message struct {
	id          uint64
	sender      service.Service
	destination service.Service
	payload     []byte
}

func CreateMessage(destination service.Service, sender service.Service, payload []byte) Message {
	return Message{
		id:          uint64(START_ID) + 1,
		sender:      sender,
		destination: destination,
		payload:     payload,
	}
}
