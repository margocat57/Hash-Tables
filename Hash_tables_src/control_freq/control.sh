#!/usr/bin/env bash

FREQ_FILE="/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq"
TEMP_FILE="/sys/class/hwmon/hwmon1/temp22_input"

while true; do
    PROCESS=""

    if pgrep -f "run_opt_test0" > /dev/null; then
        PROCESS="run_opt_test0"
    elif pgrep -f "run_opt_test1" > /dev/null; then
        PROCESS="run_opt_test1"
    elif pgrep -f "run_opt_test2" > /dev/null; then
        PROCESS="run_opt_test2"
    elif pgrep -f "run_opt_test3" > /dev/null; then
        PROCESS="run_opt_test3"
    elif pgrep -f "run_pgo_test2" > /dev/null; then
        PROCESS="run_pgo_test2"
    fi

    if [[ -n "$PROCESS" ]]; then
        FREQ=$(cat "$FREQ_FILE")
        TEMP=$(cat "$TEMP_FILE")
        DATE=$(date +%H-%M-%S)

        echo "$PROCESS;$FREQ;$TEMP;$DATE" >> "control_freq/control.csv"
    fi

    sleep 0.1
done 


# можно также
# for i in 0 1 2 3; do
#   if pgrep -f "run_opt_test$i" > /dev/null; then ...

