#!/bin/sh
xxd -g 4 -c 4 -e $1 | awk '{ print $2 }'
