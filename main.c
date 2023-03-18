#include "socket.h"
#include "helpers.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

int sockfd;
unsigned short id;
short times_burst[PROBES];

// Bash color codes
int colors[COLORS] = {
	231, 230, 229, 228,
	227, 226, 220, 214,
	208, 202, 196, 197,
	198, 199, 200, 201,
	207, 213, 219, 225
};

// Initialize global variables
void init(void) {
	srand(time(NULL));
	id = rand();
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	memset(times_burst, -1, sizeof(times_burst));
}

int main(int argc, char* argv[]) {
	int ttl = 0, last_ttl = 0, count = 0;
	long long start, end = 0;
	char* route = NULL;
	bool success = false;
	// Initialize
	init();
	// Guard check we can safely proceed to main loop
	if (!guard(sockfd, argc, argv)) return EXIT_FAILURE;
	// Main loop
	while (true) {
		if (count == 0) {
			// Once we've received all responses for a burst,
			// print the route and calculate the average time
			if (route != NULL) {
				int ms = calc_avg_time(times_burst);
				// Print the route and average time
				set_color(get_color(colors, ttl));
				printf("%s", route);
				if (ms == -1) printf(" (\?\?\?)\n");
				else printf(" (%d ms)\n", ms);
				clear_color();
				// We have reached the target
				if (strcmp(route, argv[1]) == 0) {
					success = true;
					break;
				}
				free(route);
				route = NULL;
			}
			// Transmit a new burst
			burst_transmit(sockfd, argv[1], id, ++ttl);
			start = current_timestamp();
			count = PROBES;
		}
		// Check if we've received a response
		if (is_receive_ready(sockfd, 1)) {
			end = current_timestamp();
			struct packet* packet = receive(sockfd);
			// Check if the response is valid and belongs to the current burst
			if (packet->id >= id && packet->id <= id + PROBES) {
				// Store the time it took to receive the response
				times_burst[count - 1] = end - start;
				last_ttl = ttl;
				count--;
				// If this is the first response, set the route
				if (route == NULL) route = packet->ip;
				// Otherwise, free given route
				else free(packet->ip);
			}
			free(packet);
		} else {
			// If we haven't received a response 
			// for the last 5 routers, stop probing
			if (ttl - last_ttl >= 5) break;
			// Print a star to indicate a timeout
			printf("\e[30;2m  *\e[0m\n");
			// Reset the burst
			count = 0;
		}
	}
	// Print the resulting outcome of the traceroute
	if (success) printf("\e[32m\nSuccess \e[0m\n");
	else printf("\e[31m\nFailed to connect\e[0m\n");
	return EXIT_SUCCESS;
}