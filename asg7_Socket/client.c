#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#define BUFFER_MAX 128
uint16_t default_port_no = 5000;
const char *default_ip_addr = "127.0.0.1";

void setup_tcp_communication() 
{
    char buffer[BUFFER_MAX], recv_buff[BUFFER_MAX];
    int sockfd = 0;
    struct sockaddr_in server_addr;
    int ret, sent_recv_bytes = 0;
    socklen_t addr_len = sizeof(struct sockaddr);
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == -1) {
        printf("Error to create socket\n");
        exit(EXIT_FAILURE);
    }
    // init infomation of server
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(default_port_no);
    if(inet_pton(AF_INET, default_ip_addr, &server_addr.sin_addr) == -1) {
        printf("Error in inet_pton\n");
        exit(EXIT_FAILURE);
    }
    // connect to server
    ret = connect(sockfd, (struct sockaddr*)&server_addr, addr_len);
    if(ret == 0) {
        printf("Connected\n");
    } else {
        printf("Failed to connect\n");
        exit(EXIT_FAILURE);
    }
    while (1) {
        memset(buffer, 0, sizeof(buffer));
        printf("Input data send to server: ");
        fflush(stdin);
        scanf("%s", buffer);

        /* send the data to server*/
        sent_recv_bytes = sendto(sockfd, buffer, strlen(buffer) + 1, 0, (struct sockaddr*)&server_addr, sizeof(struct sockaddr));
        printf("No of bytes sent = %d\n", sent_recv_bytes);
    
        /* Client also want to reply from server after sending data*/
        printf("Waiting for response:\n");
        sent_recv_bytes =  recvfrom(sockfd, recv_buff, BUFFER_MAX, 0, (struct sockaddr*)&server_addr, &addr_len);
        printf("No of bytes recvd = %d\n", sent_recv_bytes);
        printf("Buffer from server: %s", recv_buff);      
    } 

}

int main()
{
    setup_tcp_communication();
    printf("Client quits\n");
    return 0;
}