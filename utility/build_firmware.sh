#!/bin/bash

# Helper script used to execute build commands inside the docker container.
# $ docker exec -i ostentus bash < utility/build_firmware.sh

cmake -S . \
		-B build-RPI_PICO \
		-DPICO_BUILD_DOCS=0 \
    -DUSER_C_MODULES="../../../gc9a01_mpy/src/micropython.cmake;../../../pico_i2c/micropython.cmake" \
#    -DUSER_C_MODULES=../../../pico_i2c/micropython.cmake \
		-DMICROPY_BOARD=RPI_PICO \
		-DCMAKE_C_COMPILER_LAUNCHER=ccache \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache

cmake --build build-RPI_PICO/ -j 2

#Copy the newly built binary to the repo's root folder:
cp build-RPI_PICO/firmware.uf2 ../../../../.

