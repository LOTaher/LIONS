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
	"net"

	"github.com/LOTaher/lmp"
)

func LmpNetSendPacket(conn net.Conn, packet *lmp.LmpPacket) error {
	bytes, err := packet.Serialize()
	if err != nil {
		return errors.New("Could not serialize packet")
	}

	n, err := conn.Write(bytes)
	if err != nil {
		return errors.New("Could not write bytes to connection")
	}

	if n < 0 {
		return errors.New("Did not send any bytes to connection")
	}

	return nil
}

func LmpNetRecvPacket(conn net.Conn) (lmp.LmpPacket, error) {
	var buffer = make([]byte, lmp.LmpPacketMaxSize)
	terminated := false
	size := 0

	for {
		var scratch = make([]byte, lmp.LmpPacketMaxSize)

		n, err := conn.Read(scratch)
		if err != nil {
			return lmp.LmpPacket{}, errors.New("Could not read bytes from connection")
		}

		if n <= 0 {
			break
		}

		for i := 0; i < n; i++ {
			buffer[size] = scratch[i]
			size++

			if scratch[i] == lmp.LmpPacketTerminate {
				terminated = true
				break
			}
		}

		if terminated {
			break
		}
	}

	pkt, err := lmp.Deserialize(buffer)
	if err != nil {
		return lmp.LmpPacket{}, errors.New("Unable to deserialize buffer")
	}

	return pkt, nil
}
