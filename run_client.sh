index=$1
port=$2
trace=$3
num_server=$4

./client CDF_$trace.txt $index $port $num_server > result_"$trace"_"$index"
