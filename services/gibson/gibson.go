package main

import (
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"net"
	"net/http"
	"os"

	"github.com/LOTaher/lmp"
)

var LIONS_API_KEY = ""

const BUFFER_LEN = 20 // key=[16 char key]
const ADMIRAL_ENDPOINT = "100.113.240.39:5321"
const ENTRY_IP = "100.113.240.39"
const ENTRY_PORT = 8800

func createAdmiralConnection(ch chan lmp.LmpPacket) {
	localAddr := &net.TCPAddr{
		IP:   net.ParseIP(ENTRY_IP),
		Port: ENTRY_PORT,
	}

	dial := &net.Dialer{
		LocalAddr: localAddr,
	}

	conn, err := dial.Dial("tcp", ADMIRAL_ENDPOINT)
	if err != nil {
		fmt.Println(err)
		fmt.Printf("Failed to connect to admiral\n")
		os.Exit(1)
	}

	for true {
		pkt := <-ch

		buf, err := pkt.Serialize()
		if err != nil {
			fmt.Printf("Unable to serialize packet\n")
			continue
		}

		conn.Write(buf)
	}

	conn.Close()
}

func generateKey() string {
	key := make([]byte, 16)
	_, err := rand.Read(key)
	if err != nil {
		panic(err)
	}

	return hex.EncodeToString(key)
}

// NOTE(laith): closure patterning so hard rn. good way to substitute inputs that require
// a function type
func receptionHandler(ch chan lmp.LmpPacket) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Method == "POST" {
			// key := r.Header.Get("X-LIONS-KEY")
			// if key == "" {
			// 	w.WriteHeader(http.StatusBadRequest)
			// 	fmt.Fprintf(w, "No API Key Provided")
			// 	return
			// }
			// if key != LIONS_API_KEY {
			// 	w.WriteHeader(http.StatusBadRequest)
			// 	fmt.Fprintf(w, "Incorrect API Key")
			// 	return
			// }

			var sendPacket lmp.LmpPacket = lmp.LmpPacketInit()

			sendPacket.Version = 2
			sendPacket.Type = lmp.LmpTypeSend
			sendPacket.Arg = lmp.LmpArgSend
			sendPacket.Flags = 0
			sendPacket.Payload = []byte("12?")
			sendPacket.PayloadLength = 3

			buf := make([]byte, BUFFER_LEN)
			r.Body.Read(buf)
			ch <- sendPacket
		} else {
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		return
	}
}

func main() {
	LIONS_API_KEY = generateKey()
	pktChannel := make(chan lmp.LmpPacket)

	go createAdmiralConnection(pktChannel)

	fmt.Printf("Entry API Key: %s\n", LIONS_API_KEY)

	http.HandleFunc("/reception", receptionHandler(pktChannel))

	http.ListenAndServe(":8080", nil)
}
