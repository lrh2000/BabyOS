CC      := gcc
CXX     := g++
AS      := gcc
LD      := g++
OBJCOPY := objcopy
OBJDUMP := objdump
RM      := rm -rf
MKDIR   := mkdir -p
DIRNAME := dirname

SRC      :=
BUILD    := ./.kbuild
OUTPUT   := ./output
KERN_LDS := ./kernel.lds
KERN_MAP := $(OUTPUT)/kernel.map
KERN_ELF := $(OUTPUT)/kernel.elf
KERNEL   := $(OUTPUT)/kernel

ASFLAGS  := -c -fPIE -ffreestanding -fno-builtin \
            -nostdlib -nostdinc -I./include
CFLAGS   := -c  ### unused
CXXFLAGS := -c -ffreestanding -fno-stack-protector -mno-red-zone \
            -fPIE -fno-exceptions -fno-rtti -fno-strict-aliasing \
            -mno-sse -mno-mmx -mno-3dnow -std=c++17 -m64 \
            -fno-builtin -nostdlib -nostdinc -Wall -Wextra \
            -Werror -O0 -I./include -mcmodel=small
LDFLAGS  := -Wl,-T$(KERN_LDS) -Wl,-Map=$(KERN_MAP) -Wl,--build-id=none \
            -nostdlib -fno-builtin -ffreestanding -fPIE
OBJCOPYFLAGS  := -O binary

include boot/module.mk

OBJ      := $(patsubst %,$(BUILD)/%.o,$(SRC))
DEP      := $(patsubst %,$(BUILD)/%.d,$(SRC))

.PHONY:first clean veryclean

first: $(KERNEL)

-include $(DEP)

$(BUILD)/%.cpp.d: %.cpp
	./depend.sh $@ $< $(CXX) $(CXXFLAGS)
$(BUILD)/%.S.d: %.S
	./depend.sh $@ $< $(AS) $(ASFLAGS)

$(BUILD)/%.cpp.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<
$(BUILD)/%.S.o: %.S
	$(AS) $(ASFLAGS) -o $@ $<

$(KERNEL): $(DEP) $(OBJ) $(KERN_LDS)
	$(MKDIR) `$(DIRNAME) $(KERNEL)`
	$(LD) $(OBJ) $(LDFLAGS) -o $(KERN_ELF)
	$(OBJCOPY) $(OBJCOPYFLAGS) $(KERN_ELF) $(KERNEL)

clean:
	$(RM) $(BUILD)

veryclean: clean
	$(RM) $(OUTPUT)
