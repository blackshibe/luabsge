#!/bin/bash
log="${1//\\//}"
[ -z "$log" ] && exit 0
grep -q "ld returned" "$log" 2>/dev/null || exit 0
grep -q "undefined reference" "$log" 2>/dev/null && exit 0

args=$(grep -m1 "g++.exe" "$log" | sed -E 's/^.*g\+\+\.exe //; s/ && cd \."?$//')
[ -z "$args" ] && exit 0

script="${BASH_SOURCE[0]//\\//}"
cd "$(dirname "$script")/build" || exit 0

gpp=/c/msys64/ucrt64/bin/g++.exe
[ -x "$gpp" ] || gpp=$(command -v g++)

collect2=$("$gpp" $args -v 2>&1 | grep -m1 "collect2.exe")
[ -z "$collect2" ] && exit 0

echo ""
echo "[surface_link_error] output:"
eval "$collect2" 2>&1