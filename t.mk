local_prefix := t

local_executable := test-lokatt

local_objects += main.o
local_objects += test-dummy.o
local_objects += test-stack.o
local_objects += test-strbuf.o

local_shared_libraries := liblokatt

include common.mk
