// Client side C/C++ program to demonstrate Socket programming 
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

#define _GNU_SOURCE

#include "random_variable.h"
#define NUM_FLOW 5000
#define THREADSTACK (PTHREAD_STACK_MIN + 32768)


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

struct flow_info {
    int flow_id;
    int flow_size;
    char* server_addr;
    int server_port;
    struct timespec start_time;
    unsigned int max_pacing_rate; // bytes per second
};

bool send_flow(struct flow_info* f) {
    char *msg = (char*)malloc(5000000);
    int sock = 0, valread; 
    struct sockaddr_in serv_addr; 
    
    if(msg == NULL) {
        printf("heap is full\n");
    }
    // memset(msg, '0', f->flow_size * sizeof(char));
    // msg[f->flow_size - 1] = 'd';
    char buffer[1] = {0};
    buffer[0] = 'd';
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
        std::cout << f->server_addr << std::endl;
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
    // if( f->max_pacing_rate && setsockopt(sock, SOL_SOCKET, SO_MAX_PACING_RATE,
	// 	       &f->max_pacing_rate, sizeof(f->max_pacing_rate)) == -1) {
    //     free(msg);
    //     close(sock);
    //     return false;
    // }
    // std::cout << f->flow_size << std::endl;
    while(sent_bytes != f->flow_size) {
        int should_sent = f->flow_size - sent_bytes > 5000000 ?  5000000 : f->flow_size - sent_bytes;
        // if(should_sent  == f->flow_size - sent_bytes) {
        //     msg[should_sent - 1] == 'd';
        //     std::cout << should_sent - 1 << std::endl;
        // }
        int sent = send(sock, msg, should_sent, 0); 
        if(sent == -1) {
            fprintf(stderr, "socket error");
            fprintf(stderr, "socket() failed: %s\n", strerror(errno));
            free(msg);
            close(sock);
            return false;

        } else
            sent_bytes += sent;
        // std::atomic_fetch_add(&agg_stats.interval_bytes, (unsigned long)sent);
		// std::atomic_fetch_add(&agg_stats.total_bytes, (unsigned long)sent);
    }
    send(sock, buffer,1, 0); 
    // std::cout <<  "flow sent finish\n" << std::endl;
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
//    	clock_gettime(CLOCK_REALTIME, &f->start_time);
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
    // const char* cdf_file = argv[1];
    // int index = atoi(argv[2]);
    int server_port = atoi(argv[1]);
    unsigned int flow_size = atoi(argv[2]);
    unsigned int max_pace_rate = atoi(argv[3]); 
    // int num_hosts = atoi(argv[4]);
    int num_hosts = 1;
    // char** server_addrs = (char *[]){"5.0.0.10", "6.0.0.10", "7.0.0.10", "8.0.0.10", "9.0.0.10", "10.0.0.10", "11.0.0.10", "12.0.0.10"};
    // char** server_addrs = (char *[]){"5.0.0.10", "6.0.0.10"};
    std::vector<std::string> server_addrs;
    std::vector<std::thread> workers;   
    // set_cpu_affinity();
    // double bandwidth = 10000000000;
    // double load = 0.5;
    // struct exp_random_variable exp_r;
    // struct empirical_random_variable emp_r;
    // init_empirical_random_variable(&emp_r, cdf_file ,true);
    // double lambda = bandwidth * load / (emp_r.mean_flow_size * 8.0 / 1460 * 1500);
    // init_exp_random_variable(&exp_r, 1.0 / lambda);
    // // unsigned int flow_size = (uint32_t)(value_emp(&emp_r) + 0.5) * 1460;
    // double time = value_exp(&exp_r);
    // pthread_t threads[NUM_FLOW];
    // pthread_attr_t  attrs;
    // pthread_attr_init(&attrs);
    // pthread_attr_setstacksize(&attrs, THREADSTACK);
    // struct timespec offset1, offset2; 

    for (int i = 1; i <= num_hosts; i++) {
        server_addrs.push_back("192.168.10.116");
        // server_addrs.push_back(std::to_string(i) + ".0.0.10");

    }
   	// workers.push_back(std::thread(aggre_thread, &agg_stats));

    for (int i = 0; i < NUM_FLOW; i++) {

// //        clock_gettime(CLOCK_REALTIME, &offset1); 
//         struct timespec wait;
//         wait.tv_nsec = time * 1e9;
//             nanosleep(&wait, NULL);
// 	//usleep(time * 1000000);
// //        clock_gettime(CLOCK_REALTIME, &offset2); 
// //	printf("time:%f\n", time);
// //        printf("old: %f\n", diff(offset1, offset2));
// //	printf("diff:%f\n", diff(offset1, offset2) - time);
//         // printf("time:%f\n", time);
//         // continue;
            struct flow_info *f = (flow_info *)malloc(sizeof(struct flow_info));
            // printf("flow %u size:%d\n", i, flow_size);
            f->flow_id = i;
            f->flow_size = flow_size;
            f->server_port = server_port;
            f->max_pacing_rate = max_pace_rate;
            clock_gettime(CLOCK_REALTIME, &f->start_time);
            // int addr_index = 0;
            // while((addr_index = rand() % num_hosts) == index) {

            // }
            std::string server_addr = server_addrs[0];
            f->server_addr = (char*)malloc(strlen(server_addr.c_str()) + 1);
            strcpy(f->server_addr, server_addr.c_str());
            start_client((void *)f);
//         int thread_id = pthread_create(&threads[i],  &attrs, start_client, (void *)f);
//         if(thread_id < 0) {
//             printf("error number:%d\n", thread_id);
//         }
//         time = value_exp(&exp_r);
//         flow_size = (uint32_t)(value_emp(&emp_r) + 0.5) * 1460;
//         f = NULL;
   }

//    pthread_attr_destroy(&attrs);
	// for(int i = 0; i < workers.size(); i++) {
	// 	workers[i].join();
	// }
    pthread_exit(NULL);

} 

