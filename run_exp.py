#!/usr/bin/env python3
import argparse
import os
import signal
import subprocess
import tempfile
import threading
import time
import xmlrpc.server
# from process_output import *

def run_client(cpu, port, flow_size, max_rate, filename):
    f = open(filename, 'w')
    args = ["taskset", "-c", str(cpu), "./client", str(port), str(flow_size),str(max_rate)]
    return subprocess.Popen(args, stdout=f, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL, universal_newlines=True)

def run_server(cpu, port, ip_addr, filename):
    f = open(filename, 'w')
    print ("run server")
    args = ["taskset", "-c", str(cpu), "./server", ip_addr, str(port)]
    return subprocess.Popen(args, stdout=f, stderr=subprocess.STDOUT, stdin=subprocess.DEVNULL, universal_newlines=True)

def parse_args():
    parser = argparse.ArgumentParser(description="Run TCP measurement experiments on the receiver.")

    # Add arguments
    parser.add_argument("--start_port", type=int, default=5000, help="start_port")
    parser.add_argument("--num_conns", type=int, default=1, help="Number of RPC style connections.")
    parser.add_argument('--client', action='store_true', default=False, help='Client or Server')
    parser.add_argument("--flow_size", type=int, default=128000, help="Specify the flow size")
    parser.add_argument("--max_rate", type=int, default=0, help="Write raw output to the directory.")
    parser.add_argument("--ip", type=str, default="192.168.10.117", help="IP address")


    # Parse and verify arguments
    args = parser.parse_args()
    return args


if __name__ == "__main__":
    threads = []
    # Parse args
    args = parse_args()
    for i in range(args.num_conns):
        if args.client:
            thread = run_client(i, args.start_port + i, args.flow_size, args.max_rate, "result_{}".format(i))
        else:
            thread = run_server(i, args.start_port, args.ip, "result_{}".format(i))
        threads.append(thread)
    for p in threads:
        p.wait()

