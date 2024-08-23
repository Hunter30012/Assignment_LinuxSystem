#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>
#include <unistd.h>
#include <openssl/ssl.h>
#include "get_info.h"
#include "speed_test.h"

float start_dl_time, stop_dl_time, start_ul_time, stop_ul_time;
int thread_all_stop = 0;
long int total_dl_size = 0, total_ul_size = 0;

static pthread_mutex_t pthread_mutex = PTHREAD_MUTEX_INITIALIZER; 

static void stop_all_thread(int signo) 
{
    if(signo == SIGALRM) {
        thread_all_stop = 1;
    }
    return;
}

static float get_uptime(void) 
{
    FILE* fp;
    float uptime, idle_time;

    if((fp = fopen("/proc/uptime", "r"))!=NULL) {
        fscanf (fp, "%f %f\n", &uptime, &idle_time);
        fclose (fp);
        return uptime;
    }
    return -1;
}

static void *calculate_ul_speed_thread() 
{
    double ul_speed=0.0, duration=0;
    while(1) {
        stop_ul_time = get_uptime();
        duration = stop_ul_time-start_ul_time;
        //ul_speed = (double)total_ul_size/1024/1024/duration*8;
        ul_speed = (double)total_ul_size/1000/1000/duration*8;
        if(duration>0) {
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bUpload speed: %0.2lf Mbps", ul_speed);
            fflush(stdout);
        }
        usleep(500000);

        if(thread_all_stop) {
            stop_ul_time = get_uptime();
            duration = stop_ul_time-start_ul_time;
            //ul_speed = (double)total_ul_size/1024/1024/duration*8;
            ul_speed = (double)total_ul_size/1000/1000/duration*8;
            if(duration) {
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bUpload speed: %0.2lf Mbps", ul_speed);
                fflush(stdout);
            }
            break;
        }
    }
    return NULL;
}

static void *calculate_dl_speed_thread() 
{
    double dl_speed=0.0, duration=0;
    while(1) {
        stop_dl_time = get_uptime();
        duration = stop_dl_time-start_dl_time;
        //dl_speed = (double)total_dl_size/1024/1024/duration*8;
        dl_speed = (double)total_dl_size/1000/1000/duration*8;
        if(duration>0) {
            printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bDownload speed: %0.2lf Mbps", dl_speed);
            fflush(stdout);
        }
        usleep(500000);

        if(thread_all_stop) {
            stop_dl_time = get_uptime();
            duration = stop_dl_time-start_dl_time;
            //dl_speed = (double)total_dl_size/1024/1024/duration*8;
            dl_speed = (double)total_dl_size/1000/1000/duration*8;
            if(duration>0) {
                printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\bDownload speed: %0.2lf Mbps", dl_speed);
                fflush(stdout);
            }   
            break;
        }
    }
    return NULL;
}

void *download_thread(void *arg) 
{
    thread_t *t_arg = (thread_t *)arg;
    int fd;
    char sbuf[256]={0}, rbuf[DL_BUFFER_SIZE];
    struct timeval tv;
    fd_set fdSet;
    pthread_detach(pthread_self());
    if((fd = socket(t_arg->servinfo.sin_family, SOCK_STREAM, 0)) == -1) {                                                  
        perror("Open socket error!\n");
        goto err;
    }

    if(connect(fd, (struct sockaddr *)&t_arg->servinfo, sizeof(struct sockaddr)) == -1) {
        perror("Socket connect error!\n");
        goto err;
    }

    sprintf(sbuf,
            "GET /%s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "User-Agent: status\r\n"
            "Accept: */*\r\n\r\n", t_arg->request_url, t_arg->domain_name);

    if(send(fd, sbuf, strlen(sbuf), 0) != strlen(sbuf)) {
        perror("Can't send data to server\n");
        goto err;
    }

    while(1) {
        FD_ZERO(&fdSet);
        FD_SET(fd, &fdSet);

        tv.tv_sec = 3;
        tv.tv_usec = 0;
        int status = select(fd + 1, &fdSet, NULL, NULL, &tv);                                                         

        int recv_byte = recv(fd, rbuf, sizeof(rbuf), 0);
        if(status > 0 && FD_ISSET(fd, &fdSet)) {
            if(recv_byte < 0) {
                printf("Can't receive data!\n");
                break;
            } else if(recv_byte == 0){
                break;
            } else {
                pthread_mutex_lock(&pthread_mutex);
                total_dl_size+=recv_byte;
                pthread_mutex_unlock(&pthread_mutex);
            }

            if(thread_all_stop) {
                // printf("Thread terminate\n");
                break;
            }
        }   
    }

err: 
    if(fd) close(fd);
    t_arg->running=0;
    return NULL;
}

int speedtest_download(server_data_t *nearest_server, int num_thread, op_protocol_t protocol) 
{
    thread_t *thread;
    const char download_filename[64]="random3500x3500.jpg";  //23MB
    char url[128]={0}, request_url[128]={0}, dummy[128]={0}, buf[128];
    char *ptr=NULL;
    int i;
    if(protocol == HTTP_PROTOCOL)
        sscanf(nearest_server->url, "http://%[^/]/%s", dummy, request_url);
    else if(protocol == HTTPS_PROTOCOL)
        sscanf(nearest_server->url, "https://%[^/]/%s", dummy, request_url);

    strncpy(url, request_url, sizeof(request_url));
    memset(request_url, 0, sizeof(request_url));

    ptr = strtok(url, "/");
    while(ptr!=NULL) {
        memset(buf, 0, sizeof(buf));
        strncpy(buf, ptr, strlen(ptr));

        //Change file name
        if(strstr(buf, "upload.")!=NULL) {
            strcat(request_url, download_filename);
        } else {
            strcat(request_url, buf);
            strcat(request_url, "/");
        }
        ptr = strtok(NULL, "/");
    }
    thread = (thread_t *)calloc(num_thread, sizeof(thread_t));
    memset(thread, 0, sizeof(thread_t) * num_thread);
    start_dl_time = get_uptime();
    while(1) {
        for(i = 0; i < num_thread; i++) {
            memcpy(&thread[i].servinfo, &nearest_server->servinfo, sizeof(struct sockaddr_in));
            memcpy(&thread[i].domain_name, &nearest_server->domain_name, sizeof(nearest_server->domain_name));
            memcpy(&thread[i].request_url, request_url, sizeof(request_url));
            thread->protocol = protocol;
            if(thread[i].running == 0) {
                thread[i].thread_index = i;
                thread[i].running = 1;
                pthread_create(&thread[i].tid, NULL, download_thread, &thread[i]);
            }
        }
        if(thread_all_stop) {
            // free(thread);
            break;
        }
    }
    return 1;
}

void *upload_thread(void *arg) 
{
    int fd;
    char data[UL_BUFFER_SIZE], sbuf[512];
    uint32_t j, size=0;
    struct timeval tv;
    fd_set fdSet;

    thread_t *t_arg = (thread_t *)arg;
    pthread_detach(pthread_self());
    memset(data, 0, sizeof(char) * UL_BUFFER_SIZE);

    if((fd = socket(t_arg->servinfo.sin_family, SOCK_STREAM, 0)) == -1) {                                                  
        perror("Open socket error!\n");
        goto err;
    }
    if(connect(fd, (struct sockaddr *)&(t_arg->servinfo), sizeof(struct sockaddr)) == -1) {
        printf("Socket connect error!\n");
        goto err;
    }
    // printf("Request url: %s, domain name: %s, size: %ld\n", t_arg->request_url, t_arg->domain_name, sizeof(data)*UL_BUFFER_TIMES);
    sprintf(sbuf,
            "POST /%s HTTP/1.0\r\n"
            "Content-type: application/x-www-form-urlencoded\r\n"
            "Host: %s\r\n"
            "Content-Length: %ld\r\n\r\n", t_arg->request_url, t_arg->domain_name, sizeof(data)*UL_BUFFER_TIMES);

    if((size=send(fd, sbuf, strlen(sbuf), 0)) != strlen(sbuf)) {
        printf("Can't send header to server\n");
        goto err;
    }

    pthread_mutex_lock(&pthread_mutex);
    total_ul_size = total_ul_size + size;
    pthread_mutex_unlock(&pthread_mutex);

    for(j=0; j<UL_BUFFER_TIMES; j++) {
        if((size=send(fd, data, sizeof(data), 0)) != sizeof(data)) {
            printf("Can't send data to server\n");
            goto err;
        }
#if 0
        size_t remaining = sizeof(data);
        size_t total_sent = 0;

        while (remaining > 0) {
            ssize_t sent = send(fd, data + total_sent, remaining, 0);
            if (sent <= 0) {
                perror("Can't send data to server");
                goto err;
            }
            total_sent += sent;
            remaining -= sent;
        }
#endif
        pthread_mutex_lock(&pthread_mutex);
        total_ul_size = total_ul_size + size;
        pthread_mutex_unlock(&pthread_mutex);
        if(thread_all_stop)
            goto err;
    }
    while(1) {
        FD_ZERO(&fdSet);
        FD_SET(fd, &fdSet);

        tv.tv_sec = 3;
        tv.tv_usec = 0;
        int status = select(fd + 1, &fdSet, NULL, NULL, &tv);    

        int recv_byte = recv(fd, sbuf, sizeof(sbuf), 0);
        if(status > 0 && FD_ISSET(fd, &fdSet)) {
            if(recv_byte < 0) {
                printf("Can't receive data!\n");
                break;
            } else if(recv_byte == 0){
                break;
            }
        }   
    }
err: 
    // printf("Thread terminate\n");
    if(fd) close(fd);
    t_arg->running = 0;
    return NULL;
}

#if 0
void *setup_tcp_communication(void *arg) 
{
    char buffer[UL_BUFFER_SIZE];
    int sockfd = 0;
    struct sockaddr_in server_addr;
    int ret, sent_recv_bytes = 0;
    socklen_t addr_len = sizeof(struct sockaddr);
    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    // init infomation of server
    memset(&server_addr, 0, sizeof(struct sockaddr_in));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(default_port_no);
    if(inet_pton(AF_INET, default_ip_addr, &server_addr.sin_addr) == -1) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
    // connect to server
    ret = connect(sockfd, (struct sockaddr*)&server_addr, addr_len);
    if(ret == 0) {
        printf("Connected\n");
    } else {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    // communicate with server
    size_t total_sent = 0, remaining = sizeof(buffer);

    while (1) {
        total_sent = 0;
        remaining = sizeof(buffer);
        while (remaining > 0) {
            ssize_t sent = send(sockfd, buffer + total_sent, remaining, 0);
            if (sent < 0) {
                perror("Can't send data to server");
            }
            total_sent += sent;
            remaining -= sent;
        }
        pthread_mutex_lock(&pthread_mutex);
        total_ul_size = total_ul_size + total_sent;
        pthread_mutex_unlock(&pthread_mutex);

    }
    close(sockfd);
}
#endif
int speedtest_upload(server_data_t *nearest_server, int num_thread, op_protocol_t protocol) 
{
    thread_t *thread;
    int i;
    char dummy[128]={0}, request_url[128]={0};
    if(protocol == HTTP_PROTOCOL)
        sscanf(nearest_server->url, "http://%[^/]/%s", dummy, request_url);
    else if(protocol == HTTPS_PROTOCOL)
        sscanf(nearest_server->url, "https://%[^/]/%s", dummy, request_url);
    thread = (thread_t *)calloc(num_thread, sizeof(thread_t));
    memset(thread, 0, num_thread * sizeof(thread_t));
    start_ul_time = get_uptime();
    while(1) {
        for(i = 0 ; i < num_thread; i++) {
            memcpy(&thread[i].servinfo, &nearest_server->servinfo, sizeof(struct sockaddr_in));
            memcpy(&thread[i].domain_name, nearest_server->domain_name, sizeof(nearest_server->domain_name));
            memcpy(&thread[i].request_url, request_url, sizeof(request_url));
            thread->protocol = protocol;
            if(thread[i].running == 0) {
                thread[i].thread_index = i;
                thread[i].running = 1;
                pthread_create(&thread[i].tid, NULL, upload_thread, &thread[i]);
            }
        }
        if(thread_all_stop)
        {
            // free(thread);
            break;
        }
    }
    return 1;
}

void speed_test_download(server_data_t *best_server, server_data_t *select_server, int num_thread, op_protocol_t protocol)
{
    struct itimerval timerVal;
    pthread_t thread_cal;
    thread_all_stop = 0;
    signal(SIGALRM, stop_all_thread);
    printf("\n");
    memset(&timerVal, 0, sizeof(timerVal));
    timerVal.it_value.tv_sec = SPEEDTEST_DURATION;
    timerVal.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timerVal, NULL);

    pthread_create(&thread_cal, NULL, calculate_dl_speed_thread, NULL);
    if(select_server == NULL) {
        speedtest_download(best_server, num_thread, protocol);
    } else {
        speedtest_download(select_server, num_thread, protocol);
    }
    // pthread_join(thread_cal, NULL);    
}

void speed_test_upload(server_data_t *best_server, server_data_t *select_server, int num_thread, op_protocol_t protocol)
{
    struct itimerval timerVal;
    pthread_t thread_cal;
    thread_all_stop = 0;
    signal(SIGALRM, stop_all_thread);
    printf("\n");
    memset(&timerVal, 0, sizeof(timerVal));
    timerVal.it_value.tv_sec = SPEEDTEST_DURATION;
    timerVal.it_value.tv_usec = 0;
    setitimer(ITIMER_REAL, &timerVal, NULL);

    pthread_create(&thread_cal, NULL, calculate_ul_speed_thread, NULL);
    if(select_server == NULL) {
        speedtest_upload(best_server, num_thread, protocol);
    } else {
        speedtest_upload(select_server, num_thread, protocol);
    }
    // pthread_join(thread_cal, NULL);
}
