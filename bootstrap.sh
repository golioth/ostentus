#!/bin/bash

# Don't use recursive on submodules because we don't need everything
git submodule update --init

pushd submodules/pimoroni-pico
git submodule update --init
git apply ../../patches/pimoroni-pico-board-fixup.patch
popd

pushd submodules/micropython
git submodule update --init -- lib/pico-sdk lib/tinyusb lib/lwip lib/micropython-lib/ lib/mbedtls
popd

# Patch step used by pimoroni-pico
pushd submodules/micropython/ports/rp2
../../../pimoroni-pico/micropython/_board/board-fixup.sh badger2040 /PIMORONI_BADGER2040 ../../../pimoroni-pico/micropython/_board
make BOARD=PIMORONI_BADGER2040 submodules
popd

# Checkout version of Pico SDK that includes `pico/i2c_slave.h`
# but don't disrupt the patches made in the previous step
pushd submodules/micropython/lib/pico-sdk
git stash
git fetch && git checkout 1.5.1
git stash pop --quiet
popd
