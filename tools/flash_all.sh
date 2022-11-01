#!/bin/bash

function whereami() {
    SOURCE=$1
    [[ "$SOURCE" == "" ]] && SOURCE="${BASH_SOURCE[ $((${#BASH_SOURCE[*]}-1)) ]}"  # Last one is the first that was run.

    while [ -h "$SOURCE" ]; do  # resolve $SOURCE until the file is no longer a symlink
        DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
        SOURCE="$(readlink "$SOURCE")"
        [[ $SOURCE != /* ]] && SOURCE="$DIR/$SOURCE"  # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located
    done
    DIR="$( cd -P "$( dirname "$SOURCE" )" && pwd )"
    echo $DIR
}

function currentpos() {
    whereami "${BASH_SOURCE[ 0 ]}"
}


NODE_COUNT=2

ESP32_PATH="$(currentpos)/esp32"
ESPTOOL_PY="${ESP32_PATH}/tools/esptool.py"
# ESPTOOL_PY="echo"

BOOT_LOADER="${ESP32_PATH}/tools/sdk/esp32/bin/bootloader_dio_80m.bin"
BOOT_APP0="${ESP32_PATH}/tools/partitions/boot_app0.bin"

MAIN_INO="$(currentpos)/../src/build/Main.ino.bin"
MAIN_PARTITIONS_INO="$(currentpos)/../src/build/Main.ino.partitions.bin"


function flash {
    port=$1
    [[ "${port}" == "" ]] && port='/dev/ttyUSB0'
    echo ">>> Flashing.. on ${port}"

    ${ESPTOOL_PY} --chip esp32 --port ${port} --baud 921600  \
        --before default_reset --after hard_reset  \
        write_flash -z --flash_mode dio --flash_freq 80m --flash_size detect  \
        0x1000  "${BOOT_LOADER}"  \
        0x8000  "${MAIN_PARTITIONS_INO}"  \
        0xe000  "${BOOT_APP0}"  \
        0x10000 "${MAIN_INO}"
}


ports=`seq 0 $((NODE_COUNT - 1))`
for p in ${ports}; do
    dev=/dev/ttyUSB${p}
    echo "--------- Flash ${dev} ---------"
    if [ -c ${dev} ]; then
        # echo $(ps ax | grep "screen ${dev}" -i)
        # echo $(pgrep "screen ${dev}" -f -i -c)
        # exit

        # if [ $(pgrep "screen ${dev}" -f -i -c) -ne 0 ]; then
        #     echo ">>> Occupy ${dev}"
        #     pid=$(pgrep "screen ${dev}" -f)
        #     kill $pid
        #     pid=$(pgrep "SCREEN ${dev}" -f)
        #     kill $pid
        # fi

        # for pid in $(pgrep "screen ${dev}" -f -i); do
        #     echo ">>> Occupy ${dev}"
        #     kill $pid
        # done

        "$(currentpos)"/occupy_port.sh ${dev}

        flash "${dev}"
    else
        echo "${dev} does not exist!"
    fi
    echo
done

