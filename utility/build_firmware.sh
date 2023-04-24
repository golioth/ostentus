#!/bin/bash

# Helper script used to execute build commands inside the docker container.
# $ docker exec -i ostentus bash < utility/build_firmware.sh

cmake -S . \
		-B build-$BOARD_TYPE \
		-DPICO_BUILD_DOCS=0 \
		-DUSER_C_MODULES=$BADGER_COMPILE_DIR/pimoroni-pico/micropython/modules/badger2040-micropython.cmake \
		-DMICROPY_BOARD=$BOARD_TYPE \
		-DCMAKE_C_COMPILER_LAUNCHER=ccache \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache

cmake --build build-PIMORONI_BADGER2040/ -j 2

#Copy the newly built binary to the repo's root folder:
cp build-PIMORONI_BADGER2040/firmware.uf2 ../../../../.

