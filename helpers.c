#include "helpers.h"
#include "socket.h"

// Calculate average time
int calc_avg_time(short times[PROBES]) {
	int sum = 0;
	for (int i = 0; i < PROBES; i++) {
		if (times[i] == -1) return -1;
		sum += times[i];
	}
	return sum / PROBES;
}

// Function for sending burst of packets
void burst_transmit(int sockfd, char* target, int id, int ttl) {
	for (int i = 0; i < PROBES; i++)
		transmit(sockfd, target, ttl, id + i, ttl - 1);
}

// Function for measuring miliseconds
long long current_timestamp() {
	struct timeval te; 
	gettimeofday(&te, NULL);
	long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000;
	return milliseconds;
}

// Return color for given route
int get_color(int* colors, int ttl) {
	return colors[ttl % COLORS];
}

// Gruard function for main
bool guard(int sockfd, int argc, char* argv[]) {
	// Check if user passed enough arguments
	if (argc != 2) {
		fprintf(stderr, "usage: %s [TARGET]\n", argv[0]);
		return false;
	}
	// Check if socket was created successfully
	if (sockfd < 0) {
		fprintf(stderr, "socket error: %s\n", strerror(errno)); 
		return false;
	}
	return true;
}

void set_color(int color) {
	printf("\x1b[38;5;%dm", color);
}

void clear_color(void) {
	printf("\e[0m");
}
