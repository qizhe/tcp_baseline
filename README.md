# tcp_baseline
This repository contains the code we used for getting results of TCP and DCTCP in cloudlab for [dcPIM](https://github.com/Terabit-Ethernet/dcPIM).

## SIGCOMM 2022 Artifact Evaluation

### Configure Cloudlab Machines
We conduct our experiment using the [m510](http://docs.cloudlab.us/hardware.html#%28part._cloudlab-utah%29) machines available at CloudLab.
The attachment in the Hotcrp contains the cloudlab account info and private/public key pairs and the passcode.
1. Login the [Cloudlab](https://www.cloudlab.us).  
2. Find the [dcpim_chassis](https://www.cloudlab.us/p/ba9b05f3790cb9f88e84a10f480fb3193dd4d56c) profile in the Project Profiles and instantiate the profile (using chassis 13) to start the experiments.


### Run experiments

Clone the repo into your local machine,

```
git clone https://github.com/Terabit-Ethernet/dcPIM.git
cd dcPIM/implementation
```

Download public and private key pairs in the Hotcrp. 
Copy the keys into .ssh repo and add keys.
```
cp id_ed25519 ~/.ssh/id_ed25519
cp id_ed25519.pub ~/.ssh/id_ed25519.pub
ssh-add -K ~/.ssh/id_ed25519
```

We provide oneshot script for running the experiment (32 server testbeds) which sending commands from your local machine to all remote servers:
```
cd script/
./run_32_tcp.sh
./run_32_dctcp.sh
```

The parsed result is in `result/websearch_32_slowdown_size_tcp.dat` and `result/websearch_32_slowdown_size_dctcp.dat`. The format of files:
```
SIZE_OF_FLOWS MEAN_SLOWDOWN DIFF_BETWEEN_TAIL_AND_MEAN 
```
