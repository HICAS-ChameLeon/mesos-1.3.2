#!/bin/bash
cd /home/lemaker/open-source/mesos-1.3.2/build && sudo ./bin/mesos-master.sh --work_dir=/home/lemaker/open-source/mesos-1.3.2/build/master_work_dir --zk=zk://172.20.110.35:2181/mesos --quorum=1
