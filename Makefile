liblokatt_objects :=
liblokatt_objects += error.o
liblokatt_objects += ring-buffer.o

binary := lokatt

test_binaries :=
test_binaries += test-ring-buffer

headers :=
headers += error.h
headers += ring-buffer.h

liblokatt = liblokatt.a
test_objects := $(patsubst %, %.o, $(test_binaries))
objects := $(binary).o $(liblokatt_objects) $(test_objects)
deps := $(objects:.o=.d)
sources := $(objects:.o=.c) $(headers)
plists := $(objects:.o=.plist)
tags := tags

CC := clang
CFLAGS := -Wall -Wextra -pthread -ggdb -O0
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
endif

%.d: %.c
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM $< | sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' > $@

%.o: %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

%.plist: %.c
	$(QUIET_ANALYZE)$(CC) $(CFLAGS) --analyze -o $@ $<

test-%: test-%.o $(LIBS)
	$(QUIET_LD)$(LD) $(LDFLAGS) -o $@ $^

all: $(tags) $(binary) $(test_binaries)

$(liblokatt): $(liblokatt_objects)
	$(QUIET_AR)$(RM) $@ && $(AR) rcs $@ $^

$(binary): $(binary).o $(LIBS)
	$(QUIET_LD)$(LD) $(LDFLAGS) -o $@ $^

analyze: $(plists)

$(tags): $(sources)
	$(QUIET_TAGS)ctags --fields=+l $(sources)

clean:
	$(RM) $(deps)
	$(RM) $(objects)
	$(RM) $(LIBS)
	$(RM) $(plists)
	$(RM) $(tags)
	$(RM) $(binary) $(test_binaries)

-include $(deps)
