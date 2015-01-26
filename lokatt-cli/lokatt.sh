#!/bin/bash

# When we setup a proper virtaulenv, this script will not be needed
script_path=$(dirname $(readlink -f $0))
common_path=$script_path/../common

export PYTHONPATH=$common_path:$PYTHONPATH
python3 $script_path/lokatt.py
