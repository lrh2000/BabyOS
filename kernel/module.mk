PWD  := kernel
FILE := cpu/gdt.cpp memory/paging.cpp memory/mm_pages.cpp

SRC  += $(patsubst %,$(PWD)/%,$(FILE))
