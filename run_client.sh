index=$1
port=$2
trace=$3


./client CDF_$trace.txt $index $port > result_$trace_$index
