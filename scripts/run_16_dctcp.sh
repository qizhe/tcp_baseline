#!/bin/bash

./run_config_dctcp.sh 16
./run_exp.sh websearch 16
./get_result.sh 16
python3 parse_result.py 16 websearch
mv ../result/websearch_16_slowdown_size.dat ../result/result_websearch_16_slowdown_size_dctcp.dat
