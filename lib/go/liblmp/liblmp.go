/*  liblmp.go - Utilities for the LIONS Middleware Protocol
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

package liblmp

import (
	"errors"
	"io"
	"net"

	"lmp"
)

type AdmiralService byte

const (
	Admiral AdmiralService = iota
	Reception
	S2
	Gibson
	Laitt
	LightCtl
)

func SendPacket(conn net.Conn, packet *lmp.LmpPacket) error {
	bytes, err := packet.Serialize()
	if err != nil {
		return errors.New("could not serialize packet")
	}

	n, err := conn.Write(bytes)
	if err != nil {
		return errors.New("could not write bytes to connection")
	}

	if n < 0 {
		return errors.New("did not send any bytes to connection")
	}

	return nil
}

func ReadPacket(conn net.Conn) (lmp.LmpPacket, error) {
	var buffer = make([]byte, lmp.LmpPacketMaxSize)
	size := 0
	b := make([]byte, 1)

	for {
		// NOTE(laith): reading one character at a time so i don't discard two packets at once after only seeing the first packet's terminating byte
		_, err := io.ReadFull(conn, b)
		if err != nil {
			// NOTE(laith): not surfacing my own error as its important to keep io.EOF readable upstream
			return lmp.LmpPacket{}, err
		}

		buffer[size] = b[0]
		size++

		if b[0] == lmp.LmpPacketTerminate {
			break
		}

		if size >= lmp.LmpPacketMaxSize {
			return lmp.LmpPacket{}, errors.New("packet exceeded max size")
		}
	}

	pkt, err := lmp.Deserialize(buffer[:size])
	if err != nil {
		return lmp.LmpPacket{}, errors.New("unable to deserialize buffer")
	}

	return pkt, nil
}

func SendHandshake(conn net.Conn, service int) error {
	sendInitPacket := lmp.LmpPacket{
		Version:       0x02,
		Type:          lmp.LmpTypeInit,
		Arg:           lmp.LmpArgInitInit,
		Flags:         lmp.LmpFlagsNone,
		Payload:       []byte{lmp.LmpPayloadEmpty},
		PayloadLength: 1,
	}

	if err := SendPacket(conn, &sendInitPacket); err != nil {
		return err
	}

	sendSendPacket := lmp.LmpPacket{
		Version:       0x02,
		Type:          lmp.LmpTypeSend,
		Arg:           lmp.LmpArgSend,
		Flags:         lmp.LmpFlagsNone,
		Payload:       []byte{byte(Admiral), byte(service)},
		PayloadLength: 2,
	}

	if err := SendPacket(conn, &sendSendPacket); err != nil {
		return err
	}

	readPacket, err := ReadPacket(conn)
	if err != nil {
		return err
	}

	if readPacket.Arg != lmp.LmpArgInitAccept {
		return errors.New("did not recieve accept packet from admiral")
	}

	return nil
}
