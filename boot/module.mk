PWD  := boot
FILE := header.S initcall.S efi/main.cpp

SRC  += $(patsubst %,$(PWD)/%,$(FILE))
