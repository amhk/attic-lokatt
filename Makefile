# Naming all PHONY targets
.PHONY: all clean cclean test ctest pytest

all:
	$(MAKE) -C common/lib all

cclean:
	$(MAKE) -C common/lib clean

ctest:
	$(MAKE) -C common/lib test

#TODO: create a test suite for python
pytest:

test: pytest ctest

clean: cclean
