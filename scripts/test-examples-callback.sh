#!/usr/bin/env bash

SCRIPT_DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
callback_port=8888
callback_file_path="${SCRIPT_DIR}/../.test/test-examples-callback.txt"

listen_for_callbacks() {
  rm -f "$callback_file_path"
  touch "$callback_file_path"
  while true; do
    local response
    response=$(nc -l "$callback_port")
    printf "%s\n" "$response" >> "$callback_file_path"
  done
}

wait_for_example() {
  local example="$1"
  local count
  local timeout
  count=0
  timeout=15
  while true; do
    if [ "$(tail -n 1 "$callback_file_path")" = "$example" ]; then
      break
    fi
    ((count=count+1))
    if (( count > timeout )); then
      break
    fi
    sleep 1
  done
}

wait_for_warmup() {
  while true; do
    if [ "$(tail -n 1 "$callback_file_path")" = "WARMUP" ]; then
      break
    fi
    echo "WARMUP" | nc localhost "$callback_port" || true
    sleep 1
  done
}
