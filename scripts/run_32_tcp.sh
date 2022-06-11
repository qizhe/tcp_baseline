.#!/bin/bash

./run_config.sh 32
./run_exp.sh websearch 32
./get_result.sh 32
python3 parse_result.py 32 websearch
mv ../result/websearch_32_slowdown_size.dat ../result/result_websearch_32_slowdown_size_tcp.dat
