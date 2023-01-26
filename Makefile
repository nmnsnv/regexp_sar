
build:
	./build.sh

clean:
	rm -rf dist regexp_sar.egg-info build .eggs/ regexp_sar/*.so

upload-test: clean build
	python3 -m twine upload --repository testpypi dist/*
