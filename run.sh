#!/bin/zsh
# Usage: ./run.sh build|run|clean <any file or folder inside a project>
# Walks up to the nearest folder containing a makefile and runs make there.
set -e
mode="${1:-run}"; target="${2:-.}"
dir="$target"; [ -d "$dir" ] || dir="$(dirname "$dir")"
dir="$(cd "$dir" && pwd)"
while [ "$dir" != "/" ] && [ ! -f "$dir/makefile" ] && [ ! -f "$dir/Makefile" ]; do dir="$(dirname "$dir")"; done
if [ ! -f "$dir/makefile" ] && [ ! -f "$dir/Makefile" ]; then
  echo "No makefile found above $target. Copy resources/sampleProject/makefile next to your main.cpp (or use ./new-project.sh)." >&2; exit 1
fi
cd "$dir"
case "$mode" in build) make ;; clean) make clean ;; *) make run ;; esac
