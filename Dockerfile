# Project: TCP Server v1.0

FROM ubuntu:20.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt update && \
    apt install -y qt5-default build-essential && \
    apt clean

WORKDIR /app/server
COPY . .

RUN qmake QtEchoServerClean.pro && make
RUN mv QtEchoServerClean tcp_server

EXPOSE 12345
CMD ["./tcp_server"]