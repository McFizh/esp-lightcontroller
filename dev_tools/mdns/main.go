package main

import (
   "fmt"
   "os"
   "os/signal"
   "syscall"
   "time"
    "github.com/hashicorp/mdns"
)

func main() {
    host, _ := os.Hostname()
    info := []string{"Test service for esp"}
    service, _ := mdns.NewMDNSService(host, "_esp._tcp", "", "", 8000, nil, info)
    server, _ := mdns.NewServer(&mdns.Config{Zone: service})
    defer server.Shutdown()

    entriesCh := make(chan *mdns.ServiceEntry, 4)
    go func() {
        for entry := range entriesCh {
            fmt.Printf("mDns entry: %v\n", entry)
        }
    }()
    mdns.Lookup("_lightctrl._tcp", entriesCh)
    close(entriesCh)

    // Wait for ctrl+c.. then cleanup
    c := make(chan os.Signal)
    signal.Notify(c, os.Interrupt, syscall.SIGTERM)
    go func() {
        <-c
        os.Exit(1)
    }()

    for {
        time.Sleep(1);
    }
}
