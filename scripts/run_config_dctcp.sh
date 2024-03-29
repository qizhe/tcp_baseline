#!/bin/bash
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
ms1332)

num_hosts=$1

# set up the server
for addr in  "${ssh_array[@] : 0 : $num_hosts}";
	do 
		ssh -o StrictHostKeyChecking=no -p 22 artifact@$addr.utah.cloudlab.us "git clone https://qizhe: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@github.com/qizhe/tcp_baseline.git; cd ~/tcp_baseline;git pull; make" &
		ssh -o StrictHostKeyChecking=no -p 22 artifact@$addr.utah.cloudlab.us "sudo sysctl -w net.ipv4.tcp_congestion_control=dctcp;sudo sysctl -w net.ipv4.tcp_ecn_fallback=0;" &
	done
