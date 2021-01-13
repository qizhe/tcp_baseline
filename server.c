// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#include <limits.h>
 #include <stdint.h>
#include <inttypes.h>
#include <vector>
// inet_pton ()
#define TOTAL_FLOW 300000
#define THREADSTACK (PTHREAD_STACK_MIN + 16384)

void* receive_flow(void* arg) {
	int new_socket = static_cast<int>(reinterpret_cast<intptr_t>(arg));
	char buffer[1460] = {0}; 
	// printf("new connection\n");
	while(1) {
		int valread = read(new_socket, buffer, 1460);
		if(buffer[valread - 1] == 'd') {
			// printf("get d: val read:%d\n", valread);
			break;
		}
	}
	close(new_socket);

    pthread_exit(NULL);

}
int main(int argc, char const *argv[]) 
{ 
	int server_fd, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	// char *hello = "Hello from server"; 
	const char *server_addr = argv[1];
	int server_port = atoi(argv[2]);

	// Creating socket file descriptor 
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	
	// Forcefully attaching socket to the port 8080 
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
												&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	printf("addr: %s\n", server_addr);
	int value;
	if((value = inet_pton(AF_INET, server_addr, &address.sin_addr)) <= 0) 
	{
		printf("value:%d\n", value); 
		printf("\nInvalid address/ Address not supported \n"); 
	} 
	address.sin_family = AF_INET; 
	// address.sin_addr.s_addr = INADDR_ANY;

	address.sin_port = htons( server_port ); 
	
	// Forcefully attaching socket to the port 8080 
	if (bind(server_fd, (struct sockaddr *)&address, 
								sizeof(address))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	} 
	if (listen(server_fd, 3) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
    pthread_t threads[TOTAL_FLOW];
    pthread_attr_t  attrs;
    pthread_attr_init(&attrs);
    pthread_attr_setstacksize(&attrs, THREADSTACK);
    int i = 0;
	while (1) 
	{ 
		int new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen);
    	pthread_create(&threads[i], &attrs, receive_flow, (void*)new_socket);
    	i++;

	} 
    pthread_attr_destroy(&attrs);

	pthread_exit(NULL);
} 

