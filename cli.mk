local_prefix := cli

local_executable := lokatt

local_objects += main.o

local_shared_libraries := liblokatt

include common.mk
