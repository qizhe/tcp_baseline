#!/bin/bash

./run_config.sh 8
./run_exp.sh websearch 8
./get_result.sh 8
python3 parse_result.py 8 websearch
mv ../result/websearch_8_slowdown_size.dat ../result/result_websearch_8_slowdown_size_tcp.dat
