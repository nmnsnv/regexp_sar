
build:
	./build.sh

clean:
	rm -rf dist regexp_sar.egg-info build

upload-test:
	python3 -m twine upload --repository testpypi dist/*
