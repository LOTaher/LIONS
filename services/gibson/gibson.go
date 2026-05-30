package main

import (
	"crypto/rand"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"net"
	"net/http"
	"os"

	"liblmp"
	"lmp"
)

var LIONS_API_KEY = ""

const (
	ADMIRAL_ENDPOINT = "100.113.240.39:5321"
	ENTRY_IP         = "100.113.240.39"
	ENTRY_PORT       = 8800
)

const LIONS_LOGO = "[LIONS //]"
const LIONS_LOGO_COLORED = LIONS_COLOR + LIONS_LOGO + COLOR_RESET

const (
	LIONS_COLOR          = "\x1b[38;5;220m"
	SERVICE_COLOR_GIBSON = "\x1b[38;5;34m"
	COLOR_RESET          = "\x1b[0m"
)

// NOTE(laith): update when adding new guest accounts
var GUEST_LIST = map[string]bool{"guest1": true}

func verifyBody(body []byte) error {
	str := string(body)
	if !GUEST_LIST[str] {
		return errors.New("Incorrect body content")
	}

	return nil
}

func createAdmiralConnection(pktChannel chan lmp.LmpPacket) {
	localAddr := &net.TCPAddr{
		IP:   net.ParseIP(ENTRY_IP),
		Port: ENTRY_PORT,
	}

	dial := &net.Dialer{
		LocalAddr: localAddr,
	}

	conn, err := dial.Dial("tcp", ADMIRAL_ENDPOINT)
	if err != nil {
		fmt.Sprintf("%s Failed to connect to admiral\n", LIONS_LOGO_COLORED)
		os.Exit(1)
	}

	if err := liblmp.SendHandshake(conn, int(liblmp.Gibson)); err != nil {
		fmt.Sprintf("%s Failed to handshake admiral\n", LIONS_LOGO_COLORED)
	}

	for true {
		pkt := <-pktChannel

		buf, err := pkt.Serialize()
		if err != nil {
			// NOTE(laith): all packets should serialize properly at this point
			panic(err)
		}

		fmt.Println("payload that would be sent to admiral recieved: ", buf)

		// conn.Write(buf)
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

// NOTE(laith): using closures is a good way to substitute inputs that require
// a function type
func receptionHandler(pktChannel chan lmp.LmpPacket) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if r.Method == "POST" {
			key := r.Header.Get("X-LIONS-KEY")
			if key == "" {
				w.WriteHeader(http.StatusBadRequest)
				fmt.Fprintf(w, "No API Key Provided")
				return
			}
			if key != LIONS_API_KEY {
				w.WriteHeader(http.StatusBadRequest)
				fmt.Fprintf(w, "Incorrect API Key")
				return
			}

			body, err := io.ReadAll(r.Body)
			if err != nil {
				w.WriteHeader(http.StatusBadRequest)
				fmt.Fprintf(w, "Failed to read body content")
				return
			}

			err = verifyBody(body)
			if err != nil {
				w.WriteHeader(http.StatusBadRequest)
				fmt.Fprint(w, err.Error())
				return
			}

			var sendPacket lmp.LmpPacket

			// NOTE(laith): [Dest][Sender]
			var admiralHeader = []byte{byte(liblmp.Reception), byte(liblmp.Gibson)}
			payload := append(admiralHeader, body...)

			sendPacket.Version = 2
			sendPacket.Type = lmp.LmpTypeSend
			sendPacket.Arg = lmp.LmpArgSend
			sendPacket.Flags = lmp.LmpFlagsNone
			sendPacket.Payload = payload
			sendPacket.PayloadLength = len(payload)

			pktChannel <- sendPacket
		} else {
			w.WriteHeader(http.StatusBadRequest)
			return
		}

		fmt.Fprintf(w, "Sent request to reception")
	}
}

func cors(next http.Handler) http.Handler {
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Access-Control-Allow-Origin", "*")
		w.Header().Set("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS")
		w.Header().Set("Access-Control-Allow-Headers", "Content-Type, X-LIONS-KEY")

		if r.Method == http.MethodOptions {
			w.WriteHeader(http.StatusNoContent)
			return
		}

		next.ServeHTTP(w, r)
	})
}

func main() {
	LIONS_API_KEY = generateKey()
	pktChannel := make(chan lmp.LmpPacket)

	go createAdmiralConnection(pktChannel)

	fmt.Printf("%s Starting Gibson - HTTP Bridge - Version 1\n", LIONS_LOGO_COLORED)
	fmt.Printf("%s API Key: %s\n", LIONS_LOGO_COLORED, LIONS_API_KEY)

	mux := http.NewServeMux()
	mux.HandleFunc("/reception", receptionHandler(pktChannel))

	if err := http.ListenAndServe(":8081", cors(mux)); err != nil {
		panic(err)
	}
}
