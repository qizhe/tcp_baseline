// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <stdlib.h>     /* malloc, free, rand */
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <unistd.h> 
#include <string.h> 
#include <pthread.h>
#include <unistd.h>

#include "random_variable.h"
#define PORT 8080 
#define NUM_FLOW 5000

struct flow_info {
	int flow_id;
	int flow_size;
	char* server_addr;
};
void* start_client(void* info) {
	struct flow_info* f = (struct flow_info*)info;
	int sock = 0, valread; 
	struct sockaddr_in serv_addr; 
	char *msg = (char*)malloc(f->flow_size * sizeof(char));
	memset(msg, '0', f->flow_size * sizeof(char));
	msg[f->flow_size - 1] = 'd';
	char buffer[1024] = {0}; 
	unsigned int sent_bytes = 0;
	while ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
	} 

	serv_addr.sin_family = AF_INET; 
	serv_addr.sin_port = htons(PORT); 
	
	// Convert IPv4 and IPv6 addresses from text to binary form 
	if(inet_pton(AF_INET, f->server_addr, &serv_addr.sin_addr)<=0) 
	{ 
		printf("\nInvalid address/ Address not supported \n"); 
	} 

	while (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		//printf("\nConnection Failed \n"); 
	}
	while(sent_bytes != f->flow_size) {
		unsigned int sent = send(sock, msg + sent_bytes, f->flow_size - sent_bytes, 0); 
		sent_bytes += sent;
	} 
	valread = read( sock , buffer, 1);
	if(buffer[0] == 'd') {
		// flow finishes track info
		printf("flow %u finish\n", f->flow_id);
	} 
	free(msg);
	free(f);
	close(sock);
    pthread_exit(NULL);

}

int main(int argc, char const *argv[]) 
{ 
	const char* cdf_file = argv[1];
	double bandwidth = 10000000000;
	double load = 0.6;
	struct exp_random_variable exp_r;
	struct empirical_random_variable emp_r;
	init_empirical_random_variable(&emp_r, cdf_file ,true);
	double lambda = bandwidth * load / (emp_r.mean_flow_size * 8.0 / 1460 * 1500);
    init_exp_random_variable(&exp_r, 1.0 / lambda);
    uint32_t flow_size = (uint32_t)(value_emp(&emp_r) + 0.5) * 1460;
    double time = value_exp(&exp_r);
    pthread_t threads[NUM_FLOW];
    for (int i = 0; i < NUM_FLOW; i++) {
    	printf("time: %d\n", (int)(time * 1000000));
    	printf("flow size: %d\n", (int)(flow_size));

    	usleep(time * 1000000);
    	struct flow_info *f = malloc(sizeof(struct flow_info));
    	f->flow_id = i;
    	f->flow_size = flow_size;
    	f->server_addr = "127.0.0.1";
    	pthread_create(&threads[i], NULL, start_client, (void *)f);
    	time = value_exp(&exp_r);
		flow_size = (uint32_t)(value_emp(&emp_r) + 0.5) * 1460;
    }
	pthread_exit(NULL);

} 

