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

// type ReceptionResponse struct {
// 	Username string
// 	Password string
// }

func getConfirmation() {
	// get a packet to send to admiral to confirm
	var sendPacket lmp.LmpPacket = lmp.LmpPacketInit()

	// send packet to admiral
	sendPacket.Version = 2
	sendPacket.Type = lmp.LmpTypeSend
	sendPacket.Arg = lmp.LmpArgSend
	sendPacket.Flags = 0
	sendPacket.Payload = []byte("12?")
	sendPacket.PayloadLength = 3

	buf, err := sendPacket.Serialize()

	if err != nil {
		fmt.Printf("Unable to serialize packet.\n")
		os.Exit(1)
	}

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

	conn.Write(buf)
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

func receptionHandler(w http.ResponseWriter, r *http.Request) {
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

		buf := make([]byte, BUFFER_LEN)
		r.Body.Read(buf)

		// getConfirmation
		getConfirmation()
	} else {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	return
}

func main() {
	LIONS_API_KEY = generateKey()

	fmt.Printf("Entry API Key: %s\n", LIONS_API_KEY)

	http.HandleFunc("/reception", receptionHandler)

	http.ListenAndServe(":8080", nil)
}
