.PHONY: all
all: liblokatt t cli

.PHONY: test
test: liblokatt t
	@LD_LIBRARY_PATH=out/liblokatt out/t/test-lokatt $(T)

.PHONY: lokatt
lokatt: liblokatt cli
	@LD_LIBRARY_PATH=out/liblokatt out/cli/lokatt

.PHONY: clean
clean:
	$(RM) -r out

include clean.mk
include liblokatt.mk

include clean.mk
include t.mk

include clean.mk
include cli.mk
