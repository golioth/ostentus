# Ostentus

![Golioth Ostentus board showing the Golioth logo with all LEDs
illuminated](img/ostentus_hardware_revC.jpg)

This repository contains the firmware for the Golioth Ostentus.

Ostentus is a faceplate designed for use with Golioth hardware demonstrations.
The printed circuit board includes five reverse-mounted LEDs for status
indication, an ePaper display for output, three capacitive touch pads, and an
accelerometer. The faceplace receives commands via i2c to control the following
features:

* Show a splashscreen from image file stored on the mass storage partition.
* Display information on slides, including a slide label and data which may be
  updated using partial refresh.
* Control the state of the five status LEDs.
* The may be controlled using capacitive touch input to change to the next and
  previous slides, and to toggle the summary slide.

## Additional information

* [Ostentus Hardware repository](https://github.com/golioth/ostentus-hw)
* This firmware was originally based on the [Pimoroni Badger2040
project](https://github.com/pimoroni/badger2040).

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
    sudo usermod -a -G docker <your_username>
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
[`libostentus/include/libostentus_regmap.h`](https://github.com/golioth/libostentus/blob/main/include/libostentus_regmap.h)
