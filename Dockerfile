# syntax=docker/dockerfile:experimental
FROM ubuntu:20.04
MAINTAINER Mike Szczys mike@golioth.io

ENV BADGER_COMPILE_DIR=/root/badger
ENV OSTENTUS_VERSION=v0.0.1
ENV PIMORONI_PICO_VERSION=szczys/ostentus_v1.19.10
ENV MICROPYTHON_VERSION=9dfabcd6d3d080aced888e8474e921f11dc979bb
ENV BOARD_TYPE=PIMORONI_BADGER2040

ENV TZ=America/Chicago

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get -y update
RUN apt-get -y upgrade
RUN apt-get install -y openssh-client build-essential libreadline-dev \
    libffi-dev git pkg-config gcc-arm-none-eabi libnewlib-arm-none-eabi cmake \
    ccache python3 python3-pil


RUN mkdir -p -m 0600 ~/.ssh && \
    ssh-keyscan -H github.com >> ~/.ssh/known_hosts

RUN mkdir -p $BADGER_COMPILE_DIR
WORKDIR $BADGER_COMPILE_DIR
RUN --mount=type=ssh git clone git@github.com:golioth/ostentus.git
WORKDIR $BADGER_COMPILE_DIR/ostentus
RUN --mount=type=ssh git checkout $OSTENTUS_VERSION

WORKDIR $BADGER_COMPILE_DIR
RUN git clone https://github.com/szczys/pimoroni-pico.git
WORKDIR $BADGER_COMPILE_DIR/pimoroni-pico
RUN git checkout $PIMORONI_PICO_VERSION
RUN git submodule update --init

WORKDIR $BADGER_COMPILE_DIR
RUN git clone https://github.com/micropython/micropython.git
WORKDIR $BADGER_COMPILE_DIR/micropython
RUN git checkout $MICROPYTHON_VERSION
RUN git submodule update --init -- lib/pico-sdk lib/tinyusb

WORKDIR $BADGER_COMPILE_DIR/micropython/ports/rp2
RUN echo $(pwd)
RUN /bin/bash $BADGER_COMPILE_DIR/pimoroni-pico/micropython/_board/board-fixup.sh badger2040 /$BOARD_TYPE $BADGER_COMPILE_DIR/pimoroni-pico/micropython/_board
RUN make BOARD=PIMORONI_BADGER2040 submodules
RUN cmake -S . -B build-$BOARD_TYPE -DPICO_BUILD_DOCS=0 -DUSER_C_MODULES=$BADGER_COMPILE_DIR/pimoroni-pico/micropython/modules/badger2040-micropython.cmake -DMICROPY_BOARD=$BOARD_TYPE -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

WORKDIR $BADGER_COMPILE_DIR/micropython/ports/rp2/modules/
RUN cp $BADGER_COMPILE_DIR/ostentus/ostentus.py .
RUN cp $BADGER_COMPILE_DIR/ostentus/ostentus_leds.py .
RUN cp $BADGER_COMPILE_DIR/ostentus/splashscreen_rd.py .
WORKDIR $BADGER_COMPILE_DIR/micropython/ports/rp2

RUN cmake --build build-PIMORONI_BADGER2040/ -j 2

