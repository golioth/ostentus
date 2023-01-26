# Building micropython for Badger2040

These are vanilla build instructions for the Pimoroni Pico verision of
micropython for the Bager2040. The ostentus.py file can be added to the modules
directory near the end of the walkthrough.

After installing the generated UF2 file, you will still need to copy
i2cperipheral.mpy onto the Badger filesystem.

## Docker

Use the Dockerfile from this repo to generate a build environment:

```
## Allow image to access private repos via your local keys
eval $(ssh-agent)
ssh-add ~/.ssh/id_rsa
# (type passphrase if prompted)

## Generate container image
DOCKER_BUILDKIT=1 docker build --ssh default=$SSH_AUTH_SOCK -t "ostentus:Dockerfile" .
```

At this point the Micropython will have been packaged along with Ostentus files.
You can access the .uf2 as follows:

```
container_id=$(docker create "ostentus:Dockerfile")
docker cp "$container_id:/root/badger/micropython/ports/rp2/build-PIMORONI_BADGER2040/firmware.uf2" .
docker rm "$container_id"
```

If you prefer to enter the container use this command:

```
docker run -it ostentus:Dockerfile /bin/bash

## Working dir is /root/badger/micropython/ports/rp2

## The build command is:
cmake --build build-PIMORONI_BADGER2040/ -j 2
```

## Manually building the package

Change the first line to the path you'd like to use for all packages needed for
this process.

```shell
export BADGER_COMPILE_DIR=/home/mike/golioth-compile/badger

mkdir $BADGER_COMPILE_DIR
export OSTENTUS_VERSION=szczys/hardware_bringup
export PIMORONI_PICO_VERSION=szczys/ostentus_v1.19.10
export MICROPYTHON_VERSION=9dfabcd6d3d080aced888e8474e921f11dc979bb
export BOARD_TYPE=PIMORONI_BADGER2040

cd $BADGER_COMPILE_DIR

git clone git@github.com:golioth/ostentus.git
cd ostentus
git checkout $OSTENTUS_VERSION
cd ..

git clone git@github.com:szczys/pimoroni-pico.git
cd pimoroni-pico
git checkout $PIMORONI_PICO_VERSION
git submodule update --init
cd ..

git clone https://github.com/micropython/micropython.git
cd micropython
git checkout $MICROPYTHON_VERSION
git submodule update --init -- lib/pico-sdk lib/tinyusb
make -C mpy-cross
cd ports/rp2
$BADGER_COMPILE_DIR/pimoroni-pico/micropython/_board/board-fixup.sh badger2040 /$BOARD_TYPE $BADGER_COMPILE_DIR/pimoroni-pico/micropython/_board
## If you screw up the previous line, go into $BADGER_COMPILE_DIR/micropython/lib/pico-sdk and restore the changed files (ask me how I know)

make BOARD=PIMORONI_BADGER2040 submodules
cmake -S . -B build-$BOARD_TYPE -DPICO_BUILD_DOCS=0 -DUSER_C_MODULES=$BADGER_COMPILE_DIR/pimoroni-pico/micropython/modules/badger2040-micropython.cmake -DMICROPY_BOARD=$BOARD_TYPE -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

## Copy ostentus files into the modules directory
cp $BADGER_COMPILE_DIR/ostentus/boot.py $BADGER_COMPILE_DIR/micropython/ports/rp2/modules/.
cp $BADGER_COMPILE_DIR/ostentus/ostentus.py $BADGER_COMPILE_DIR/micropython/ports/rp2/modules/.
cp $BADGER_COMPILE_DIR/ostentus/ostentus_leds.py $BADGER_COMPILE_DIR/micropython/ports/rp2/modules/.
cp $BADGER_COMPILE_DIR/ostentus/splashscreen_rd.py $BADGER_COMPILE_DIR/micropython/ports/rp2/modules/.

cmake --build build-PIMORONI_BADGER2040/ -j 2

```

## Known issues

Not building on Linux Mint 21 (5.15.0-53) using GNU Make 4.3

```shell
In file included from /home/mike/golioth-compile/badger/micropython/py/mpstate.h:33,
                 from /home/mike/golioth-compile/badger/micropython/py/runtime.h:29,
                 from /home/mike/golioth-compile/badger/pimoroni-pico/micropython/modules/badger2040/badger2040.h:2,
                 from /home/mike/golioth-compile/badger/pimoroni-pico/micropython/modules/badger2040/badger2040.cpp:8:
/home/mike/golioth-compile/badger/pimoroni-pico/micropython/modules/badger2040/badger2040.cpp: In function 'std::string mp_obj_to_string_r(void* const&)':
/home/mike/golioth-compile/badger/micropython/py/misc.h:54:53: error: size of array is not an integral constant-expression
   54 | #define MP_STATIC_ASSERT(cond) ((void)sizeof(char[1 - 2 * !(cond)]))
      |                                                   ~~^~~~~~~~~~~~~
```
