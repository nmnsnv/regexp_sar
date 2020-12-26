from setuptools import setup, Extension


def main():
    with open("README.md", "r", encoding="utf-8") as fh:
        long_description = fh.read()
    setup(name="regexp-sar",
          version="0.1.0b",

          description="SAR Regexp Engine for Python",
          long_description=long_description,
          long_description_content_type="text/markdown",

          url="https://github.com/nmnsnv/regexp_sar",

          author="Noam Nisanov",
          author_email="noam.nisanov@gmail.com",

          test_suite='nose.collector',
          tests_require=['nose'],

          ext_modules=[Extension("regexp_sar._regexp_sar_c",
                                 ["sarmodule.c", "sarcore.c"])],
          headers=['sarcore.h'],
          license="MIT",
          python_requires='>=3',
          packages=["regexp_sar"])


if __name__ == "__main__":
    main()
