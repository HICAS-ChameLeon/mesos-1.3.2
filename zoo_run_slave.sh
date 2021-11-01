#!/bin/bash
cd /home/lemaker/open-source/mesos-1.3.2/build && sudo ./bin/mesos-agent.sh --master=zk://172.20.110.35:2181/mesos --work_dir=/home/lemaker/open-source/mesos-1.3.2/build/slave_work_dir
