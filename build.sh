#!/bin/sh

docker build \
    -f docker/Dockerfile.build \
     --progress=plain -t \
     foonix-env:latest \
     -o type=local,dest=./out .
