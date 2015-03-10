liblokatt_objects :=
liblokatt_objects += adb.o
liblokatt_objects += bridge.o
liblokatt_objects += demux.o
liblokatt_objects += dict.o
liblokatt_objects += error.o
liblokatt_objects += ring-buffer.o
liblokatt_objects += strbuf.o

binary := lokatt

test_binaries :=
test_binaries += test-adb
test_binaries += test-dict
test_binaries += test-ring-buffer
test_binaries += test-strbuf

headers :=
headers += adb.h
headers += bridge.h
headers += error.h
headers += lokatt.h
headers += ring-buffer.h
headers += strbuf.h
headers += test.h

liblokatt = liblokatt.so
test_objects := $(patsubst %, %.o, $(test_binaries)) test.o
objects := $(binary).o $(liblokatt_objects) $(test_objects)
deps := $(objects:.o=.d)
sources := $(objects:.o=.c) $(headers)
plists := $(objects:.o=.plist)
tags := tags

CC := clang
CFLAGS := -Wall -Wextra -pthread -ggdb -O0 -fPIC
CFLAGS += -DDEBUG

LD := $(CC)
LDFLAGS := $(CFLAGS)
LIBS := $(liblokatt)

ifndef V
	QUIET_DEP = @echo "    DEP $@";
	QUIET_CC = @echo "    CC $@";
	QUIET_LD = @echo "    LINK $@";
	QUIET_AR = @echo "    AR $@";
	QUIET_ANALYZE = @echo "    ANALYZE $@";
	QUIET_TAGS = @echo "    TAGS";
	QUIET_TEST = @echo "    TEST";
endif

ifdef VALGRIND
	TESTFLAGS := --valgrind
endif

ifdef GDB
	TESTFLAGS := --gdb
endif

ifndef T
	T := $(test_binaries)
endif

%.d: %.c
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@

%.o: %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

%.plist: %.c
	$(QUIET_ANALYZE)$(CC) $(CFLAGS) --analyze -o $@ $<

test-%: test-%.o test.o $(LIBS)
	$(QUIET_LD)$(LD) $(LDFLAGS) -o $@ $^

.PHONY: all
all: $(tags) $(binary) $(test_binaries)

$(LIBS): $(liblokatt_objects)
	$(QUIET_LD)$(LD) $(LDFLAGS) -shared -o $@ $^

$(binary): $(binary).o $(LIBS)
	$(QUIET_LD)$(LD) $(LDFLAGS) -o $@ $^

.PHONY: analyze
analyze: $(plists)

$(tags): $(sources)
	$(QUIET_TAGS)ctags --fields=+l $(sources)

.PHONY: test
test: $(test_binaries)
	$(QUIET_TEST)./run-tests.py $(TESTFLAGS) $(T)

.PHONY: clean
clean:
	$(RM) $(deps)
	$(RM) $(objects)
	$(RM) $(LIBS)
	$(RM) $(plists)
	$(RM) $(tags)
	$(RM) $(binary) $(test_binaries)

-include $(deps)
