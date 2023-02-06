# Build Ostentus UF2 using Docker

## Prepare Submodules

Several steps are needed to prepare the submodules of this repo. This includes
selectively cloning the submodules of submodules (a recursive clone pulls far
too many unneccessary packages) and applying patches to submodules.

This prep work has been collecting into a helper script that should be run after
cloning:

```shell
./bootstrap.sh
```

## Prepare Docker

These preparation steps are only necessary once during setup

### Installing Docker

```shell
sudo apt install docker.io
sudo usermod -a -G docker mike
newgrp docker
```

### Building Docker Image

```shell
docker build -t ostentus_build .
```

### Create Docker Container

```shell
docker run -dt \
--user $(id -u):$(id -g) \
--name ostentus \
--mount type=bind,source=$(pwd)/,target=/ostentus \
ostentus_build
```

### Build the Micropythong Cross Compiler

```shell
docker exec -w /ostentus/submodules/micropython ostentus make -C mpy-cross
```

## Building with the Docker package

This section is the standard project build workflow. You will:

1. Enter an interactive bash shell in the Docker container
2. Run the CMake commands from the /ostentus/submodules/micropython/ports/rp2
   directory.
3. Copy the generated firmware.uf2 to the root of this repo.

```shell
docker exec -it ostentus /bin/bash

cmake -S . -B build-$BOARD_TYPE -DPICO_BUILD_DOCS=0 -DUSER_C_MODULES=$BADGER_COMPILE_DIR/pimoroni-pico/micropython/modules/badger2040-micropython.cmake -DMICROPY_BOARD=$BOARD_TYPE -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

cmake --build build-PIMORONI_BADGER2040/ -j 2

#Copy the newly built binary to the repo's root folder:
cp build-PIMORONI_BADGER2040/firmware.uf2 ../../../../.

exit
```

## Removing the Docker container/image

The following commands will remove the Docker container and image from your
system:

```shell
docker stop ostentus
docker rm ostentus
docker rmi ostentus_build
```
