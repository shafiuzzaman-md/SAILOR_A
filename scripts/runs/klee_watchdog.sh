#!/bin/bash
# KLEE Watchdog — kills KLEE processes that exceed a time limit
# Run alongside SAILOR: nohup bash klee_watchdog.sh > klee_watchdog.log 2>&1 &

MAX_KLEE_HOURS=${1:-6}  # default 6 hours max per KLEE run
CHECK_INTERVAL=300      # check every 5 minutes

echo "[Watchdog] Started. Max KLEE runtime: ${MAX_KLEE_HOURS}h"
echo "[Watchdog] Check interval: ${CHECK_INTERVAL}s"

while true; do
    # Find KLEE processes running longer than MAX_KLEE_HOURS
    max_seconds=$((MAX_KLEE_HOURS * 3600))

    # Check inside docker containers
    for container in $(docker ps --format '{{.Names}}' | grep sailor); do
        # Get KLEE PIDs and their elapsed time
        docker exec "$container" ps -eo pid,etimes,comm 2>/dev/null | grep klee | while read pid elapsed comm; do
            if [ "$elapsed" -gt "$max_seconds" ] 2>/dev/null; then
                echo "[Watchdog] $(date): Killing stuck KLEE (PID=$pid, elapsed=${elapsed}s) in $container"
                docker exec "$container" kill -9 "$pid" 2>/dev/null
            fi
        done
    done

    # Also check host-level KLEE processes
    ps -eo pid,etimes,comm | grep klee | while read pid elapsed comm; do
        if [ "$elapsed" -gt "$max_seconds" ] 2>/dev/null; then
            echo "[Watchdog] $(date): Killing stuck host KLEE (PID=$pid, elapsed=${elapsed}s)"
            kill -9 "$pid" 2>/dev/null
        fi
    done

    sleep $CHECK_INTERVAL
done
