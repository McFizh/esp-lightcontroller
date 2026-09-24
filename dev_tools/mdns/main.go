package main

import (
	"fmt"
	"log"
	"net"
	"os"
	"os/signal"
	"strconv"
	"strings"
	"syscall"

	"github.com/hashicorp/mdns"
)

func testConnection(hostname string, port int) {
	target := net.JoinHostPort(hostname, strconv.Itoa(port))

	conn1, err := net.Dial("tcp", target)
	if err != nil {
		log.Println(err)
		return
	}
	defer conn1.Close()

	conn2, err := net.Dial("tcp", target)
	if err != nil {
		log.Println(err)
		return
	}
	defer conn2.Close()

	conn3, err := net.Dial("tcp", target)
	if err != nil {
		log.Println(err)
		return
	}
	defer conn3.Close()

	conn4, err := net.Dial("tcp", target)
	if err != nil {
		log.Println(err)
		return
	}
	defer conn4.Close()

	fmt.Fprintf(conn1, "CONN (1)\n")
	fmt.Fprintf(conn2, "CONN (2)\n")
	fmt.Fprintf(conn3, "CONN (3)\n")
	fmt.Fprintf(conn4, "CONN (4)\n")
}

func runQuery() {
	log.Println("Running mDns query...")
	entriesCh := make(chan *mdns.ServiceEntry, 4)
	go func() {
		for entry := range entriesCh {
			// Filter out non-related responses
			if !strings.HasSuffix(entry.Name, "_lightctrl._tcp.local.") {
				continue
			}

			log.Printf("mDns entry: %v\n", entry)
			testConnection(entry.AddrV4.String(), entry.Port)
		}
	}()
	mdns.Lookup("_lightctrl._tcp", entriesCh)
	close(entriesCh)
}

func main() {
	// Advertise _esp._tcp service on port 8000
	host, _ := os.Hostname()
	info := []string{"Test service for esp"}
	service, _ := mdns.NewMDNSService(host, "_esp._tcp", "", "", 8000, nil, info)
	server, _ := mdns.NewServer(&mdns.Config{Zone: service})

	// Search for lightcontrollers and run connection test if found
	runQuery()

	// Create a co-routine to wait for ctrl+c.. then cleanup
	c := make(chan os.Signal, 1)
	signal.Notify(c, os.Interrupt, syscall.SIGTERM)
	go func() {
		// Block until signal arrives from channel
		<-c

		// Shutdown server
		server.Shutdown()
		os.Exit(1)
	}()

	// Block the app from exiting and let mdns server run in background
	select {}
}
