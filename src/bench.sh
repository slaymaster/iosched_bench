#!/bin/bash

#add scheduler to scheds to add it to the benchmark
declare -a scheds=("noop" "cfq" "deadline")

for i in "${scheds[@]}"
    do
        echo $i | sudo tee /sys/block/sda/queue/scheduler
        ./iosched 10 > test$i
        rm -rf testefb*
        echo 3 | sudo tee /proc/sys/vm/drop_caches
    done

echo "NOOP AVERAGE:"
cat testnoop | datamash mean 1
echo "CFQ AVERAGE:"
cat testcfq | datamash mean 1
echo "DEADLINE AVERAGE:"
cat testdeadline | datamash mean 1
