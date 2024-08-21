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
#include <unistd.h>
#include "get_info.h"
#include "speed_test.h"

#define DEFAULT_NUM_THREAD 3


#define THREAD_NUMBER 4
#define SPEEDTEST_DURATION 15

#define UL_BUFFER_SIZE 8192
#define UL_BUFFER_TIMES 10240
#define DL_BUFFER_SIZE 8192

float start_dl_time, stop_dl_time, start_ul_time, stop_ul_time;
int thread_all_stop = 0;
long int total_dl_size = 0, total_ul_size = 0;

#define NEAREST_SERVERS_NUM 5

static pthread_mutex_t pthread_mutex = PTHREAD_MUTEX_INITIALIZER; 


typedef struct thread {
    int thread_index;
    int running;                                                                                                              
    pthread_t tid;
    char domain_name[128];
    char request_url[128];
    struct sockaddr_in servinfo;
} thread_t;

thread_t thread[THREAD_NUMBER];

int _debug_ = 0;

void stop_all_thread(int signo) {
    if(signo == SIGALRM) {
        thread_all_stop=1;
    }
    return;
}

float get_uptime(void) {
    FILE* fp;
    float uptime, idle_time;

    if((fp = fopen("/proc/uptime", "r"))!=NULL) {
        fscanf (fp, "%f %f\n", &uptime, &idle_time);
        fclose (fp);
        return uptime;
    }
    return -1;
}

void *calculate_ul_speed_thread() {
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

void *calculate_dl_speed_thread() {
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

void *download_thread(void *arg) {
    thread_t *t_arg = arg;
    int i = t_arg->thread_index;

    int fd;
    char sbuf[256]={0}, rbuf[DL_BUFFER_SIZE];
    struct timeval tv;
    fd_set fdSet;

    if((fd = socket(thread[i].servinfo.sin_family, SOCK_STREAM, 0)) == -1) {                                                  
        perror("Open socket error!\n");
        goto err;
    }

    if(connect(fd, (struct sockaddr *)&thread[i].servinfo, sizeof(struct sockaddr)) == -1) {
        perror("Socket connect error!\n");
        goto err;
    }

    sprintf(sbuf,
            "GET /%s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "User-Agent: status\r\n"
            "Accept: */*\r\n\r\n", thread[i].request_url, thread[i].domain_name);

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

            if(thread_all_stop)
                break;
        }   
    }

err: 
    if(fd) close(fd);
    thread[i].running=0;
    return NULL;
}

int speedtest_download(server_data_t *nearest_server) {
    const char download_filename[64]="random3500x3500.jpg";  //23MB
    char url[128]={0}, request_url[128]={0}, dummy[128]={0}, buf[128];
    char *ptr=NULL;
    int i;

    sscanf(nearest_server->url, "http://%[^/]/%s", dummy, request_url);
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

    start_dl_time = get_uptime();
    while(1) {
        for(i=0; i<THREAD_NUMBER; i++) {
            memcpy(&thread[i].servinfo, &nearest_server->servinfo, sizeof(struct sockaddr_in));
            memcpy(&thread[i].domain_name, &nearest_server->domain_name, sizeof(nearest_server->domain_name));
            memcpy(&thread[i].request_url, request_url, sizeof(request_url));
            if(thread[i].running == 0) {
                thread[i].thread_index = i;
                thread[i].running = 1;
                pthread_create(&thread[i].tid, NULL, download_thread, &thread[i]);
            }
        }
        if(thread_all_stop)
            break;
    }
    return 1;
}

void *upload_thread(void *arg) {
    int fd;
    char data[UL_BUFFER_SIZE], sbuf[512];
    uint32_t i, j, size=0;
    struct timeval tv;
    fd_set fdSet;

    thread_t *t_arg = arg;
    i = t_arg->thread_index;

    memset(data, 0, sizeof(char) * UL_BUFFER_SIZE);

    if((fd = socket(thread[i].servinfo.sin_family, SOCK_STREAM, 0)) == -1) {                                                  
        perror("Open socket error!\n");
        goto err;
    }

    if(connect(fd, (struct sockaddr *)&thread[i].servinfo, sizeof(struct sockaddr)) == -1) {
        printf("Socket connect error!\n");
        goto err;
    }

    sprintf(sbuf,
            "POST /%s HTTP/1.0\r\n"
            "Content-type: application/x-www-form-urlencoded\r\n"
            "Host: %s\r\n"
            "Content-Length: %ld\r\n\r\n", thread[i].request_url, thread[i].domain_name, sizeof(data)*UL_BUFFER_TIMES);

    if((size=send(fd, sbuf, strlen(sbuf), 0)) != strlen(sbuf)) {
        printf("Can't send header to server\n");
        goto err;
    }

    pthread_mutex_lock(&pthread_mutex);
    total_ul_size+=size;
    pthread_mutex_unlock(&pthread_mutex);

    for(j=0; j<UL_BUFFER_TIMES; j++) {
        // if((size=send(fd, data, sizeof(data), 0)) != sizeof(data)) {
        //     // printf("Can't send data to server\n");
        //     // goto err;
        // }
        size = send(fd, data, sizeof(data), 0);
        pthread_mutex_lock(&pthread_mutex);
        total_ul_size+=size;
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
    if(fd) close(fd);
    thread[i].running=0;
    return NULL;
}

int speedtest_upload(server_data_t *nearest_server) {
    int i;
    char dummy[128]={0}, request_url[128]={0};
    sscanf(nearest_server->url, "http://%[^/]/%s", dummy, request_url);

    start_ul_time = get_uptime();
    while(1) {
        for(i=0; i<THREAD_NUMBER; i++) {
            memcpy(&thread[i].servinfo, &nearest_server->servinfo, sizeof(struct sockaddr_in));
            memcpy(&thread[i].domain_name, &nearest_server->domain_name, sizeof(nearest_server->domain_name));
            memcpy(&thread[i].request_url, request_url, sizeof(request_url));
            if(thread[i].running == 0) {
                thread[i].thread_index = i;
                thread[i].running = 1;
                pthread_create(&thread[i].tid, NULL, upload_thread, &thread[i]);
            }
        }
        if(thread_all_stop)
            break;
    }
    return 1;
}

int main(int argc, char **argv)
{
    int best_server_index;
    pthread_t pid;
    struct sockaddr_in server_info;
    client_data_t client_data;
    server_data_t list_servers[NEAREST_SERVERS_NUM];
    struct itimerval timerVal;

    if(argc == 2) {
        if(strcmp(argv[1], "--debug") ==0) {
            _debug_ = 1;
        }
    }
    memset(&timerVal, 0, sizeof(struct itimerval));
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

        //Set speed test timer
        signal(SIGALRM, stop_all_thread);
        timerVal.it_value.tv_sec = SPEEDTEST_DURATION;
        timerVal.it_value.tv_usec = 0;
        setitimer(ITIMER_REAL, &timerVal, NULL);

        // for download
        pthread_create(&pid, NULL, calculate_dl_speed_thread, NULL);
        speedtest_download(&list_servers[best_server_index]);

        sleep(2);
        printf("\n");
        thread_all_stop=0;
        setitimer(ITIMER_REAL, &timerVal, NULL);

        // for upload
        pthread_create(&pid, NULL, calculate_ul_speed_thread, NULL);
        speedtest_upload(&list_servers[best_server_index]);
        printf("\n");

    }

    while (1)
    {
        /* code */
    }

    return 0;
}
