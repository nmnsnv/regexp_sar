#!/usr/bin/env bash

# NOTE: This is supposed to run inside manylinux2014_x86_64 container

cd /sar

python3.6 setup.py test
TEST_STATUS=$?

if [[ $TEST_STATUS -ne 0 ]]; then
    echo "SAR Tests Failed!" >&2
    exit 1
fi

python3.6 setup.py sdist
python3.6 setup.py bdist_wheel

cd dist/
PACKAGE_NAME=$(ls | grep 'whl$')
VALID_NAME=$(perl -e '$ARGV[0] =~ s/cp3\d+m-linux_x86_64/abi3-manylinux2010_x86_64/; print $ARGV[0] ."\n"' "$PACKAGE_NAME")

mv "$PACKAGE_NAME" "$VALID_NAME"

cd ../

rm -r build
rm -r regexp_sar.egg-info/
rm -r regexp_sar/*.so
rm -r .eggs/
