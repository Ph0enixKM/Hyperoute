#include "socket.h"
#include "helpers.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>

int sockfd;
unsigned short id;
char* target_route = NULL;
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
void init(enum input_type type, char* argv[]) {
	srand(time(NULL));
	id = rand();
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	memset(times_burst, -1, sizeof(times_burst));
	target_route = argv[1];
	if (type == HOSTNAME) {
		target_route = hostname_to_ip(target_route);
		printf("Resolving hostname (\e[35m%s\e[0m) to IPv4 \e[34m%s\e[0m\n\n", argv[1], target_route);
	}
}

int main(int argc, char* argv[]) {
	int ttl = 0, last_ttl = 0, count = 0;
	long long start, end = 0;
	char* route = NULL;
	bool success = false;
	enum input_type type;
	// Guard check we can safely proceed to main loop
	if (!guard(sockfd, argc, argv, &type)) return EXIT_FAILURE;
	// Initialize
	init(type, argv);
	// Main loop
	while (true) {
		if (count == 0) {
			// Once we've received all responses for a burst,
			// print the route and calculate the average time
			if (route != NULL) {
				int ms = calc_avg_time(times_burst);
				// Print the route and average time
				set_color(get_color(colors, ttl));
				printf("%17s", route);
				if (ms == -1) printf("\t(\?\?\?)");
				else printf("  (%4d ms)", ms);
				// Get hostname if possible
				char* hostname = ip_to_hostname(route);
				if (hostname != NULL) {
					printf("\t%s", hostname);
				}
				printf("\n");
				clear_color();
				// We have reached the target
				if (strcmp(route, target_route) == 0) {
					success = true;
					break;
				}
				free(route);
				route = NULL;
			}
			// Transmit a new burst
			burst_transmit(sockfd, target_route, id, ++ttl);
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
			char star[] = "*.*.*.*";
			int color = 243 - 2 * (ttl - last_ttl);
			printf("\x1b[38;5;%dm%17s  ( --- ms)\e[0m\n", color, star);
			// Reset the burst
			count = 0;
		}
	}
	// Print the resulting outcome of the traceroute
	if (success) printf("\nSuccess 🎉\e[0m\n");
	else printf("\nFailed to connect\e[0m\n");
	return EXIT_SUCCESS;
}