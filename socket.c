#include "socket.h"
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "helpers.h"

u_int16_t compute_icmp_checksum (const void *buff, int length) {
	u_int32_t sum;
	const u_int16_t* ptr = buff;
	assert (length % 2 == 0);
	for (sum = 0; length > 0; length -= 2)
		sum += *ptr++;
	sum = (sum >> 16) + (sum & 0xffff);
	return (u_int16_t)(~(sum + (sum >> 16)));
}

bool is_receive_ready(int sockfd, long long* time) {
	fd_set descriptors;
	FD_ZERO (&descriptors);
	FD_SET (sockfd, &descriptors);
	long long before = current_timestamp();
	struct timeval tv;
	tv.tv_sec = *time / 1000;
	tv.tv_usec = (*time % 1000) * 1000;
	int ready = select(sockfd+1, &descriptors, NULL, NULL, &tv);
	long long after = current_timestamp();
	*time -= (after - before);
	return ready == 1;
}

struct packet* receive(int sockfd) {
	// Receive packet
	struct sockaddr_in sender;	
	socklen_t sender_len = sizeof(sender);
	u_int8_t buffer[IP_MAXPACKET];
	ssize_t packet_len = recvfrom (sockfd, buffer, IP_MAXPACKET, 0, (struct sockaddr*)&sender, &sender_len);
	// Check if packet was received successfully
	if (packet_len < 0) {
		fprintf(stderr, "recvfrom error: %s\n", strerror(errno)); 
		return NULL;
	}
	char* ip_str = malloc(INET_ADDRSTRLEN);
	inet_ntop(AF_INET, &(sender.sin_addr), ip_str, sizeof(char) * INET_ADDRSTRLEN);
	struct ip* ip_header = (struct ip*) buffer;
	ssize_t	ip_header_len = 4 * ip_header->ip_hl;
	unsigned short ip = *(short*)(buffer + ip_header_len + 32);
	unsigned short seq = *(short*)(buffer + ip_header_len + 34);
	struct packet* p = malloc(sizeof(struct packet));
	p->ip = ip_str;
	p->id = ip;
	p->seq = seq;
	return p;
}

void transmit(int sockfd, char* target, int ttl, short id, short seq) {
	// Set up ICMP header
	struct icmp header;
	header.icmp_type = ICMP_ECHO;
	header.icmp_code = 0;
	header.icmp_hun.ih_idseq.icd_id = id;
	header.icmp_hun.ih_idseq.icd_seq = seq;
	// Must be set to zero for checksum calculation
	header.icmp_cksum = 0;
	header.icmp_cksum = compute_icmp_checksum ((u_int16_t*)&header, sizeof(header));
	// Set up recipient
	struct sockaddr_in recipient;
	bzero(&recipient, sizeof(recipient));
	recipient.sin_family = AF_INET;
	inet_pton(AF_INET, target, &recipient.sin_addr);
	// Set up TTL
	setsockopt (sockfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(int));
	// Send packet
	ssize_t bytes_sent = sendto(
		sockfd,
		&header,
		sizeof(header),
		0,
		(struct sockaddr*)&recipient,
		sizeof(recipient)
	);
}
