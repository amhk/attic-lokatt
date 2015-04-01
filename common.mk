ifndef local_prefix
$(error local_prefix not defined)
endif

ifndef local_objects
$(error local_objects not defined)
endif

ifneq ($(and $(local_shared_library),$(local_executable)),)
$(error "local_shared_library and local_executable are mutually exclusive")
endif

out := out/$(local_prefix)
objects := $(foreach o,$(local_objects),$(out)/$(o))
deps := $(objects:.o=.d)

CC := clang
CFLAGS :=
CFLAGS += -Wall -Wextra
CFLAGS += -fPIC
CFLAGS += -I.
CFLAGS += -ggdb -O0

LN := clang
LNFLAGS := $(CFLAGS) -pthread
LIBS :=

LEX := flex
LEXFLAGS :=

ifdef local_shared_libraries
LNFLAGS += $(foreach lib,$(local_shared_libraries),-Lout/$(lib))
LIBS += $(foreach lib,$(local_shared_libraries:lib%=%),-l$(lib))
endif

ifndef V
	QUIET_CC = @echo "    CC $@";
	QUIET_DEP = @echo "    DEP $@";
	QUIET_LN = @echo "    LINK $@";
	QUIET_MKDIR = @echo "    MKDIR $@";
	QUIET_LEX = @echo "    LEX $@";
endif

$(out):
	$(QUIET_MKDIR)mkdir -p $@

.PRECIOUS: $(out)/%.h $(out)/%.c
$(out)/%.h $(out)/%.c: $(local_prefix)/%.lex | $(out)
	$(QUIET_LEX)$(LEX) $(LEXFLAGS) $<

$(out)/%.d: $(local_prefix)/%.c | $(out)
	$(QUIET_DEP)$(CC) $(CFLAGS) -MM -MG $< | sed "s+.*:+$@ $@:+" | sed 's+\.d+.o+' >$@

$(out)/%.o: $(out)/%.c | $(out)
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

$(out)/%.o: $(local_prefix)/%.c | $(out)
	$(QUIET_CC)$(CC) $(CFLAGS) -c -o $@ $<

ifdef local_shared_library
.PHONY: $(local_prefix)
$(local_prefix): $(out)/$(local_shared_library)

$(out)/$(local_shared_library): $(objects) | $(out)
	$(QUIET_LN)$(LN) $(LNFLAGS) -shared -o $@ $^
endif

ifdef local_executable
.PHONY: $(local_prefix)
$(local_prefix): $(out)/$(local_executable)

ifdef local_shared_libraries
$(out)/$(local_executable): $(foreach lib,$(local_shared_libraries),out/$(lib)/$(lib).so)
endif

$(out)/$(local_executable): $(objects) | $(out)
	$(QUIET_LN)$(LN) $(LNFLAGS) -o $@ $^ $(LIBS)
endif

-include $(deps)
