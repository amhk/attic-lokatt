#!/bin/bash

# When we setup a proper virtaulenv, this script will not be needed
export PYTHONPATH=../common:$PYTHONPATH
python3 lokatt.py
