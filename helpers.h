#ifndef HELPERS_H
#define HELPERS_H

#define PROBES 3
#define COLORS 20

#include <sys/time.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

struct response {
	unsigned short id;
	short time;
};

enum input_type {
	IP,
	HOSTNAME
};

// Calculate average time
int calc_avg_time(short times[PROBES]);

// Function for sending burst of packets
void burst_transmit(int sockfd, char* target, int id, int ttl);

// Function for measuring miliseconds
long long current_timestamp();

// Return color for given route
int get_color(int* colors, int ttl);

// Gruard function for main
bool guard(int sockfd, int argc, char* argv[], enum input_type* type);

// Set bash color
void set_color(int color);

// Clear bash color
void clear_color(void);

// Check if given string is a valid IPv4 address
bool is_valid_target(char* ip, enum input_type* type);

// Convert hostname to IP address
char *hostname_to_ip(const char *hostname);

// Convert IP address to hostname
char *ip_to_hostname(const char *ip_address);

// Check if given route is new in the array or not
bool is_new_route(char* routes[PROBES], char* route);

// Check if given array is empty
bool is_routes_empty(char* routes[PROBES]);

// Add new route to the array
void store_route(char* routes[PROBES], char* route);

#endif
