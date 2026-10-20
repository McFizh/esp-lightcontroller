#include "tcp_server.h"
#include "tcp_client_task.h"
#include "connection.h"

#include <string.h>

#include "freertos/FreeRTOS.h"

#include "freertos/task.h"
#include "lwip/sockets.h"

#include "esp_log.h"

#include "../build/main/app_config.h"

static const char *LOG_TAG = "tcp_server";
static int listen_socket;
static void tcpserver_task(void *);

struct AppClient
{
  bool slotFree;
  int socket;
  unsigned char id;
} appClient[max_clients];

/* ***************************************************************
 * Helper methods
 * ***************************************************************/
static char *rtrim(char *text)
{
  char *back = text + strlen(text);

  while (back > text && isspace((unsigned char)back[-1]))
    back--;

  *back = '\0';

  return text;
}

/* ***************************************************************
 * Init / main TCP loop methods
 * ***************************************************************/
esp_err_t init_tcp_server()
{
  struct sockaddr_in destAddr;
  destAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  destAddr.sin_family = AF_INET;
  destAddr.sin_port = htons(SERVER_PORT);

  listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
  if (listen_socket < 0)
  {
    ESP_LOGE(LOG_TAG, "socket() failed, errno: %d", errno);
    return ESP_FAIL;
  }

  int err = bind(listen_socket, (struct sockaddr *)&destAddr, sizeof(destAddr));
  if (err != 0)
  {
    ESP_LOGE(LOG_TAG, "bind() failed, errno: %d", errno);
    return ESP_FAIL;
  }

  err = listen(listen_socket, 1);
  if (err != 0)
  {
    ESP_LOGE(LOG_TAG, "listen() failed, errno: %d", errno);
    return ESP_FAIL;
  }

  //
  for (int i = 0; i < max_clients; i++)
  {
    appClient[i].slotFree = true;
    appClient[i].id = i;
  }

  ESP_LOGI(LOG_TAG, "Listening socket ready");
  return ESP_OK;
}

void run_tcp_loop()
{
  struct sockaddr_storage sourceAddr;

  int sock;
  unsigned char i, freeSlot;

  while (1)
  {
    ESP_LOGI(LOG_TAG, "Waiting for connection...");
    printf("Free memory: %u\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));

    socklen_t addrLen = sizeof(sourceAddr);
    sock = accept(listen_socket, (struct sockaddr *)&sourceAddr, &addrLen);
    if (sock == -1)
    {
      ESP_LOGE(LOG_TAG, "accept() failed");
      continue;
    }

    freeSlot = 255;
    for (i = 0; i < max_clients; i++)
    {
      if (appClient[i].slotFree)
      {
        freeSlot = i;
        break;
      }
    }

    if (freeSlot == 255)
    {

      ESP_LOGW(LOG_TAG, "Unable to accept connection, no free slots");
      close(sock);
      continue;
    }

    // Start client handler
    appClient[freeSlot].slotFree = false;
    appClient[freeSlot].socket = sock;
    ESP_LOGI(LOG_TAG, "Client connected to socket: %d", sock);
    if (xTaskCreate(&tcpserver_task, "tcp_server_task", 3072, (void *)&appClient[freeSlot], 5, NULL) != pdPASS)
    {
      ESP_LOGE(LOG_TAG, "CLient handler task creation failed for socket: %d", sock);
      close(sock);
      appClient[freeSlot].slotFree = true;
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

  for (;;)
  {
    int len = recv(client->socket, rx_buffer, sizeof(rx_buffer) - 1, 0);

    // Connection terminated
    if (len == 0)
    {
      ESP_LOGI(LOG_TAG, "Connection terminated, socket: %d", client->socket);
      break;
    }
    else if (len == -1)
    {
      ESP_LOGI(LOG_TAG, "Connection failure, socket: %d", client->socket);
      break;
    }

    rx_buffer[len] = 0;
    rtrim(rx_buffer);

    printf("(%d) Received: %d bytes: (%s)\n", client->socket, len, rtrim(rx_buffer));

    process_client_message(client->socket, rx_buffer);
  }

  printf("(%d) Task terminated (id: %d)\n", client->socket, client->id);
  close(client->socket);
  client->slotFree = true;
  vTaskDelete(NULL);
}