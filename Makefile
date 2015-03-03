.PHONY: all
all:
	$(MAKE) -C common/lib all

.PHONY: cclean
cclean:
	$(MAKE) -C common/lib clean

.PHONY: ctest
ctest:
	$(MAKE) -C common/lib test

#TODO: create a test suite for python
.PHONY: pytest
pytest:

.PHONY: test
test: pytest ctest

.PHONY: clean
clean: cclean
