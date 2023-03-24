#ifndef SOCKET_H
#define SOCKET_H

#include <stdbool.h>
#include <stdint.h>

struct packet {
	char* ip;
	unsigned short id;
	unsigned short seq;
};

// Calculate checksum for a packet
u_int16_t compute_icmp_checksum (const void *buff, int length);

// Wait for maximally given amount of seconds for a response 
// and return if it was received within given time or not
bool is_receive_ready(int sockfd, long long* time);

// Atomic function for receiving single packet
struct packet* receive(int sockfd);

// Atomic function for sending single packet
void transmit(int sockfd, char* target, int ttl, short id, short seq);

#endif
