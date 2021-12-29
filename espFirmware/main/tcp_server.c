#include <string.h>

#include "freertos/FreeRTOS.h"

#include "freertos/task.h"
#include "lwip/sockets.h"

#include "../build/main/app_config.h"

#include "tcp_client_task.h"

int listen_socket;

static void tcpserver_task(void *);

struct AppClient {
  bool slotFree;
  int socket;
  unsigned char id;
} appClient[max_clients];


/* ***************************************************************
 * Helper methods
 * ***************************************************************/
char *rtrim(char *s) {
    char* back = s + strlen(s);
    while(isspace(*--back));
    *(back+1) = '\0';
    return s;
}


/* ***************************************************************
 * Init / main TCP loop methods
 * ***************************************************************/
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
  for(int i=0; i<max_clients; i++) {
    appClient[i].slotFree = true;
    appClient[i].id = i;
  }

  printf("Socket ready...\n");
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
    for(i=0; i<max_clients; i++) {
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

/* ***************************************************************
 * Client loop
 * ***************************************************************/
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
      rtrim(rx_buffer);

      printf("(%d) Received: %d bytes: (%s)\n", client->socket, len, rtrim(rx_buffer) );

      process_client_message(client->socket, rx_buffer );
  }

  printf("(%d) Task terminated (id: %d)\n", client->socket, client->id);
  close(client->socket);
  client->slotFree = true;
  vTaskDelete(NULL);
}