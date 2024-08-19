#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_MAX 128
uint16_t default_port_no = 5000;
const char *default_ip_addr = "127.0.0.2";

typedef struct _client
{
    int comm_fd;
    struct sockaddr_in addr_client;
} client_info_t ;

void *func_hanle_client(void *arg) 
{
    int len;
    char buffer[BUFFER_MAX];
    client_info_t *info_client = (client_info_t* )arg;
    int sock_fd = info_client->comm_fd;
    struct sockaddr_in addr_client = info_client->addr_client; 
    while (1)
    {
        memset(buffer, 0, BUFFER_MAX);
        socklen_t addr_client_len;
        if((len = recvfrom(sock_fd, buffer, BUFFER_MAX, 0,
                            (struct sockaddr *)&addr_client, 
                            &addr_client_len)) < 0) {
            perror("recvfrom");
            exit(EXIT_FAILURE);
        }
        printf("Receive %d bytes from %s:%d buffer: %s\n", 
                len, 
                inet_ntoa(addr_client.sin_addr), 
                ntohs(addr_client.sin_port), buffer);
        if(strcmp(buffer, "exit") == 0) {
            printf("Disconnected %s:%d\n", 
                    inet_ntoa(addr_client.sin_addr), 
                    ntohs(addr_client.sin_port));
            pthread_exit(NULL);
        }
        // send data back to client
        if(sendto(sock_fd, buffer, strlen(buffer) + 1, 0, 
                    (struct sockaddr *)&addr_client, 
                    addr_client_len) == -1) {
            perror("sendto");
            exit(EXIT_FAILURE);
        }
        printf("Send %d bytes to client %s:%d buffer: %s\n", 
                len, 
                inet_ntoa(addr_client.sin_addr), 
                ntohs(addr_client.sin_port), buffer);
    }
    close(sock_fd);
    return NULL;
}
int main()
{
    int sock_fd, len, comm_sock_fd, addr_client_len;
    pthread_t thread_handle;
    struct sockaddr_in addr_server, addr_client;
    client_info_t info_client;
    // create UDP socket
    if((sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // bind address to server
    memset(&addr_server, 0, sizeof(struct sockaddr_in));
    addr_server.sin_family = AF_INET;
    addr_server.sin_port = htons(default_port_no);
    if(inet_pton(AF_INET, default_ip_addr, &addr_server.sin_addr) == -1) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    if(bind(sock_fd, (struct sockaddr*)&addr_server, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    // listen for incoming connection
    if(listen(sock_fd, 10) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    printf("TCP Server is listening on port %d... \n", default_port_no);

    while (1) {
        // accept incoming client connection
        if((comm_sock_fd = accept(sock_fd,
                                 (struct sockaddr *)&addr_client, 
                                 &addr_client_len)) == -1) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("New client connected %s:%d\n",
                inet_ntoa(addr_client.sin_addr), 
                ntohs(addr_client.sin_port));
        // create handle_thread for client 
        info_client.comm_fd = comm_sock_fd;
        info_client.addr_client = addr_client;
        if(pthread_create(&thread_handle, NULL, 
                         func_hanle_client,
                         (void *)&info_client) == -1) {
            perror("pthread_create");
            exit(EXIT_FAILURE);
        }
        pthread_detach(thread_handle);
    }
    close(sock_fd);
    return 0;
}
