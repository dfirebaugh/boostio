#!/bin/bash

cmake -B "./build/`uname`" -S . -DCMAKE_EXPORT_COMPILE_COMMANDS=1
