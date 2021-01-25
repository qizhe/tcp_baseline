// Server side C/C++ program to demonstrate Socket programming 
#include <atomic>
#include <stdio.h> 
#include <stdlib.h>     /* malloc, free, rand */
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <chrono>         // std::chrono::seconds
#include <limits.h>
#include <unistd.h> 
#include <string.h> 
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>   
#include <sys/resource.h>
#include <vector>
#include <string>
#include <iostream>
#include <thread>
#include <x86intrin.h>

// inet_pton ()
#define TOTAL_FLOW 300000
#define THREADSTACK (PTHREAD_STACK_MIN + 16384)

/**
 * rdtsc(): return the current value of the fine-grain CPU cycle counter
 * (accessed via the RDTSC instruction).
 */
inline static uint64_t rdtsc(void)
{
	uint32_t lo, hi;
	__asm__ __volatile__("rdtsc" : "=a" (lo), "=d" (hi));
	return (((uint64_t)hi << 32) | lo);
}

/**
 * get_cycles_per_sec(): calibrate the RDTSC timer.
 * 
 * Return: the number of RDTSC clock ticks per second.
 */
double get_cycles_per_sec()
{
	static double cps = 0;
	if (cps != 0) {
		return cps;
	}
	
	// Take parallel time readings using both rdtsc and gettimeofday.
	// After 10ms have elapsed, take the ratio between these readings.

	struct timeval start_time, stop_time;
	uint64_t start_cycles, stop_cycles, micros;
	double old_cps;

	// There is one tricky aspect, which is that we could get interrupted
	// between calling gettimeofday and reading the cycle counter, in which
	// case we won't have corresponding readings.  To handle this (unlikely)
	// case, compute the overall result repeatedly, and wait until we get
	// two successive calculations that are within 0.1% of each other.
	old_cps = 0;
	while (1) {
		if (gettimeofday(&start_time, NULL) != 0) {
			printf("count_cycles_per_sec couldn't read clock: %s",
					strerror(errno));
			exit(1);
		}
		start_cycles = rdtsc();
		while (1) {
			if (gettimeofday(&stop_time, NULL) != 0) {
				printf("count_cycles_per_sec couldn't read clock: %s",
						strerror(errno));
				exit(1);
			}
			stop_cycles = rdtsc();
			micros = (stop_time.tv_usec - start_time.tv_usec) +
				(stop_time.tv_sec - start_time.tv_sec)*1000000;
			if (micros > 10000) {
				cps = (double)(stop_cycles - start_cycles);
				cps = 1000000.0*cps/(double)(micros);
				break;
			}
		}
		double delta = cps/1000.0;
		if ((old_cps > (cps - delta)) && (old_cps < (cps + delta))) {
			return cps;
		}
		old_cps = cps;
	}
}

/**
 * to_seconds() - Given an elapsed time measured in cycles, return a
 * floating-point number giving the corresponding time in seconds.
 * @cycles:    Difference between the results of two calls to rdtsc.
 * 
 * Return:     The time in seconds corresponding to cycles.
 */
double to_seconds(uint64_t cycles)
{
    return ((double) (cycles))/get_cycles_per_sec();
}

struct Agg_Stats {
	std::atomic<unsigned long> total_bytes;
	std::atomic<unsigned long> interval_bytes;
	uint64_t start_cycle;
	int interval_sec;
};

struct Agg_Stats agg_stats;
void init_agg_stats(struct Agg_Stats* stats, int interval_sec) {
	atomic_store(&stats->total_bytes, (unsigned long)0);
	atomic_store(&stats->interval_bytes, (unsigned long)0);
	stats->start_cycle = rdtsc();
	stats->interval_sec = interval_sec;
}

void aggre_thread(struct Agg_Stats *stats) {
	init_agg_stats(stats, 1);
	while(1) {
		uint64_t start_cycle = rdtsc();
		uint64_t end_cycle;
		double rate;
		double bytes;
    	std::this_thread::sleep_for (std::chrono::seconds(stats->interval_sec));
    	end_cycle = rdtsc();
    	bytes = atomic_load(&stats->interval_bytes);
    	rate = (bytes)/ to_seconds(
			end_cycle - start_cycle);
		printf("Throughput: "
		"%.2f Gbps, bytes: %f, time: %f\n", rate * 1e-09 * 8, (double) bytes, to_seconds(
		end_cycle - start_cycle));
    	atomic_store(&stats->interval_bytes, (unsigned long)0);
	}
}

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
    return temp.tv_sec + temp.tv_nsec * 1e-9;
}
void* receive_flow(void* arg) {
	int new_socket = static_cast<int>(reinterpret_cast<intptr_t>(arg));
	//char buffer[1460]; 
	 char *buffer = (char*)malloc(5000000);
	while(1) {
		int valread = read(new_socket, buffer, 5000000);
		std::atomic_fetch_add(&agg_stats.interval_bytes, (unsigned long)valread);
		std::atomic_fetch_add(&agg_stats.total_bytes, (unsigned long)valread);
		if(buffer[valread - 1] == 'd') {
			// printf("get d: val read:%d\n", valread);
			send(new_socket, buffer, 1, 0);
			// break;
		}
	}
	close(new_socket);
	free(buffer);
    // pthread_exit(NULL);

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
    std::vector<std::thread> workers;   

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
 
   	workers.push_back(std::thread(aggre_thread, &agg_stats));

	while (1) 
	{ 
		int new_socket = accept(server_fd, (struct sockaddr *)&address, 
					(socklen_t*)&addrlen);
    	// pthread_create(&threads[i], &attrs, receive_flow, (void*)new_socket);
		receive_flow((void*)new_socket);
    	i++;

	} 
    // pthread_attr_destroy(&attrs);
 	for(int i = 0; i < workers.size(); i++) {
		workers[i].join();
	}
	pthread_exit(NULL);
} 

