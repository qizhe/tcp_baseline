// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <pthread.h>
#define PORT 8080 
// inet_pton ()
#define TOTAL_FLOW 30000
void* receive_flow(void* arg) {
	int new_socket = *((int *) arg);
	char buffer[1024] = {0}; 

	while(1) {
		int valread = read(new_socket, buffer, 1024);
		if(buffer[valread - 1] == 'd') {
			char* end_str = "d";
			valread = send(new_socket , end_str , 1 , 0);
			while(valread != 1) {
				valread = send(new_socket , end_str , 1 , 0);
			}
			break;
		}
	}
    close(new_socket);
    pthread_exit(NULL);

}
int main(int argc, char const *argv[]) 
{ 
	int server_fd, new_socket, valread; 
	struct sockaddr_in address; 
	int opt = 1; 
	int addrlen = sizeof(address); 
	char *hello = "Hello from server"; 
	
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
	address.sin_family = AF_INET; 
	address.sin_addr.s_addr = INADDR_ANY; 
	address.sin_port = htons( PORT ); 
	
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
    int i = 0;
	while (1) 
	{ 
		new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen);
    	pthread_create(&threads[i], NULL, receive_flow, &new_socket);
    	i++;

	} 
	pthread_exit(NULL);
} 

