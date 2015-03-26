local_prefix := liblokatt

local_shared_library := liblokatt.so

local_objects += adb-backend.o
local_objects += device.o
local_objects += dummy-backend.o
local_objects += error.o
local_objects += index.o

include common.mk
