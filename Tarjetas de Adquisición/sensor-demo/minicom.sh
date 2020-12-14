#!/bin/sh

sudo minicom –b 115200 –D /dev/ttyACM$1
