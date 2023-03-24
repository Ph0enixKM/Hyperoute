#include "helpers.h"
#include "socket.h"
#include <sys/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

int calc_avg_time(short times[PROBES]) {
	int sum = 0;
	for (int i = 0; i < PROBES; i++) {
		if (times[i] == -1) return -1;
		sum += times[i];
	}
	return sum / PROBES;
}

void burst_transmit(int sockfd, char* target, int id, int ttl) {
	for (int i = 0; i < PROBES; i++)
		transmit(sockfd, target, ttl, id + i, id - (i + ttl));
}

long long current_timestamp() {
	struct timeval te; 
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
	return milliseconds;
}

int get_color(int* colors, int ttl) {
	return colors[ttl % COLORS];
}

bool guard(int sockfd, int argc, char* argv[], enum input_type* type) {
	// Check if user passed enough arguments
	if (argc != 2) {
		fprintf(stderr, "\e[33musage\e[0m: %s [TARGET]\n", argv[0]);
		return false;
	}
	// Check if user passed a valid IPv4 address
	if (!is_valid_target(argv[1], type)) {
		fprintf(stderr, "\e[31merror\e[0m: invalid IPv4 address\n");
		return false;
	}
	// Check if socket was created successfully
	if (sockfd < 0) {
		fprintf(stderr, "\e[31msocket error\e[0m: %s\n", strerror(errno)); 
		return false;
	}
	return true;
}

bool is_valid_target(char* ip, enum input_type* type) {
	struct sockaddr_in sa;
	int result = inet_pton(AF_INET, ip, &(sa.sin_addr));
	// Check if it's a hostname
	if (!result) {
		struct hostent *hostent_ptr;
		hostent_ptr = gethostbyname(ip);
		*type = HOSTNAME;
		if (hostent_ptr == NULL) return false;
		return true;
	}
	*type = IP;
	return result;
}

void set_color(int color) {
	printf("\x1b[38;5;%dm", color);
}

void clear_color(void) {
	printf("\e[0m");
}

char *hostname_to_ip(const char *hostname) {
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    char *ip_address = NULL;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(hostname, NULL, &hints, &result) != 0) return NULL;
    for (rp = result; rp != NULL; rp = rp->ai_next) {
        struct sockaddr_in *sa = (struct sockaddr_in *)rp->ai_addr;
        ip_address = inet_ntoa(sa->sin_addr);
        break;
    }
    freeaddrinfo(result);
    return ip_address;
}

char *ip_to_hostname(const char *ip_address) {
    struct sockaddr_in sa;
    struct hostent *hostent_ptr;
    if (inet_pton(AF_INET, ip_address, &sa.sin_addr) != 1) return NULL;
    hostent_ptr = gethostbyaddr(&sa.sin_addr, sizeof(sa.sin_addr), AF_INET);
    if (hostent_ptr == NULL) return NULL;
    return hostent_ptr->h_name;
}

bool is_new_route(char* routes[PROBES], char* route) {
	for (int i = 0; i < PROBES; i++) {
		if (routes[i] == NULL) continue;
		if (strcmp(routes[i], route) == 0) {
			return false;
		}
	}
	return true;
}

bool is_routes_empty(char* routes[PROBES]) {
	for (int i = 0; i < PROBES; i++)
		if (routes[i] != NULL) return false;
	return true;
}

void store_route(char* routes[PROBES], char* route) {
	for (int i = 0; i < PROBES; i++) {
		if (routes[i] == NULL) {
			routes[i] = route;
			return;
		}
	}
}
