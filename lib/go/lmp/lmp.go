/*  lmp.go - LIONS Middleware Protocol
    Copyright (C) 2026 splatte.dev

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>. */

package lmp

import (
	"errors"
)

// TODO(laith): go-ify this code, still too c-pilled

/* [0] Version */
var LmpVersions = []uint8{1, 2}

/* [1] Type */
const (
	LmpTypeInit    = uint8(0x01)
	LmpTypePing    = uint8(0x02)
	LmpTypeSend    = uint8(0x03)
	LmpTypeTerm    = uint8(0x04)
	LmpTypeInvalid = uint8(0x05)
)

/* [2] Argument */
const (
	LmpArgPing            = uint8(0x00)
	LmpArgInitInit        = uint8(0x01)
	LmpArgInitAccept      = uint8(0x02)
	LmpArgSend            = uint8(0x00)
	LmpArgTermClean       = uint8(0x01)
	LmpArgTermBusy        = uint8(0x02)
	LmpArgInvalidVersion  = uint8(0x01)
	LmpArgInvalidType     = uint8(0x02)
	LmpArgInvalidMessage  = uint8(0x03)
	LmpArgInvalidArgument = uint8(0x04)
	LmpArgInvalidFlags    = uint8(0x05)
	LmpArgInvalidPayload  = uint8(0x06)
)

/* [3] Flags */
const (
	LmpFlagsNone      = uint8(0)
	LmpFlagsLog       = uint8(1 << 0)
	LmpFlagsIncognito = uint8(1 << 1)
)

/* [4] Payload */
const LmpPayloadEmpty = uint8(0x00)

/* Packet */
const (
	LmpPacketHeaderSize     = 0x04  // 4
	LmpPacketMaxSize        = 0x5DC // 1500
	LmpPacketMinSize        = 0x05  // 5
	LmpPacketTerminate      = uint8(0x7F)
	LmpPacketPayloadMaxSize = 0x5D7 // 1495
)

type LmpPacket struct {
	Version       uint8
	Type          uint8
	Arg           uint8
	Flags         uint8
	Payload       []byte
	PayloadLength int
}

func LmpPacketInit() LmpPacket {
	return LmpPacket{}
}

func packetArgsAreValid(packetType uint8, arg uint8) bool {
	switch packetType {
	case LmpTypeInit:
		return arg == LmpArgInitInit || arg == LmpArgInitAccept
	case LmpTypePing:
		return arg == LmpArgPing
	case LmpTypeSend:
		return arg == LmpArgSend
	case LmpTypeTerm:
		return arg == LmpArgTermClean || arg == LmpArgTermBusy
	case LmpTypeInvalid:
		return arg >= LmpArgInvalidVersion && arg <= LmpArgInvalidPayload
	}

	return false
}

func (p *LmpPacket) Serialize() ([]byte, error) {
	var bufferSize = LmpPacketHeaderSize + p.PayloadLength + 1

	if bufferSize < LmpPacketMinSize || bufferSize > LmpPacketMaxSize {
		return nil, errors.New("Incorrect packet size")
	}

	var buffer = make([]byte, bufferSize)

	if p.Payload == nil || p.PayloadLength < 1 {
		return buffer, errors.New("Payload must at least contain the empty payload byte")
	}

	version := uint8(0)
	for _, v := range LmpVersions {
		if p.Version == v {
			version = v
			break
		}
	}

	if version == 0 {
		return buffer, errors.New("Incorrect packet version provided")
	}

	if p.Type < LmpTypeInit || p.Type > LmpTypeInvalid {
		return buffer, errors.New("Incorrect packet type provided")
	}

	if !packetArgsAreValid(p.Type, p.Arg) {
		return buffer, errors.New("Incorrect packet arguments provided")
	}

	if p.Type == LmpTypeInit || p.Type == LmpTypeInvalid {
		if !(p.PayloadLength == 1 && p.Payload[0] == LmpPayloadEmpty) {
			return buffer, errors.New("Incorrect payload content for init and invalid packets")
		}
	}

	buffer[0] = version
	buffer[1] = p.Type
	buffer[2] = p.Arg
	buffer[3] = p.Flags
	copy(buffer[LmpPacketHeaderSize:], p.Payload[:p.PayloadLength])
	buffer[LmpPacketHeaderSize+p.PayloadLength] = LmpPacketTerminate

	return buffer, nil
}

func Deserialize(buffer []byte) (LmpPacket, error) {
	packet := LmpPacketInit()

	if buffer == nil {
		return packet, errors.New("Invalid buffer")
	}

	size := len(buffer)
	if size < LmpPacketMinSize || size > LmpPacketMaxSize {
		return packet, errors.New("Buffer has an invalid size")
	}

	version := uint8(0)
	for _, v := range LmpVersions {
		if buffer[0] == v {
			version = buffer[0]
			break
		}
	}

	if version == 0 {
		return packet, errors.New("Incorrect version byte provided")
	}

	if buffer[1] < LmpTypeInit || buffer[1] > LmpTypeInvalid {
		return packet, errors.New("Incorrect type byte provided")
	}

	if !packetArgsAreValid(buffer[1], buffer[2]) {
		return packet, errors.New("Incorrect argument byte provided")
	}

	packet.Version = version
	packet.Type = buffer[1]
	packet.Arg = buffer[2]
	packet.Flags = buffer[3]

	if (buffer[1] == LmpTypeInvalid || buffer[1] == LmpTypeInit) &&
		buffer[LmpPacketHeaderSize] != LmpPayloadEmpty {
		return packet, errors.New("Invalid payload provided")
	}

	if buffer[size-1] != LmpPacketTerminate {
		return packet, errors.New("Payload is not terminated")
	}

	payloadLen := size - LmpPacketHeaderSize - 1
	if payloadLen < 1 {
		return packet, errors.New("Invalid payload provided")
	}

	if (packet.Type == LmpTypeInit || packet.Type == LmpTypeInvalid) && payloadLen != 1 {
		return packet, errors.New("Invalid payload provided")
	}

	if payloadLen == 1 && buffer[LmpPacketHeaderSize] != LmpPayloadEmpty {
		return packet, errors.New("Invalid payload provided")
	}

	packet.Payload = buffer[LmpPacketHeaderSize : LmpPacketHeaderSize+payloadLen]
	packet.PayloadLength = payloadLen

	return packet, nil
}

// NOTE(laith): quick smoke test below

// func main() {
// 	pkt := LmpPacket{}
//
// 	pkt.Version = 2
// 	pkt.Type = LmpTypeSend
// 	pkt.Arg = LmpArgSend
// 	pkt.Flags = LmpFlagsNone
// 	pkt.Payload = []byte("hello")
// 	pkt.PayloadLength = 5
//
// 	buf, err := pkt.Serialize()
// 	if err != nil {
// 		fmt.Println(err)
// 		return
// 	}
//
// 	for i, byte := range buf {
// 		fmt.Println(i, byte)
// 	}
//
// 	pkt2, err := Deserialize(buf)
// 	if err != nil {
// 		fmt.Println(err)
// 		return
// 	}
//
// 	fmt.Println(pkt.Version)
// 	fmt.Println(pkt.Type)
// 	fmt.Println(pkt.Arg)
// 	fmt.Println(pkt.Flags)
// 	fmt.Println(pkt.Payload)
// 	fmt.Println(pkt.PayloadLength)
//
// 	fmt.Println(pkt2.Version)
// 	fmt.Println(pkt2.Type)
// 	fmt.Println(pkt.Arg)
// 	fmt.Println(pkt2.Flags)
// 	fmt.Println(pkt2.Payload)
// 	fmt.Println(pkt2.PayloadLength)
// }
