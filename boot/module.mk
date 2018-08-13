PWD  := boot
FILE := header.S initcall.S memory.cpp efi/main.cpp

SRC  += $(patsubst %,$(PWD)/%,$(FILE))
