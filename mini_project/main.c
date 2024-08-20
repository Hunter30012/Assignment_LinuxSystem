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

#define DEFAULT_NUM_THREAD 3
#define SPEEDTEST_DOMAIN_NAME "www.speedtest.net"
#define CONFIG_REQUEST_URL "speedtest-config.php"
#define FILE_HTTP "my_http_file.txt"

#define SPEEDTEST_SERVERS_DOMAIN_NAME "c.speedtest.net"
#define SERVERS_LOCATION_REQUEST_URL "speedtest-servers-static.php?"

#define FILE_DIRECTORY_PATH "/tmp/"
#define NEAREST_SERVERS_NUM 5
#define THREAD_NUMBER 4
#define SPEEDTEST_DURATION 15

#define UL_BUFFER_SIZE 8192
#define UL_BUFFER_TIMES 10240
#define DL_BUFFER_SIZE 8192

static pthread_mutex_t pthread_mutex = PTHREAD_MUTEX_INITIALIZER; 

typedef struct client_data {
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

typedef struct thread {
    int thread_index;
    int running;                                                                                                              
    pthread_t tid;
    char domain_name[128];
    char request_url[128];
    struct sockaddr_in servinfo;
} thread_t;

int get_ipv4_addr(char *domain_name, struct sockaddr_in *servinfo) {
    struct addrinfo hints, *addrinfo, *p;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    if((status = getaddrinfo(domain_name, "http", &hints, &addrinfo)) != 0) {
        printf("Resolve DNS Failed: Can't get ip address! (%s)\n", domain_name);
        return 0;
    }

    for(p=addrinfo; p!=NULL; p=p->ai_next) {
        if(p->ai_family == AF_INET) {
            memcpy(servinfo, (struct sockaddr_in *)p->ai_addr, sizeof(struct sockaddr_in));
            printf("server infor: %s:%d\n", inet_ntoa(servinfo->sin_addr), ntohs(servinfo->sin_port));
        }
    }
    freeaddrinfo(addrinfo);
    return 1;
}

int get_http_file(struct sockaddr_in *serv, char *domain_name, char *request_url, char *filename) {
    int fd;
    char sbuf[256]={0}, tmp_path[128]={0};
    char rbuf[8192];
    struct timeval tv;
    fd_set fdSet;
    FILE *fp=NULL;

    if((fd = socket(serv->sin_family, SOCK_STREAM, 0)) == -1) {
        perror("Open socket error!\n");
        if(fd) close(fd);
        return 0;
    }
    if(connect(fd, (struct sockaddr *)serv, sizeof(struct sockaddr)) == -1) {
        perror("Socket connect error!\n");
        if(fd) close(fd);
        return 0;
    }

    sprintf(sbuf,
            "GET /%s HTTP/1.0\r\n"
            "Host: %s\r\n"
            "User-Agent: status\r\n"
            "Accept: */*\r\n\r\n", request_url, domain_name);                                                                 

    if(send(fd, sbuf, strlen(sbuf), 0) != strlen(sbuf)) {
        perror("Can't send data to server\n");
        if(fd) close(fd);
        return 0;
    }

    sprintf(tmp_path, "%s%s", FILE_DIRECTORY_PATH, filename);
    fp = fopen(tmp_path, "w+");

    while(1) {
        char *ptr=NULL;
        memset(rbuf, 0, sizeof(rbuf));
        FD_ZERO(&fdSet);
        FD_SET(fd, &fdSet);

        tv.tv_sec = 3;
        tv.tv_usec = 0;
        int status = select(fd + 1, &fdSet, NULL, NULL, &tv);
        int i = recv(fd, rbuf, sizeof(rbuf), 0);
        if(status > 0 && FD_ISSET(fd, &fdSet)) {
            if(i < 0) {
                printf("Can't get http file!\n");
                close(fd);
                fclose(fp);
                return 0;
            } else if(i == 0) {
                break;
            } else {
                if((ptr = strstr(rbuf, "\r\n\r\n")) != NULL) {
                    ptr += 4;
                    fwrite(ptr, 1, strlen(ptr), fp);
                } else {
                    fwrite(rbuf, 1, i, fp);
                }
            }
        }
    }
    close(fd);
    fclose(fp);
    return 1;
}

int get_nearest_server(double lat_c, double lon_c, server_data_t *nearest_servers) {
    FILE *fp=NULL;
    char filePath[128]={0}, line[512]={0}, url[128]={0}, lat[64]={0}, lon[64]={0}, name[128]={0}, country[128]={0};
    double lat_s, lon_s, distance;
    int count=0, i=0, j=0;

    sprintf(filePath, "%s%s", FILE_DIRECTORY_PATH, SERVERS_LOCATION_REQUEST_URL);

    if((fp=fopen(filePath, "r"))!=NULL) {
        while(fgets(line, sizeof(line)-1, fp)!=NULL) {
            if(!strncmp(line, "<server", 7)) {
                //ex: <server url="http://88.84.191.230/speedtest/upload.php" lat="70.0733" lon="29.7497" name="Vadso" country="Norway" cc="NO" sponsor="Varanger KraftUtvikling AS" id="4600" host="88.84.191.230:8080"/>
                sscanf(line, "%*[^\"]\"%255[^\"]\"%*[^\"]\"%255[^\"]\"%*[^\"]\"%255[^\"]\"%*[^\"]\"%255[^\"]\"%*[^\"]\"%255[^\"]\"", url, lat, lon, name, country);

                lat_s = atof(lat);
                lon_s = atof(lon);

                // distance = calcDistance(lat_c, lon_c, lat_s, lon_s);

                for(i=0; i<NEAREST_SERVERS_NUM; i++) {
                    if(nearest_servers[i].url[0] == '\0') {
                        strncpy(nearest_servers[i].url, url, sizeof(url));
                        nearest_servers[i].latitude = lat_s;
                        nearest_servers[i].longitude = lon_s;
                        strncpy(nearest_servers[i].name, name, sizeof(name)); 
                        strncpy(nearest_servers[i].country, country, sizeof(country)); 
                        nearest_servers[i].distance = distance;
                        break;
                    } else {
                        if(distance <= nearest_servers[i].distance) {
                            memset(&nearest_servers[NEAREST_SERVERS_NUM-1], 0, sizeof(server_data_t));
                            for(j=NEAREST_SERVERS_NUM-1; j>i; j--) {
                                strncpy(nearest_servers[j].url, nearest_servers[j-1].url, sizeof(nearest_servers[j-1].url));
                                nearest_servers[j].latitude = nearest_servers[j-1].latitude;
                                nearest_servers[j].longitude = nearest_servers[j-1].longitude;
                                strncpy(nearest_servers[j].name, nearest_servers[j-1].name, sizeof(nearest_servers[j-1].name));
                                strncpy(nearest_servers[j].country, nearest_servers[j-1].country, sizeof(nearest_servers[j-1].country));
                                nearest_servers[j].distance = nearest_servers[j-1].distance;
                            }
                            strncpy(nearest_servers[i].url, url, sizeof(url));
                            nearest_servers[i].latitude = lat_s;
                            nearest_servers[i].longitude = lon_s;
                            strncpy(nearest_servers[i].name, name, sizeof(name)); 
                            strncpy(nearest_servers[i].country, country, sizeof(country)); 
                            nearest_servers[i].distance = distance;
                            break;
                        } 
                    }
                }
                count++;
            } 
        }
    }
    if(count>0)
        return 1;
    else 
        return 0;
}

int get_best_server(server_data_t *nearest_servers) {
    FILE *fp=NULL;
    int i=0, latency, best_index=-1;
    char latency_name[16]="latency.txt";
    char latency_file_string[16]="test=test";
    char latency_url[NEAREST_SERVERS_NUM][128], latency_request_url[128];
    char url[128], buf[128], filePath[64]={0}, line[64]={0};
    struct timeval tv1, tv2;
    struct sockaddr_in servinfo;

    sprintf(filePath, "%s%s", FILE_DIRECTORY_PATH, latency_name);

    //Generate request url for latency
    for(i=0; i<NEAREST_SERVERS_NUM; i++) {
        char *ptr=NULL;
        memset(latency_url[i], 0, sizeof(latency_url[i]));
        strncpy(url, nearest_servers[i].url, sizeof(nearest_servers[i].url));

        ptr = strtok(url, "/");
        while(ptr!=NULL) {
            memset(buf, 0, sizeof(buf));    
            strncpy(buf, ptr, strlen(ptr));

            //Change file name to "latency.txt"
            if(strstr(buf, "upload.")!=NULL) {
                strcat(latency_url[i], latency_name);
            } else {
                strcat(latency_url[i], buf);
                strcat(latency_url[i], "/");
            }

            if(strstr(buf, "http:")) 
                strcat(latency_url[i], "/");

            ptr = strtok(NULL, "/");
        }

        //Get domain name
        sscanf(latency_url[i], "http://%[^/]", nearest_servers[i].domain_name);

        //Get request url
        memset(latency_request_url, 0, sizeof(latency_request_url));
        if((ptr = strstr(latency_url[i], nearest_servers[i].domain_name)) != NULL) {
            ptr += strlen(nearest_servers[i].domain_name);
            strncpy(latency_request_url, ptr, strlen(ptr));
        }

        if(get_ipv4_addr(nearest_servers[i].domain_name, &servinfo)) {
            memcpy(&nearest_servers[i].servinfo, &servinfo, sizeof(struct sockaddr_in));

            //Get latency time
            gettimeofday(&tv1, NULL);
            get_http_file(&servinfo, nearest_servers[i].domain_name, latency_request_url, latency_name);
            gettimeofday(&tv2, NULL);
        }

        if((fp=fopen(filePath, "r")) !=NULL) {
            fgets(line, sizeof(line), fp);
            if(!strncmp(line, latency_file_string, strlen(latency_file_string)))
                nearest_servers[i].latency = (tv2.tv_sec - tv1.tv_sec) * 1000000 + tv2.tv_usec - tv1.tv_usec;
            else
                nearest_servers[i].latency = -1;

            fclose(fp);
            unlink(filePath);
        } else {
            nearest_servers[i].latency = -1;
        }
    }

    //Select low latency server
    for(i=0; i<NEAREST_SERVERS_NUM; i++) {
        if(i == 0) {
            best_index = i;
            latency = nearest_servers[i].latency;
        } else {
            if(nearest_servers[i].latency < latency && nearest_servers[i].latency != -1) {
                best_index = i;
                latency = nearest_servers[i].latency;
            }           
        }
    }
    return best_index;
}



int main()
{
    struct sockaddr_in server_info;
    client_data_t client_data;


    memset(&client_data, 0, sizeof(client_data_t));
    get_ipv4_addr(SPEEDTEST_DOMAIN_NAME, &server_info);
    get_ipv4_addr(SPEEDTEST_SERVERS_DOMAIN_NAME, &server_info);
    while (1)
    {
        /* code */
    }
    
}
