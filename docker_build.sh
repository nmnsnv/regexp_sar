#!/usr/bin/env bash

# NOTE: This is supposed to run inside manylinux2014_x86_64 container

cd /sar

python3.6 setup.py sdist
python3.6 setup.py bdist_wheel

cd dist/
PACKAGE_NAME=$(ls | grep 'whl$')
VALID_NAME=$(perl -e '$ARGV[0] =~ s/cp3\d+m-linux_x86_64/abi3-manylinux2010_x86_64/; print $ARGV[0] ."\n"' "$PACKAGE_NAME")

mv "$PACKAGE_NAME" "$VALID_NAME"

cd ../

rm -r build
rm -r regexp_sar.egg-info/
