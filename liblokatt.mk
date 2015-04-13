local_prefix := liblokatt

local_shared_library := liblokatt.so

local_objects += adb-backend.o
local_objects += adb.o
local_objects += device.o
local_objects += dummy-backend.o
local_objects += error.o
local_objects += filter-lexer.o
local_objects += filter.o
local_objects += index.o
local_objects += stack.o
local_objects += strbuf.o

include common.mk
