#!/usr/bin/env bash

# Usage: $0 [start]
# If give start version, test will start at and after `start`

start_time=$(date +%s)

# working path
HOME=$(cd "$(dirname "$0")" || exit; pwd)

stds=(11 14 17)

echo "[INFO] Test start."

succeed=0

logs="$HOME/logs"
for std in "${stds[@]}"; do
  mkdir -p "$logs"
  f="$logs/std-cxx$std-log.log"

  echo -n "[INFO] Testing std=c++$std ..."

  bash "$HOME/test_cxx_standard.sh" "$std" > "$f" 2>&1
  status=$?
  if [ $status -eq 0 ]; then
    echo " [OK]"
    ((succeed++))
  else
    echo " [Failed]"
    # cat "$f"  #< cat for quick fix
    echo "[ERROR] Test failed on std=c++$std. Aborted."
    echo "[ERROR] Check $f"
    exit $status
  fi
done

# get test take time
end_time=$(date +%s)
run_time=$((end_time - start_time))
minutes=$((run_time / 60))
seconds=$((run_time % 60))
spent=$(if [ $minutes -gt 0 ]; then echo "${minutes}m${seconds}s"; else echo "${seconds}s"; fi)

# echo conclusion
echo "[INFO] Take $spent."
echo "[INFO] Test succeeded in $succeed standards."
