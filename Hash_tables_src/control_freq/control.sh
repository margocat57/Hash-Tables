#!/usr/bin/env bash

FREQ_FILE="/sys/devices/system/cpu/cpu3/cpufreq/scaling_cur_freq"
TEMP_FILE="/sys/class/hwmon/hwmon1/temp22_input"

while true; do
    PROCESS=""

    if pgrep -f "run_opt_test0" > /dev/null; then
        PROCESS=$(pgrep -af "run_opt_test0" | grep -oE 'run_opt_test0(_O[0-3]+)?' | head -n 1)
    elif pgrep -f "run_opt_test1" > /dev/null; then
        PROCESS=$(pgrep -af "run_opt_test1" | grep -oE 'run_opt_test1(_O[0-3]+)?' | head -n 1)
    elif pgrep -f "run_opt_test2" > /dev/null; then
        PROCESS=$(pgrep -af "run_opt_test2" | grep -oE 'run_opt_test2(_O[0-3]+)?' | head -n 1)
    elif pgrep -f "run_opt_test3" > /dev/null; then
        PROCESS=$(pgrep -af "run_opt_test3" | grep -oE 'run_opt_test3(_O[0-3]+)?' | head -n 1)
    elif pgrep -f "run_pgo_test2" > /dev/null; then
        PROCESS=$(pgrep -af "run_pgo_test2" | grep -oE 'run_pgo_test2(_O[0-3]+)?' | head -n 1)
    fi

    if [[ -n "$PROCESS" ]]; then
        FREQ=$(cat "$FREQ_FILE")
        TEMP=$(cat "$TEMP_FILE")
        DATE=$(date +%H-%M-%S)

        echo "$PROCESS;$FREQ;$TEMP;$DATE" >> "control_freq/control.csv"
    fi

    sleep 0.1
done 



