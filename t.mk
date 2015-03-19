local_prefix := t

local_executable := test-lokatt

local_objects += main.o

local_shared_libraries := liblokatt

include common.mk
