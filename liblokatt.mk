local_prefix := liblokatt

local_shared_library := liblokatt.so

local_objects += device.o
local_objects += error.o

include common.mk
