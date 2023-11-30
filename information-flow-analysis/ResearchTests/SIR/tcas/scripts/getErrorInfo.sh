#!/bin/bash


(printf "{\n"
for f in {1..41}; do
  diff ../source.alt/source.orig/tcas.c ../versions.alt/versions.orig/v$f/tcas.c 1>&2
  read error
  echo "$f: ($error,),"
done

printf "}\n"
) > ../errorInfo
