FROM alpine:edge

RUN apk add --no-cache libgcc gcc g++ make cmake git zlib-dev

COPY . /usr/src/sockets

WORKDIR /usr/src/sockets

RUN sh build.sh

CMD ["./build/test_tree"]


