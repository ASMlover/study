#!/usr/bin/env bash
EXE=./build/mslang
for f in benchmarks/*.ms; do
  echo -n "$(basename $f): "
  $EXE --benchmark 5 "$f"
done
