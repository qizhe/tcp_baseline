ssh_array=(
ms1301
ms1302
ms1303
ms1304
ms1305
ms1306
ms1307
ms1308
ms1309
ms1310
ms1311
ms1312
ms1313
ms1314
ms1315
ms1316
ms1317
ms1318
ms1319
ms1320
ms1321
ms1322
ms1323
ms1324
ms1325
ms1326
ms1327
ms1328
ms1329
ms1330
ms1331
ms1332
)

workload=$1
num_hosts=$2

i=1

sleep 5

i=1

#run experiment
for addr in  "${ssh_array[@] : 0 : $num_hosts}";
	do 
	 	ssh -o StrictHostKeyChecking=no -p 22 artifact@$addr.utah.cloudlab.us "cd ~/tcp_baseline; ./run_server.sh 10.10.1.$i 5000" &
	 	((i = i + 1))
	done

sleep 5
i=0
for addr in  "${ssh_array[@] : 0 : $num_hosts}";
	do 
	 	ssh -o StrictHostKeyChecking=no -p 22 artifact@$addr.utah.cloudlab.us "cd ~/tcp_baseline; ./run_client.sh $i 5000 $workload 32" > debug &
	 	((i = i + 1))
	done

sleep 120

for addr in  "${ssh_array[@] : 0 : $num_hosts}";
	do 
	 	ssh -o StrictHostKeyChecking=no -p 22 artifact@$addr.utah.cloudlab.us "killall server; killall client" > debug &
	 	((i = i + 1))
	done
