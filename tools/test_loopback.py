#!/usr/bin/python
'''
Tool for test sending from E34 on /dev/ttyUSB?, and receive on another E34 on /dev/ttyUSB?.

I have got this code from Berdy.
'''
__author__ = "Berdy"
# __copyright__ = "Copyright 2007, The Cogent Project"
# __credits__ = ["Rob Knight", "Peter Maxwell", "Gavin Huttley",
#                     "Matthew Wakefield"]
__license__ = "GPL"
__version__ = "1.0.0"
__maintainer__ = "Pasakorn Tiwatthanont"
# __email__ = "..."
# __status__ = "Production"


import sys
import serial
import time
from datetime import datetime
import string
import random


TMO_PERIOD = 3.
DEFAULT_PAYLOAD_LEN = 500


# -----------------------------------------------------------------------------
def print_info(str):
    current_time = datetime.now().strftime("%H:%M:%S")
    print(current_time + '>' + str)


# -----------------------------------------------------------------------------
def random_string(N):
    str = ''.join(random.choices(string.ascii_letters + string.digits, k=N))
    str = str + '\n'
    return str


# -----------------------------------------------------------------------------
if __name__ == '__main__':
    if len(sys.argv) == 1:
        print('Ex.> {} <port> <baud> [payload_len | {}]'.format(sys.argv[0], DEFAULT_PAYLOAD_LEN))
    serial_port = sys.argv[1]
    serial_baud = sys.argv[2]
    payload_len = int(sys.argv[3]) if len(sys.argv) >= 4 else DEFAULT_PAYLOAD_LEN
    print_info(str(sys.argv))

    result_ok = 0
    result_failed = 0
    result_timeout = 0
    start_time = time.time()

    with serial.Serial(serial_port, serial_baud) as ser:
        while ((time.time() - start_time) < 60):
            # send test string to slave device
            send_str = random_string(payload_len)  #"0123456789a0123456789b0123456789c0123456789d0123456789e0123456789f" #random_string(30)
            recv_str = ''
            ser.write(send_str.encode())
            waiting = True
            send_time = time.time()
            # print_info(send_str)

            while (waiting):
                # read until endline (non blocking)
                if (ser.in_waiting > 0):
                    recv_str = recv_str + ser.read(ser.inWaiting()).decode('ascii')
                    if '\n' in recv_str:
                        if (recv_str == send_str):
                            result_ok += 1
                            print_info('ok '+ recv_str)
                        else:
                            result_failed += 1
                            print_info('failed : ' + recv_str)
                        waiting = False

                # check timeout
                elapse_time = time.time() - send_time
                if (elapse_time > TMO_PERIOD):
                    waiting = False
                    result_timeout += 1
                    print_info('timeout '+ recv_str)
                time.sleep(0.1)

            # suspend sending thread
            #time.sleep(0.5)

        ser.close()

    success = (result_ok * 100) / (result_ok + result_failed + result_timeout)
    print("ok:%d failed:%d timeout:%d success:%.2f%%" %(result_ok, result_failed, result_timeout, success))
