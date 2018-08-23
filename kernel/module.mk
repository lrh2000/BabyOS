PWD  := kernel
FILE := cpu/gdt.cpp memory/paging.cpp memory/mm_pages.cpp \
        memory/mm_alloc.cpp

SRC  += $(patsubst %,$(PWD)/%,$(FILE))
