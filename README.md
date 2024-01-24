# Ostentus

Use a Pimoroni Badger ePaper display as an i2c-controlled display for Golioth
Reference Designs.

## Installation

Precompiled `.uf2` binaries can be found in the Releases section.


### Building firmware

Micropython has known issues building with some CMake versions. This repository
includes a Dockerfile that will install a known-working build environment.
Installation and use of the Docker container has been simplified as much as
possible.

0. After cloning the repo, run the bootstrap script to configure submodules

    ```shell
    ./bootstrap.sh
    ```

1. Install Docker on your system

    **Note**: If you have installed
    [Docker Desktop](https://www.docker.com/products/docker-desktop/) on macOS
    you can skip this step.

    ```shell
    sudo apt install docker.io
    sudo usermod -a -G docker mike
    newgrp docker
    ```

2. Perform initialization of the Docker image/container

    ```shell
    make docker_init

    # macOS
    make docker_init_mac
    ```

3. Build firmware

    ```shell
    make
    ```

The result of running `make` is a `firmware.uf2` file in the root directory of
this repository that can be flashed onto the Pico.

If you encounter issues using this approach, please consult the [Build with
Docker](build-with-docker.md) guide for building the Ostentus binary.

## Controlling Ostentus

It is recommended that you use the `libostentus` library to control Ostentus
from Zephyr. (This is included as [a submodule of this repo](libostentus).) Add
[the libostentus repo](https://github.com/golioth/libostentus) to your Zephyr
project as a submodule.

Use any i2c-controller to write to the display on device-address `0x12`.

The source of truth for the memory registers is from the `golioth/libostentus`
repository (included as a submodule). Please consult
`libostentus/include/libostentus_regmap.h`

## Example:

Using micropython on a Raspberry Pi Pico:

```python
>>> i2c.scan()
[18]
>>> i2c.writeto_mem(0x12, 0x20, "Hello World!")
>>> i2c.writeto_mem(0x12, 0x22, "Golioth rulez!")
>>> i2c.writeto_mem(0x12, 0x01, b'\x00')
```

This example scans for and prints out the ostentus i2c device (address `0x12`)
before writing to lines 0 and 2 in memory and using a full refresh to show those
values on the display.

![Badger2040](img/ostentus_badger2040.jpg)

## i2cperipheral micropython library

This is based on the [i2cperipheral library for the
RP2040](https://github.com/adamgreen/i2cperipheral). The `.mpy` file that is
included in this library is too old for the Badger2040 build. Use the following
steps to build a more modern version:

1. Clone the `i2cperipheral` repo
2. Run `make init` to initialize the library
3. Enter the `micropython` library and run `git stash`
4. Use `git checkout v1.19` to check out the correct version
5. Go back to the i2cperipheral directory and run `make init`
6. Run `make` to build i2cperipheral.mpy which can be placed on the Badger2040
