OUTPUT_FORMAT("elf64-x86-64","elf64-x86-64","elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)
ENTRY(efi_entry)

SECTIONS
{
  . = 0;

  .header : AT(ADDR(.header)) {
    *(.header)
  }
  . = ALIGN(4K);

  .text : AT(ADDR(.text)) {
    *(.text*)
  }
  . = ALIGN(4K);

  .data : AT(ADDR(.data)) {
    *(.rodata*)
    *(.data)

    . = ALIGN(8);
    __init_array_start = .;
    *(SORT(.init_array.*))
    __init_array_end = .;
  }
  __data_end = .;
  . = ALIGN(4K);

  __bss_start = .;
  .bss : AT(ADDR(.bss)) {
    *(.bss)
  }
  . = ALIGN(4K);
  __bss_size = . - __bss_start;

  _end = .;
  /DISCARD/ : {
    *(*)
  }
}

