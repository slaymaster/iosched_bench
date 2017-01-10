#!/bin/bash

declare -a scheds=("noop" "cfq" "deadline")

for i in "${scheds[@]}"
    do
        echo $i | sudo tee /sys/block/sda/queue/scheduler
        ./iosched 10 > test$i
    done
