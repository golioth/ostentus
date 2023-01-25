FROM ubuntu:20.04
MAINTAINER Mike Szczys mike@golioth.io

ENV TZ=America/Chicago

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get -y update
RUN apt-get -y upgrade
RUN apt-get install -y build-essential libreadline-dev libffi-dev git pkg-config gcc-arm-none-eabi libnewlib-arm-none-eabi cmake ccache python3 python3-pil
RUN mkdir /root/badger

# Hack to add a non-public repo:
ADD ostentus /root/badger/ostentus
