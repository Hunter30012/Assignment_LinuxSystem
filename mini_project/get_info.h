#ifndef __GET_INFO__
#define __GET_INFO__

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define DEBUG _debug_
#define DEBUG_PRINT(fmt, ...) \
            do { if (DEBUG) fprintf(stdout, fmt, __VA_ARGS__); } while (0)
#define LOG(fmt) \
            do { if (DEBUG) printf(fmt); } while (0)
#define FILE_DIRECTORY_PATH "/tmp/"

typedef struct _client_data {
    char ipAddr[128];
    double latitude;
    double longitude;
    char isp[128];
} client_data_t;

typedef struct server_data {
    char url[128];
    double latitude;
    double longitude;
    char name[128];
    char country[128];
    double distance;
    int latency;
    char domain_name[128];
    struct sockaddr_in servinfo;
} server_data_t;

int get_ipv4_addr(char *domain_name, struct sockaddr_in *servinfo);

int get_http_file(struct sockaddr_in *serv, char *domain_name, char *request_url, char *filename);

int get_ip_address_position(char *fileName, client_data_t *client_data);

int get_nearest_server(double lat_c, double lon_c, server_data_t *nearest_servers, uint8_t num_server);

int get_best_server(server_data_t *nearest_servers, uint8_t number_server);


#endif