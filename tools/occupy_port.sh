#!/bin/bash

dev=$1
mode=$2

[[ "${mode}" == "Verifying" ]] && echo 'No occupying.. Just verify.' && exit 0

for pid in $(pgrep "screen ${dev}" -f -i); do
    echo ">>> Occupy ${dev}!"
    kill $pid
done
