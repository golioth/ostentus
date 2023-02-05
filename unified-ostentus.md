

```shell
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
```

## Installing Docker

```shell
sudo apt install docker.io
sudo usermod -a -G docker mike
newgrp docker
```

## Building Docker Image

```shell
docker build -t ostentus_build .
```


## Running Docker Image

### Create container

```shell
docker run -dt \
--name ostentus \
--mount type=bind,source=$(pwd)/,target=/root/ostentus \
ostentus_build
```

### Enter container via bash

```shell
docker exec -it ostentus /bin/bash
cd /root/ostentus/submodules/micropython/

# One time to build the cross compiler:
make -C mpy-cross

cd ports/rp2
cmake -S . -B build-$BOARD_TYPE -DPICO_BUILD_DOCS=0 -DUSER_C_MODULES=$BADGER_COMPILE_DIR/pimoroni-pico/micropython/modules/badger2040-micropython.cmake -DMICROPY_BOARD=$BOARD_TYPE -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
cmake --build build-PIMORONI_BADGER2040/ -j 2

#Copy the newly built binary to the repo's root folder:
cp build-PIMORONI_BADGER2040/firmware.uf2 ../../../../.
```
