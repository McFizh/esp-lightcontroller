#include "tcp_client_task.h"
#include "led_driver.h"

#include <string.h>

#include "lwip/sockets.h"

static int pwm_channels[CHANNEL_CNT] = {0};

static bool parse_value(const char *in, int *out)
{
    char *end;
    errno = 0;

    long val = strtol(in, &end, 10);

    if (end == in || *end != '\0' || errno == ERANGE)
        return false;

    *out = (int)val;
    return true;
}

static void reply_to_set_status(int socket, char *message)
{
    char *found1;
    char *found2;
    char *tokPtr;
    int channel;
    int value;

    if (strtok_r(message, " ", &tokPtr) == NULL)
    {
        char *msg = "Command expects 2 parameters\n";
        send(socket, msg, strlen(msg), 0);
        return;
    }

    found1 = strtok_r(NULL, " ", &tokPtr);
    if (found1 == NULL)
    {
        char *msg = "Parameter 1 missing\n";
        send(socket, msg, strlen(msg), 0);
        return;
    }

    found2 = strtok_r(NULL, " ", &tokPtr);
    if (found2 == NULL)
    {
        char *msg = "Parameter 2 missing\n";
        send(socket, msg, strlen(msg), 0);
        return;
    }

    if (!parse_value(found1, &channel) || !parse_value(found2, &value))
    {
        char *msg = "Only numbers are accepted as parameters\n";
        send(socket, msg, strlen(msg), 0);
        return;
    }

    if (channel < 0 || channel >= CHANNEL_CNT)
    {
        char *msg = "Channel out of range\n";
        send(socket, msg, strlen(msg), 0);
        return;
    }

    if (value < 0 || value > 100)
    {
        char *msg = "Channel value out of range\n";
        send(socket, msg, strlen(msg), 0);
        return;
    }

    if (set_channel_value(channel, value) != ESP_OK)
    {
        send(socket, "FAILED\n", 7, 0);
        return;
    }

    pwm_channels[channel] = value;
    send(socket, "OK\n", 3, 0);
}

static void reply_to_query_status(int socket)
{
    char status_string[64];

    snprintf(status_string, sizeof(status_string),
             "[ %d, %d, %d, %d, %d, %d, %d, %d, %d ]\n",
             pwm_channels[0], pwm_channels[1], pwm_channels[2],
             pwm_channels[3], pwm_channels[4], pwm_channels[5],
             pwm_channels[6], pwm_channels[7], pwm_channels[8]);
    send(socket, status_string, strlen(status_string), 0);
}

static void reply_to_ping(int socket)
{
    send(socket, "PONG\n", 5, 0);
}

/* **************************************************************************
 *
 * **************************************************************************/
void process_client_message(int socket, char *message)
{
    if (strncmp(message, "PING", 4) == 0)
    {
        reply_to_ping(socket);
        return;
    }

    if (strncmp(message, "SET_STATE", 9) == 0)
    {
        reply_to_set_status(socket, message);
        return;
    }

    if (strncmp(message, "STATUS", 6) == 0)
    {
        reply_to_query_status(socket);
        return;
    }

    //
    char *msg = "Invalid command\n";
    send(socket, msg, strlen(msg), 0);
}
