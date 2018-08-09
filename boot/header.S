  .section ".header","a"
  .extern _end
  .global _head

_head:
dos_header:
  # IMAGE_DOS_HEADER
  .ascii "MZ"                           # e_magic
  .org 0x3c
  .long pe_header - _head               # e_lfanew

pe_header:
  # IMAGE_NT_HEADER
  .ascii "PE\0\0"                       # Signature

file_header:
  # IMAGE_FILE_HEADER       FileHeader
  .word 0x8664                          # Machine (IMAGE_FILE_MACHINE_AMD64)
  .word 2                               # NumberOfSections
  .long 0                               # TimeDateStamp
  .long 0                               # PointerToSymbolsTable
  .long 0                               # NumberOfSymbols
  .word section_table - optional_header # SizeOfOptionalHeader
  .word 0x206                           # Characteristics
                                        # IMAGE_FILE_EXECUTABLE_IMAGE
                                        # IMAGE_FILE_LINE_NUMS_STRIPPED
                                        # IMAGE_FILE_DEBUG_STRIPPED

optional_header:
  # IMAGE_OPTIONAL_HEADER32 OptionalHeader
  .word 0x20b                           # Magic (IMAGE_NT_OPTIONAL_HDR64_MAGIC)
  .byte 0x02                            # MajorLinkerVersion
  .byte 0x14                            # MinorLinkerVersion
  .long __data_end - __text_start       # SizeOfCode
  .long 0                               # SizeOfInitializedData
  .long 0                               # SizeOfUninitializedData
  .long efi_entry - _head               # AddressOfEntry
  .long __text_start - _head            # BaseOfCode
  .quad 0                               # ImageBase
  .long 0x1                             # SectionAlignment
  .long 0x1                             # FileAlignment
  .word 0                               # MajorOperationSystemVersion
  .word 0                               # MinorOperationSystemVersion
  .word 0                               # MajorImageVersion
  .word 0                               # MinorImageVersion
  .word 0                               # MajorSubsystemVersion
  .word 0                               # MinorSubsystemVersion
  .long 0                               # Win32VersionValue
  .long _end - _head                    # SizeOfImage
  .long header_end - dos_header         # SizeOfHeader
  .long 0                               # CheckSum
  .word 0xa                             # Subsystem (IMAGE_SUBSYSTEM_EFI_APPLICATION)
  .word 0                               # DllCharacteristics
  .quad 0                               # SizeOfStackReserve
  .quad 0                               # SizeOfStackCommit
  .quad 0                               # SizeOfHeapReserve
  .quad 0                               # SizeOfHeapCommit
  .long 0                               # LoaderFlags

  .long 0x6                             # NumberOfRvaAndSizes
  # IMAGE_DATA_DIRECTORY DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES]
  .quad 0                               # Export table
  .quad 0                               # Import table
  .quad 0                               # Resource table
  .quad 0                               # Exception table
  .quad 0                               # Certicate table
  .quad 0                               # Base relocation table

section_table:
  # IMAGE_SECTION_HEADER
  .ascii ".text\0\0\0"                  # Name
  .long __data_end - __text_start       # VirtualSize
  .long __text_start - _head            # VirtualAddress
  .long __data_end - __text_start       # SizeOfRawData
  .long __text_start - _head            # PointerToRawData
  .long 0                               # PointerToRelocations
  .long 0                               # PointerToLinenumbers
  .word 0                               # NumberOfRelocations
  .word 0                               # NumberOfLinenumbers
  .long 0xe0000060                      # Characteristics
                                        # IMAGE_SCN_CNT_CODE
                                        # IMAGE_SCN_CNT_INITIALIZED_DATA
                                        # IMAGE_SCN_MEM_EXECUTE
                                        # IMAGE_SCN_MEM_READ
                                        # IMAGE_SCN_MEM_WRITE

  .ascii ".bss\0\0\0\0"                 # Name
  .long __bss_size - _head              # VirtualSize
  .long __bss_start - _head             # VirtualAddress
  .long 0                               # SizeOfRawData
  .long 0                               # PointerToRawData
  .long 0                               # PointerToRelocations
  .long 0                               # PointerToLinenumbers
  .word 0                               # NumberOfRelocations
  .word 0                               # NumberOfLinenumbers
  .long 0xc0000080                      # Characteristics
                                        # IMAGE_CNT_UNINITIALIZED_DATA
                                        # IMAGE_MEM_READ
                                        # IMAGE_MEM_WRITE

header_end:

.align 0x1000
__text_start:

  .section ".text","ax"
  .code64
  .extern exec_init_func
  .global efi_entry
efi_entry:
  movq %rcx,%rsi
  // movq %rdx,%rdx
  call exec_init_func
  cli
0:
  hlt
  jmp 0b
