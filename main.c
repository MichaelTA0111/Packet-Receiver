#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cheri/cheri.h>
#include <cheri/cheric.h>
#include <math.h>

#include "consumer.h"

#define MAXBUFLEN 200


void consume_packet(Consumer *consumer)
{
    consumer_increment_counter(consumer);
    return;
}

/*
 * Create a network socket
 */
static int
create_socket(const char *port, struct addrinfo *servinfo, struct addrinfo **p, int do_bind) {
    int sockfd;
    struct addrinfo hints, *q;
    int rv;

    // Create a hints struct
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo("localhost", port, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }

    // loop through all the results and make a socket
    for(q = servinfo; q != NULL; q = q->ai_next) {
        if ((sockfd = socket(q->ai_family, q->ai_socktype,
                q->ai_protocol)) == -1) {
            perror("Error creating socket");
            continue;
        }

        if (do_bind) {
            if (bind(sockfd, q->ai_addr, q->ai_addrlen) == -1) {
                close(sockfd);
                perror("Error binding");
                continue;
            }
        }
        break;
    }

    if (q == NULL) {
        fprintf(stderr, "Failed to create socket!\n");
        return -1;
    }

    *p = q;

    return sockfd;
}

int receive_packet(Consumer *consumer, int sockfd)
{
    int numbytes;
    struct sockaddr_storage their_addr;
    socklen_t addr_len;
    char buf[MAXBUFLEN];

    printf("Waiting for packet...\n");

    addr_len = sizeof their_addr;
    if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
        (struct sockaddr *)&their_addr, &addr_len)) == -1) {
        perror("recvfrom");
        exit(1);
    }

    buf[numbytes] = '\0';
    if (!strcmp(buf, "FINISHED")) {
        printf("Finished receiving packets!\n");
        return 0;
    }

    consumer_increment_counter(consumer);

    printf("Packet of length %d bytes received!\n", numbytes);
    printf("0x%lX %p %u\n", cheri_getperm(buf),
            buf,
            numbytes);

    int i, j;
    for (i = 0; i < ceil((double) numbytes / 16.0); i++) {
	for (j = 0; j < 16; j++) {
	    if (i*16+j < numbytes) { 
	        printf("%02X ", buf[i*16+j]);
	    } else {
	        printf("   ");
	    }
        }
	printf("| ");
	for (j = 0; j < 16; j++) {
	    if (i*16+j < numbytes) { 
	        printf("%c", buf[i*16+j]);
	    }
        }
	printf("\n");
    }

    printf("Consumer counter %lu\n", consumer->counter);

    return 1;
}

void send_consumer_details(Consumer *consumer, int sockfd, struct addrinfo *p)
{
    int numbytes;
    char msg[MAXBUFLEN];

    sprintf(msg, "%lu", consumer->counter);
    printf("%s\n", msg);

    if ((numbytes = sendto(sockfd, msg, strlen(msg),
            0, p->ai_addr, p->ai_addrlen)) == -1) {
        perror("Error with sendto command");
        exit(1);
    }

    printf("Sent %d bytes to application.\n", numbytes);
}

int main(int argc, char *argv[])
{
    int sockfd;
    Consumer consumer;
    struct addrinfo *servinfo = NULL, *p;

    consumer.counter = 0;

    if (argc != 3) {
        fprintf(stderr,"usage: ./application.o SRC_PORT DST_PORT\n");
        exit(1);
    }

    const char *src_port = argv[1];
    const char *dst_port = argv[2];
    printf("Ports %s %s\n", src_port, dst_port);

    if ((sockfd = create_socket(src_port, servinfo, &p, 1)) < 0) {
        printf("Error creating socket!\n");
        return -1;
    }

    while (receive_packet(&consumer, sockfd)) {}
    
    close(sockfd);

    if ((sockfd = create_socket(dst_port, servinfo, &p, 0)) < 0) {
        printf("Error creating socket!\n");
        return -1;
    }

    send_consumer_details(&consumer, sockfd, p);

    freeaddrinfo(servinfo);
    close(sockfd);

    return 0;
}
