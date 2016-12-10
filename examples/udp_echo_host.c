/*
 * udp_echo_host.c
 *
 * Copyright (c) 2015 Usama Masood <mirzaon@gmail.com>
 *
 * Standard MIT License apply on this source code, with the inclusion of below
 * clause.
 *
 * This source is for educational purpose only, and should never be used for
 * any other purpose. If this source is used for other than educational purpose
 * (in any form) the author will not be liable for any legal charges.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

/* Configurations. */
#define PACKET_SIZE     strlen(MSG)

/* Message that will be sent. */
#define MSG "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"   \
            "01234567890123456789012345678901234567890123456789"

int main(int argc, char **argv)
{
    char result[65535];
    int sockfd, socklen;
    struct sockaddr_in servaddr, saddr;
    int n = 0, num;

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(11000);
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.3");
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("Unable to open new socket.");
        return (1);
    }

    saddr.sin_family = PF_INET;
    saddr.sin_port = htons(11001);
    saddr.sin_addr.s_addr = inet_addr("0.0.0.0");
    if (bind(sockfd, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("Unable to bind the socket.");
        return (1);
    }

    for(;;)
    {
        snprintf(result, 1472, "[%d]%.*s", n++, PACKET_SIZE, MSG);

        socklen = sizeof(servaddr);
        if( (sendto(sockfd, result, strlen(result), 0, (struct sockaddr*) &servaddr, socklen)) < 0 )
        {
            perror("Unable to send data.");
            return (1);
        }

        memset(result, 0, 65535);
        socklen = sizeof(servaddr);
        if ((num = recvfrom(sockfd, result, 65535, 0, (struct sockaddr*) &servaddr, &socklen)) < 0)
        {
            perror("Unable to receive.");
            return (1);
        }

        printf("Got[%d]-%s\n", num, result);
    }

    close(sockfd);

    return (0);

} /* main */
