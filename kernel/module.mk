PWD  := kernel
FILE := cpu/gdt.cpp cpu/idt.cpp memory/paging.cpp memory/mm_pages.cpp \
        memory/mm_alloc.cpp video/font_8x16.cpp video/framebuffer.cpp \
        video/console.cpp debug/log.cpp cpu/exceptions.cpp cpu/exceptions.S \
        irq/entries.S irq/lapic.cpp irq/handler.cpp

SRC  += $(patsubst %,$(PWD)/%,$(FILE))
