#!/bin/bash

git submodule update --init
cd submodules/pimoroni-pico
git submodule update --init
git apply ../../patches/pimoroni-pico_1.19.10.patch
cd ..
cd micropython
git submodule update --init -- lib/pico-sdk lib/tinyusb
cd ports/rp2
../../../pimoroni-pico/micropython/_board/board-fixup.sh badger2040 /PIMORONI_BADGER2040 ../../../pimoroni-pico/micropython/_board
git apply ../../../../patches/rp2_pio_c.patch
make BOARD=PIMORONI_BADGER2040 submodules
cd ../../../..

