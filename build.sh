#!/usr/bin/env bash

docker run --mount type=bind,source="$(pwd)",target="/sar" -u `stat -c "%u:%g" ./` quay.io/pypa/manylinux2014_x86_64 /sar/docker_build.sh
