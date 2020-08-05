package main

import (
	"fmt"
	"net"
	"time"
)

func main() {
	conn1, _ := net.Dial("tcp", "192.168.1.200:5900")
	conn2, _ := net.Dial("tcp", "192.168.1.200:5900")
	conn3, _ := net.Dial("tcp", "192.168.1.200:5900")
	conn4, _ := net.Dial("tcp", "192.168.1.200:5900")

	fmt.Fprintf(conn1, "CONN (1)\n")
	fmt.Fprintf(conn2, "CONN (2)\n")
	fmt.Fprintf(conn3, "CONN (3)\n")
	fmt.Fprintf(conn4, "CONN (4)\n")

	time.Sleep(5)
}
