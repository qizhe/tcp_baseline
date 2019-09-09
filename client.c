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


#include "random_variable.h"
#define NUM_FLOW 5000
#define THREADSTACK (PTHREAD_STACK_MIN + 32768)

struct timespec diff(struct timespec start, struct timespec end)
{
    struct timespec temp;
    if ((end.tv_nsec-start.tv_nsec)<0) {
        temp.tv_sec = end.tv_sec-start.tv_sec-1;
        temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
    } else {
        temp.tv_sec = end.tv_sec-start.tv_sec;
        temp.tv_nsec = end.tv_nsec-start.tv_nsec;
    }
    return temp;
}

struct flow_info {
	int flow_id;
	int flow_size;
	char* server_addr;
	int server_port;
};
void* start_client(void* info) {
	struct flow_info* f = (struct flow_info*)info;
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *msg = (char*)malloc(f->flow_size * sizeof(char));
	if(msg == NULL) {
		printf("heap is full\n");
	}
	memset(msg, '0', f->flow_size * sizeof(char));
	msg[f->flow_size - 1] = 'd';
	char* buffer[1] = {0}; 
	unsigned int sent_bytes = 0;
	while ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(f->server_port); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, f->server_addr, &serv_addr.sin_addr)<=0) 
	{ 
		printf("server addr:%s\n", f->server_addr);
		printf("\nInvalid address/ Address not supported \n"); 
	} 
	while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		fprintf(stderr, "\nConnection Failed \n"); 
		    free(f->server_addr);
			free(f);
			free(msg);
		    pthread_exit(NULL);
	    exit(EXIT_FAILURE);

	}
    struct timespec time1, time2;
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);

	while(sent_bytes != f->flow_size) {
		int sent = send(sock, msg + sent_bytes, f->flow_size - sent_bytes, 0); 
		if(sent == -1) {
			fprintf(stderr, "socket error");
            fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		    free(f->server_addr);
			free(f);
			free(msg);
		    pthread_exit(NULL);
   	        exit(EXIT_FAILURE);

		} else
			sent_bytes += sent;
	}
	// printf("flow:%d sent finish\n", f->flow_id); 
	valread = read( sock , buffer, 1);
	if(valread < 0){
	      fprintf(stderr, "val read < 0\n");	
          fprintf(stderr, "socket() failed: %s\n", strerror(errno));
		    free(f->server_addr);
			free(f);
			free(msg);
		    pthread_exit(NULL);
	      exit(EXIT_FAILURE);
	}
	if(buffer[0] == 'd') {
		// flow finishes track info
	    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
		printf("flow %u flow_size:%d time: %u \n", f->flow_id, f->flow_size, diff(time1,time2).tv_nsec);
	}else {
        printf("flow %u ends\n", f->flow_id);
	}
    if(close(sock) < 0) {
    	printf("socket close failure\n");
      	free(f->server_addr);
		free(f);
		free(msg);
	    pthread_exit(NULL);
    	exit(EXIT_FAILURE);
    }	
    free(f->server_addr);
	free(f);
	free(msg);
    pthread_exit(NULL);

}

int main(int argc, char const *argv[]) 
{ 
	const char* cdf_file = argv[1];
	int index = atoi(argv[2]);
	int server_port = atoi(argv[3]);
	char** server_addrs = (char *[]){"192.168.9.27", "192.168.9.28", "192.168.9.29", "192.168.9.30", "192.168.9.31", "192.168.9.32"};
	double bandwidth = 10000000000;
	double load = 0.6;
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
    for (int i = 0; i < NUM_FLOW; i++) {

    	usleep(time * 1000000);
    	struct flow_info *f = malloc(sizeof(struct flow_info));
		// printf("flow %u size:%d\n", i, flow_size);
    	f->flow_id = i;
    	f->flow_size = flow_size;
    	f->server_port = server_port;
    	int addr_index = 0;
    	while((addr_index = rand() % 6) == index) {

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

