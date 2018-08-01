PWD  := boot
FILE := header.S

SRC  += $(patsubst %,$(PWD)/%,$(FILE))
