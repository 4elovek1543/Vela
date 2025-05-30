#!/bin/bash

cmake ..
make

echo "===== STARTING Vela ====="

./Vela --debug