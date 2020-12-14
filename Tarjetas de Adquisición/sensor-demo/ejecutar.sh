#!/bin/sh

make TARGET=stm32nucleo-spirit1 BOARD=ids01a5

arm-none-eabi-objcopy -O binary sensor-demoestesi.stm32nucleo-spirit1 sensordemo.bin

sudo cp sensordemo.bin /media/diego/NODE_L152RE$1
