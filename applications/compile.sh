#!/bin/bash

tput reset

if [ -z "${1}" ]; then
  export TARGET=wismote
else
  export TARGET=${1}
fi

rm *.$TARGET

set -e

make clean -f Makefile_enabled_node
make -f Makefile_enabled_node sink-node


make clean -f Makefile_enabled_node
# CFLAGS=-DDEMO 
make -f Makefile_enabled_node enabled-node

make clean -f Makefile_controller_node
make -f Makefile_controller_node

size *.$TARGET
