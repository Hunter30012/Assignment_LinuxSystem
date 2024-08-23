#ifndef __SEED_TEST__
#define __SEED_TEST__

#include <pthread.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "get_info.h"
#include "macro_sp.h"

#define DEFAULT_NUM_THREAD 3

#define SPEEDTEST_DURATION 10

#define UL_BUFFER_SIZE 8192
#define UL_BUFFER_TIMES 10240
#define DL_BUFFER_SIZE 8192

typedef struct thread {
    int thread_index;
    int running;                                                                                                              
    pthread_t tid;
    char domain_name[128];
    char request_url[128];
    struct sockaddr_in servinfo;
    op_protocol_t protocol;
} thread_t;

void speed_test_download(server_data_t *best_server, server_data_t *select_server, int num_thread, op_protocol_t protocol);

void speed_test_upload(server_data_t *best_server, server_data_t *select_server, int num_thread, op_protocol_t protocol);


#endif