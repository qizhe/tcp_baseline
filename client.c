// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <stdlib.h>     /* malloc, free, rand */
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <limits.h>
#include <unistd.h> 
#include <string.h> 
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>   
#include <sys/resource.h>


#define _GNU_SOURCE

#include "random_variable.h"
#define NUM_FLOW 5000
#define THREADSTACK (PTHREAD_STACK_MIN + 32768)

double diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp.tv_sec + temp.tv_nsec * 10e-9;
}

struct flow_info {
    int flow_id;
    int flow_size;
    char* server_addr;
    int server_port;
    struct timespec start_time;
};

bool send_flow(struct flow_info* f) {
    char *msg = (char*)malloc(f->flow_size * sizeof(char));
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    if(msg == NULL) {
        printf("heap is full\n");
    }
    memset(msg, '0', f->flow_size * sizeof(char));
    msg[f->flow_size - 1] = 'd';
    char* buffer[1] = {0}; 
    unsigned int sent_bytes = 0;
    while ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        free(msg);
        return false;
    } 

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(f->server_port); 
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, f->server_addr, &serv_addr.sin_addr)<=0) 
    { 
        printf("server addr:%s\n", f->server_addr);
        printf("\nInvalid address/ Address not supported \n"); 
    } 
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        fprintf(stderr,"server addr:%s\n", f->server_addr);

        fprintf(stderr, "\nConnection Failed \n"); 
            free(msg);
            close(sock);
            return false;

    }

    while(sent_bytes != f->flow_size) {
        int sent = send(sock, msg + sent_bytes, f->flow_size - sent_bytes, 0); 
        if(sent == -1) {
            fprintf(stderr, "socket error");
            fprintf(stderr, "socket() failed: %s\n", strerror(errno));
            free(msg);
            close(sock);
            return false;

        } else
            sent_bytes += sent;
    }
    // printf("flow:%d sent finish\n", f->flow_id); 
    valread = read( sock , buffer, 1);
    if(valread < 0){
        fprintf(stderr, "val read < 0\n");    
        fprintf(stderr, "socket() failed: %s\n", strerror(errno));
        free(msg);
        close(sock);
        return false;
    }
    if(valread == 0) {
        // flow finishes track info
        // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
        // struct timespec dif = diff(time1,time2);
    }
    close(sock);
    free(msg);
    return true;
}
void* start_client(void* info) {
    struct flow_info* f = (struct flow_info*)info;
    struct timespec time2;
    // clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    while(send_flow(f) == false) {
    }
    clock_gettime(CLOCK_REALTIME, &time2); 
    printf("flow %u flow_size:%d time: %f \n", f->flow_id, f->flow_size, diff(f->start_time, time2));
    free(f->server_addr);
    free(f);
}

// void set_cpu_affinity() {
//      int s, j;
//      cpu_set_t cpuset;
//        pthread_t thread;

//        thread = pthread_self();

//        /* Set affinity mask to include CPUs 0 */

//        CPU_ZERO(&cpuset);
//        for (j = 0; j < 1; j++)
//            CPU_SET(j, &cpuset);

//        s = pthread_setaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
//        if (s != 0)
//            handle_error_en(s, "pthread_setaffinity_np");

//        /* Check the actual affinity mask assigned to the thread */

//        s = pthread_getaffinity_np(thread, sizeof(cpu_set_t), &cpuset);
//        if (s != 0)
//            handle_error_en(s, "pthread_getaffinity_np");

//        printf("Set returned by pthread_getaffinity_np() contained:\n");
//        for (j = 0; j < CPU_SETSIZE; j++)
//            if (CPU_ISSET(j, &cpuset))
//                printf("    CPU %d\n", j);
// }
int main(int argc, char const *argv[]) 
{ 
    const char* cdf_file = argv[1];
    int index = atoi(argv[2]);
    int server_port = atoi(argv[3]);
    char** server_addrs = (char *[]){"5.0.0.10", "6.0.0.10", "7.0.0.10", "8.0.0.10", "9.0.0.10", "10.0.0.10", "11.0.0.10", "12.0.0.10"};
    // char** server_addrs = (char *[]){"5.0.0.10", "6.0.0.10"};
    // set_cpu_affinity();
    double bandwidth = 10000000000;
    double load = 0.5;
    struct exp_random_variable exp_r;
    struct empirical_random_variable emp_r;
    init_empirical_random_variable(&emp_r, cdf_file ,true);
    double lambda = bandwidth * load / (emp_r.mean_flow_size * 8.0 / 1460 * 1500);
    init_exp_random_variable(&exp_r, 1.0 / lambda);
    unsigned int flow_size = (uint32_t)(value_emp(&emp_r) + 0.5) * 1460;
    double time = value_exp(&exp_r);
    pthread_t threads[NUM_FLOW];
    pthread_attr_t  attrs;
    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, THREADSTACK);
    struct timespec offset1, offset2; 

    for (int i = 0; i < NUM_FLOW; i++) {

        // clock_gettime(CLOCK_REALTIME, &offset1); 

        usleep(time * 1000000);
        // clock_gettime(CLOCK_REALTIME, &offset2); 
        // printf("diff:%f\n", diff(offset1, offset2));
        // printf("time:%f\n", time);
        // continue;
        struct flow_info *f = malloc(sizeof(struct flow_info));
        // printf("flow %u size:%d\n", i, flow_size);
        f->flow_id = i;
        f->flow_size = flow_size;
        f->server_port = server_port;
        clock_gettime(CLOCK_REALTIME, &f->start_time);
        int addr_index = 0;
         while((addr_index = rand() % 8) == index) {

        }
        char* server_addr = server_addrs[addr_index];
        f->server_addr = malloc(strlen(server_addr) + 1);
        memcpy(f->server_addr, server_addr, strlen(server_addr) + 1);
        int thread_id = pthread_create(&threads[i],  &attrs, start_client, (void *)f);
        if(thread_id < 0) {
            printf("error number:%d\n", thread_id);
        }
        time = value_exp(&exp_r);
        flow_size = (uint32_t)(value_emp(&emp_r) + 0.5) * 1460;
        f = NULL;
   }
   pthread_attr_destroy(&attrs);

   pthread_exit(NULL);

} 

