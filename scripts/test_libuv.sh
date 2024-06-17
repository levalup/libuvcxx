#!/usr/bin/env bash

# working path
HOME=$(cd "$(dirname "$0")" || exit; pwd)
PROJECT="$HOME/.."
LIBUV="$PROJECT/libuv"

# working config
LAST="v1.0.0"

cd "$LIBUV" || exit
# shellcheck disable=SC2207
tags=($(git tag | grep -E '^v[0-9]+\.[0-9]+\.[0-9]+$'))
# shellcheck disable=SC2207
tags=($(echo "${tags[@]}" | tr ' ' '\n' | sort -rV))

if [ $# -gt 0 ]; then
  # find start version, and skip older version
  start="$1"
  found_start=0
  # find start index then slice
  for ((i=0; i<${#tags[@]}; i++)); do
      if [ "${tags[$i]}" == "$start" ]; then
        # find version
        found_start=1
        echo "[INFO] Skip version greater than $start."
        tags=("${tags[@]:$i}")
        break
      fi
  done
  if [ $found_start -eq 0 ]; then
    echo "[WARNING] Can not find $start. Start from the latest version."
  fi
fi

echo "[INFO] Test start."

succeed=0

logs="$HOME/logs"
for tag in "${tags[@]}"; do
  mkdir -p "$logs"
  f="$logs/$tag-log.log"

  echo -n "[INFO] Testing $tag ..."

  bash "$HOME/test_libuv_version.sh" "$tag" > "$f" 2>&1
  status=$?
  if [ $status -eq 0 ]; then
    echo " [OK]"
    ((succeed++))
  else
    echo " [Failed]"
    echo "[ERROR] Check $f"
    exit $status
  fi

  if [ "$tag" = "$LAST" ]; then
    break
  fi
done

echo "[INFO] Test succeeded in $succeed versions after $LAST."
