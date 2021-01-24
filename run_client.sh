# index=$1
num_server=$1
# num_server=$3
flow_size=$2
python3 run_exp.py --client --flow_size $flow_size --num_conns $num_server

