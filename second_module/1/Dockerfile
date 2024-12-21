FROM ubuntu:24.04

RUN apt-get update && apt-get install -y make gcc \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY Makefile /app
COPY chat.c /app
