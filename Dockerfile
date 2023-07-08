FROM ubuntu:latest

RUN apt-get -y update && apt-get install -y

RUN apt-get -y install clang cmake build-essential zlib1g-dev

COPY . /usr/src/sockets

WORKDIR /usr/src/sockets

RUN sh build.sh

CMD ["/bin/bash"]


