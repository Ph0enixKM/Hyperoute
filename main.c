#include "socket.h"
#include "helpers.h"
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

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

int main(int argc, char* argv[]) {
	int ttl = 0, last_ttl = 0, count = 0;
	long long start, end = 0;
	char* routes[PROBES];
	memset(routes, 0, sizeof(routes));
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
			if (!is_routes_empty(routes)) {
				for (int r = 0; r < PROBES; r++) {
					if (routes[r] == NULL) continue;
					// Print the route and average time
					set_color(get_color(colors, ttl));
					if (r == 0) printf("%4d. %17s", ttl, routes[r]);
					else printf("      %17s", routes[r]);
					if (times_burst[r] == -1) printf("\t(\?\?\?)");
					else printf("  (%4hd ms)", times_burst[r]);
					// Get hostname if possible
					char* hostname = ip_to_hostname(routes[r]);
					if (hostname != NULL) {
						printf("  %s", hostname);
					}
					printf("\n");
					clear_color();
					// We have reached the target
					if (strcmp(routes[r], target_route) == 0) {
						success = true;
						break;
					}
					free(routes[r]);
					routes[r] = NULL;
				}
				if (success) break;
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
			bool is_id_valid = packet->id >= id && packet->id <= id + PROBES;
			bool is_seq_valid = id - (packet->seq + ttl) >= 0 && id - (packet->seq + ttl) < PROBES;
			if (is_id_valid && is_seq_valid) {
				// Store the time it took to receive the response
				times_burst[count - 1] = end - start;
				last_ttl = ttl;
				count--;
				// Store the route if we haven't already
				if (is_new_route(routes, packet->ip)) store_route(routes, packet->ip);
				else free(packet->ip);
			}
			free(packet);
		} else {
			// If we haven't received a response 
			// for the last 5 routers, stop probing
			if (ttl - last_ttl > 5) break;
			if (is_routes_empty(routes)) {
				// Print a star to indicate a timeout
				char star[] = "*.*.*.*";
				int color = 243 - 2 * (ttl - last_ttl);
				printf("\x1b[38;5;%dm%4d. %17s  ( --- ms)\e[0m\n", color, ttl, star);
			}
			// Reset the burst
			count = 0;
			memset(routes, 0, sizeof(routes));
		}
	}
	// Print the resulting outcome of the traceroute
	if (success) printf("\nSuccess 🎉\e[0m\n");
	else printf("\nFailed to connect\e[0m\n");
	return EXIT_SUCCESS;
}