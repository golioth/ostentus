FROM ubuntu:20.04
MAINTAINER Mike Szczys mike@golioth.io

ENV BADGER_COMPILE_DIR=/root/ostentus/submodules
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
    ccache python3 python3-pil vim

