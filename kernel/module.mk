PWD  := kernel
FILE := cpu/gdt.cpp memory/paging.cpp memory/mm_pages.cpp \
        memory/mm_alloc.cpp video/font_8x16.cpp video/framebuffer.cpp

SRC  += $(patsubst %,$(PWD)/%,$(FILE))
