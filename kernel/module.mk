PWD  := kernel
FILE := cpu/gdt.cpp memory/paging.cpp

SRC  += $(patsubst %,$(PWD)/%,$(FILE))
