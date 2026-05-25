package message

import (
	"liblmp"
	"lmp"
)

var START_ID = 0

type Message struct {
	id          uint64
	sender      *liblmp.AdmiralService
	destination *liblmp.AdmiralService
	packet      lmp.LmpPacket
}

func CreateMessage(destination *liblmp.AdmiralService, sender *liblmp.AdmiralService, packet lmp.LmpPacket) Message {
	return Message{
		id:          uint64(START_ID) + 1,
		sender:      sender,
		destination: destination,
		packet:      packet,
	}
}
