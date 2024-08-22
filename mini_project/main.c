#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include "get_info.h"
#include "macro_sp.h"
#include "speed_test.h"


typedef enum {
    UPLOAD = 1,
    DOWNLOAD,
    NUM_THREAD,
    SELECT_SERVER
} optiont_t;

int _debug_ = 0;

static void print_help(void)
{
    printf("Usage (options are case sensitive):\r\n"
              "No argument: Default - speed test using best server (lowest latancy)\r\n"
              "--help - Show this help\r\n"
              "--download - Run only download testing\r\n"
              "--upload - Run only upload testing\r\n"
              "--server URL - Test server URL instead of the lowest latancy\r\n"
              "--thread N - Run test with N threads\r\n"
    );
}

int main(int argc, char **argv)
{
    optiont_t op = 0;
    int best_server_index, num_thread = DEFAULT_NUM_THREAD;
    struct sockaddr_in server_info;
    client_data_t client_data;
    server_data_t list_servers[NEAREST_SERVERS_NUM];

    if(strcmp(argv[1], "--debug") ==0) {
        _debug_ = 1;
    }
    else if(strcmp(argv[1], "--download") ==0) {
        op = DOWNLOAD;
    }
    else if(strcmp(argv[1], "--upload") ==0) {
        op = UPLOAD;
    }
    else if(strcmp(argv[1], "--thread") ==0) {
        op = NUM_THREAD;
        if(argv[2] != NULL) {
            num_thread = atoi(argv[2]);
        }
    }
    else if(strcmp(argv[1], "--help") ==0) {
        print_help();
        return 0;
    }  
    memset(&client_data, 0, sizeof(client_data_t));

    // get information of client
    if(get_ipv4_addr(SPEEDTEST_DOMAIN_NAME, &server_info)) {
        if(!get_http_file(&server_info, SPEEDTEST_DOMAIN_NAME, CONFIG_REQUEST_URL, CONFIG_REQUEST_URL)) {
            printf("Can't get your IP address information.\n");
            return 0;
        }
    }

    // Get information of servers list
    if(get_ipv4_addr(SPEEDTEST_SERVERS_DOMAIN_NAME, &server_info)) {
        if(!get_http_file(&server_info, SPEEDTEST_SERVERS_DOMAIN_NAME, SERVERS_LOCATION_REQUEST_URL, SERVERS_LOCATION_REQUEST_URL)) {
            printf("Can't get servers list.\n");
            return 0;
        }
    }
    get_ip_address_position(CONFIG_REQUEST_URL, &client_data);
    printf("============================================\n");
    printf("Your IP Address : %s\n", client_data.ipAddr);
    printf("Your IP Location: %0.4lf, %0.4lf\n", client_data.latitude, client_data.longitude);
    printf("Your ISP        : %s\n", client_data.isp);
    printf("============================================\n");

    memset(list_servers, 0, sizeof(server_data_t) * NEAREST_SERVERS_NUM);
    if(get_nearest_server(client_data.latitude, client_data.longitude, list_servers, NEAREST_SERVERS_NUM)==0) {
        printf("Can't get server list.\n"); 
        return 0;
    }

    if((best_server_index = get_best_server(list_servers, NEAREST_SERVERS_NUM)) != -1) {
        printf("==========The best server information==========\n");
        printf("URL: %s\n", list_servers[best_server_index].url);
        printf("Latitude: %lf, Longitude: %lf\n", list_servers[best_server_index].latitude, list_servers[best_server_index].longitude);
        printf("Name: %s\n", list_servers[best_server_index].name);
        printf("Country: %s\n", list_servers[best_server_index].country);
        printf("Distance: %lf (km)\n", list_servers[best_server_index].distance);
        printf("Latency: %d (us)\n", list_servers[best_server_index].latency);
        printf("===============================================\n");

        if(op == DOWNLOAD) {
            speed_test_download(&list_servers[best_server_index], list_servers, NULL, DEFAULT_NUM_THREAD);
        }
        else if(op == UPLOAD) {
            speed_test_upload(&list_servers[best_server_index], list_servers, NULL, DEFAULT_NUM_THREAD);
        }
        else if(op == NUM_THREAD) {
            speed_test_download(&list_servers[best_server_index], list_servers, NULL, num_thread);
            printf("\n\n");
            speed_test_upload(&list_servers[best_server_index], list_servers, NULL, num_thread);
        }
        printf("\n\n");
    }
    

    return 0;
}
