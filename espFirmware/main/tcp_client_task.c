#include <string.h>

#include "lwip/sockets.h"

int pwm_channels[9] = { 5,10,15,20,25,30,35,40,45 };

void reply_to_query_status(int);
void reply_to_ping(int);
void reply_to_set_status(int, char *);

void process_client_message(int socket, char *message) {
    if(strncmp(message, "PING", 4) == 0) {
        reply_to_ping(socket);
        return;
    }

    if(strncmp(message, "SET_STATE", 9) == 0) {
        reply_to_set_status(socket, message);
        return;
    }

    if(strncmp(message, "STATUS", 5) == 0) {
        reply_to_query_status(socket);
        return;
    }

    //
    char *msg = "Invalid command\n";
    send(socket, msg , strlen(msg), 0);
}

/* **************************************************************************
 *
 * **************************************************************************/
void reply_to_set_status(int socket, char *message) {
    char *found1;
    char *found2;
    int channel;
    int value;

    if(strtok(message, " ") == NULL) {
        char *msg = "Command expects 2 parameters\n";
        send(socket, msg , strlen(msg), 0);
        return;
    }

    found1 = strtok(NULL, " ");
    if(found1 == NULL) {
        char *msg = "Parameter 1 missing\n";
        send(socket, msg , strlen(msg), 0);
        return;
    }

    found2 = strtok(NULL, " ");
    if(found2 == NULL) {
        char *msg = "Parameter 2 missing\n";
        send(socket, msg , strlen(msg), 0);
        return;
    }

    channel = atoi(found1);
    value = atoi(found2);

    if(channel < 0 || channel >= 9) {
        char *msg = "Channel out of range\n";
        send(socket, msg , strlen(msg), 0);
        return;
    }

    if(value < 0 || value > 100) {
        char *msg = "Channel value out of range\n";
        send(socket, msg , strlen(msg), 0);
        return;
    }

    pwm_channels[channel] = value;
    send(socket, "OK\n", 3, 0);
}

void reply_to_query_status(int socket) {
    char *status_string;

    asprintf(&status_string, "[ %d, %d, %d, %d, %d, %d, %d, %d, %d ]\n",
        pwm_channels[0], pwm_channels[1], pwm_channels[2],
        pwm_channels[3], pwm_channels[4], pwm_channels[5],
        pwm_channels[6], pwm_channels[7], pwm_channels[8]
    );
    send( socket, status_string, strlen(status_string), 0 );

    free(status_string);
}

void reply_to_ping(int socket) {
    send(socket, "PONG\n", 5, 0);
}