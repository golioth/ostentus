FROM ubuntu:20.04
MAINTAINER Mike Szczys mike@golioth.io

ENV BADGER_COMPILE_DIR=/ostentus/submodules
ENV BOARD_TYPE=RPI_PICO
ENV CCACHE_DIR=/tmp/.ccache
ENV CCACHE_TEMPDIR=/tmp/.ccache

ENV TZ=America/Chicago

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get -y update
RUN apt-get -y upgrade
RUN apt-get install -y openssh-client build-essential libreadline-dev \
    libffi-dev git pkg-config gcc-arm-none-eabi libnewlib-arm-none-eabi cmake \
    ccache python3 python3-pil vim

RUN mkdir /ostentus
RUN chmod 777 /ostentus
WORKDIR /ostentus/submodules/micropython/ports/rp2
