#include "freertos/FreeRTOS.h"

#include "freertos/task.h"
#include "lwip/sockets.h"

int listen_socket;

#define MAX_CLIENTS 3
struct AppClient {
  bool slotFree;
  int socket;
  unsigned char id;
} appClient[MAX_CLIENTS];

void init_tcp_server() {
  char addr_str[128];

  struct sockaddr_in destAddr;
  destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  destAddr.sin_family = AF_INET;
  destAddr.sin_port = htons(5900);
  inet_ntoa_r(destAddr.sin_addr, addr_str, sizeof(addr_str) - 1);

  listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (listen_socket < 0) {
    printf("Unable to create socket: errno %d", errno);
    return;
  }

  int err = bind(listen_socket, (struct sockaddr *)&destAddr, sizeof(destAddr));
  if (err != 0) {
    printf("Socket unable to bind: errno %d", errno);
    return;
  }

  err = listen(listen_socket, 1);
  if (err != 0) {
      printf("Error occured during listen: errno %d", errno);
      return;
  }

  //
  for(int i=0; i<MAX_CLIENTS; i++) {
    appClient[i].slotFree = true;
    appClient[i].id = i;
  }

  printf("Socket ready...\n");
}

static void tcpserver_task(void *pvParameters)
{
  struct AppClient *client = (struct AppClient *)pvParameters;
  char rx_buffer[128];

  printf("(%d) Task created (id: %d)\n", client->socket, client->id);

  for( ;; ) {
      int len = recv(client->socket, rx_buffer, sizeof(rx_buffer) - 1, 0);

      // Connection terminated
      if(len == 0) {
        printf("(%d) Connection terminated\n", client->socket);
        break;
      } else if(len == -1) {
        printf("(%d) Connection failure\n", client->socket);
        break;
      }

      rx_buffer[len] = 0;
      printf("(%d) Received: %d bytes\n", client->socket, len);
      printf("(%d) %s", client->socket, rx_buffer);
  }

  close(client->socket);
  client->slotFree = true;
  vTaskDelete(NULL);
}

void run_tcp_loop() {
  struct sockaddr_in sourceAddr;
  uint addrLen = sizeof(sourceAddr);

  int sock;
  unsigned char i, freeSlot;

  while(1) {
    printf("Waiting for connection...\n");
    printf("Free memory: %d\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    sock = accept(listen_socket, (struct sockaddr *)&sourceAddr, &addrLen);
    if(sock == -1) {
      printf("Failed to create socket\n");
      continue;
    }

    freeSlot = 255;
    for(i=0; i<MAX_CLIENTS; i++) {
      if(appClient[i].slotFree) {
        freeSlot=i;
        break;
      }
    }

    if(freeSlot == 255) {
      printf("Unable to accept connection, no free slots\n");
      close(sock);
    } else {
      appClient[freeSlot].slotFree = false;
      appClient[freeSlot].socket = sock;
      printf("Connected... %d\n", sock);
      xTaskCreate(&tcpserver_task, "tcp_server_task", 2048, (void*)&appClient[freeSlot], 5, NULL);
    }
  }
}