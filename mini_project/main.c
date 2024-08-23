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
    DEFAULT_OP,
    DOWNLOAD,
    NUM_THREAD,
    SELECT_SERVER,
    DEBUG_OP,
    HTTPS_OP
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
              "--debug - Run and list 7 servers found\r\n"
    );
}

int main(int argc, char **argv)
{
    optiont_t op = DEFAULT_OP;
    int best_server_index, num_thread = DEFAULT_NUM_THREAD;
    struct sockaddr_in server_info;
    client_data_t client_data;
    server_data_t list_servers[NEAREST_SERVERS_NUM];
    server_data_t select_server;
    op_protocol_t type_pro = HTTP_PROTOCOL;
    const char *domain_name;

    if(argc > 1) {
        if(strcmp(argv[1], "--debug") ==0) {
            _debug_ = 1;
            op = DEBUG_OP;
        }
        else if(strcmp(argv[1], "--download") ==0) {
            op = DOWNLOAD;
        }
        else if(strcmp(argv[1], "--upload") ==0) {
            op = UPLOAD;
        }
        else if(strcmp(argv[1], "--https") ==0) {
            op = HTTPS_OP;
            type_pro = HTTPS_PROTOCOL;
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
        else if(strcmp(argv[1], "--server") ==0) {
            op = SELECT_SERVER;
            if(argv[2] != NULL) {
                domain_name = argv[2];
            }
        }
        else {
            printf("Please use --help to know how to use this app!\n");
            return 0;
        }  
    }
    memset(&client_data, 0, sizeof(client_data_t));

    // get information of client
    if(get_ipv4_addr(SPEEDTEST_DOMAIN_NAME, &server_info, HTTP_PROTOCOL)) {
        if(!get_http_https_file(&server_info, SPEEDTEST_DOMAIN_NAME, CONFIG_REQUEST_URL, CONFIG_REQUEST_URL)) {
            printf("Can't get your IP address information.\n");
            return 0;
        }
    }

    // Get information of servers list
    if(get_ipv4_addr(SPEEDTEST_SERVERS_DOMAIN_NAME, &server_info, type_pro)) {
        if(!get_http_https_file(&server_info, SPEEDTEST_SERVERS_DOMAIN_NAME, SERVERS_LOCATION_REQUEST_URL, SERVERS_LOCATION_REQUEST_URL)) {
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
        printf("Can't get ip port server list.\n"); 
        return 0;
    }

    if((best_server_index = get_best_server(list_servers, NEAREST_SERVERS_NUM, type_pro)) != -1) {
        printf("==========The best server information==========\n");
        printf("URL: %s\n", list_servers[best_server_index].url);
        printf("Latitude: %lf, Longitude: %lf\n", list_servers[best_server_index].latitude, list_servers[best_server_index].longitude);
        printf("Name: %s\n", list_servers[best_server_index].name);
        printf("Country: %s\n", list_servers[best_server_index].country);
        printf("Distance: %lf (km)\n", list_servers[best_server_index].distance);
        printf("Latency: %d (us)\n", list_servers[best_server_index].latency);
        printf("===============================================\n");

        switch (op) {
        case NUM_THREAD:
            speed_test_download(&list_servers[best_server_index], NULL, num_thread, type_pro);
            sleep(13);
            printf("\r\n");
            speed_test_upload(&list_servers[best_server_index], NULL, num_thread, type_pro);
            break;
        case SELECT_SERVER:
            if(get_server_use_domain(&select_server, domain_name, type_pro)) {
                speed_test_download(NULL, &select_server, num_thread, type_pro);
                sleep(13);
                printf("\r\n");
                speed_test_upload(NULL, &select_server, num_thread, type_pro);
            } else {
                return 0;
            }
            break;
        case HTTPS_OP:
        case DEFAULT_OP:
        case DOWNLOAD:
            speed_test_download(&list_servers[best_server_index], NULL, DEFAULT_NUM_THREAD, type_pro);
            if(op == DOWNLOAD) 
                break;
            sleep(13);
        case UPLOAD:
            speed_test_upload(&list_servers[best_server_index], NULL, DEFAULT_NUM_THREAD, type_pro);
            if(op == UPLOAD) 
                break;
        default:
            break;
        }
        printf("\r\n");
    }
    

    return 0;
}
