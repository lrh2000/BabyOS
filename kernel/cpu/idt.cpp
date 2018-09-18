#include <env.hpp>
#include <init.hpp>
#include <debug.hpp>
#include "gdt.hpp"

extern "C" void exception_entry_0(void);
extern "C" void exception_entry_1(void);
extern "C" void exception_entry_2(void);
extern "C" void exception_entry_3(void);
extern "C" void exception_entry_4(void);
extern "C" void exception_entry_5(void);
extern "C" void exception_entry_6(void);
extern "C" void exception_entry_7(void);
extern "C" void exception_entry_8(void);
extern "C" void exception_entry_9(void);
extern "C" void exception_entry_10(void);
extern "C" void exception_entry_11(void);
extern "C" void exception_entry_12(void);
extern "C" void exception_entry_13(void);
extern "C" void exception_entry_14(void);
extern "C" void exception_entry_15(void);
extern "C" void exception_entry_16(void);
extern "C" void exception_entry_17(void);
extern "C" void exception_entry_18(void);
extern "C" void exception_entry_19(void);
extern "C" void exception_entry_20(void);
extern "C" void exception_entry_21(void);
extern "C" void exception_entry_22(void);
extern "C" void exception_entry_23(void);
extern "C" void exception_entry_24(void);
extern "C" void exception_entry_25(void);
extern "C" void exception_entry_26(void);
extern "C" void exception_entry_27(void);
extern "C" void exception_entry_28(void);
extern "C" void exception_entry_29(void);
extern "C" void exception_entry_30(void);
extern "C" void exception_entry_31(void);

namespace idt
{
  enum : uint64_t
  {
    TRAP_GATE = 0x00000f0000000000,
    INTR_GATE = 0x00000e0000000000,
  };
  enum : uint64_t
  {
    KERN_GATE = 0x0000000000000000,
    USER_GATE = 0x0000600000000000,
  };

  struct entry_t
  {
    uint64_t lower_data;
    uint64_t upper_data;

    void set(uint64_t offset,uint64_t attributes)
    {
      upper_data = offset >> 32;
      lower_data = (offset & 0x0000ffff) | ((offset & 0xffff0000) << 32);
      lower_data |= attributes;
      lower_data |= 0x0000800000000000ull;
      lower_data |= (uint64_t)gdt::KERNEL_CS << 16;
    }

    bool present(void)
    {
      return lower_data & 0x0000800000000000;
    }
  };

  static entry_t desc_table[64];
  static uint8_t desc_table_reg[10];

  static int setup_idt(void) INIT_FUNC(kernel,CPU_IDT);

  static int setup_idt(void)
  {
    desc_table[0].set((uint64_t)&exception_entry_0,KERN_GATE | TRAP_GATE);
    desc_table[1].set((uint64_t)&exception_entry_1,KERN_GATE | TRAP_GATE);
    desc_table[2].set((uint64_t)&exception_entry_2,KERN_GATE | TRAP_GATE);
    desc_table[3].set((uint64_t)&exception_entry_3,KERN_GATE | TRAP_GATE);
    desc_table[4].set((uint64_t)&exception_entry_4,KERN_GATE | TRAP_GATE);
    desc_table[5].set((uint64_t)&exception_entry_5,KERN_GATE | TRAP_GATE);
    desc_table[6].set((uint64_t)&exception_entry_6,KERN_GATE | TRAP_GATE);
    desc_table[7].set((uint64_t)&exception_entry_7,KERN_GATE | TRAP_GATE);
    desc_table[8].set((uint64_t)&exception_entry_8,KERN_GATE | TRAP_GATE);
    desc_table[9].set((uint64_t)&exception_entry_9,KERN_GATE | TRAP_GATE);
    desc_table[10].set((uint64_t)&exception_entry_10,KERN_GATE | TRAP_GATE);
    desc_table[11].set((uint64_t)&exception_entry_11,KERN_GATE | TRAP_GATE);
    desc_table[12].set((uint64_t)&exception_entry_12,KERN_GATE | TRAP_GATE);
    desc_table[13].set((uint64_t)&exception_entry_13,KERN_GATE | TRAP_GATE);
    desc_table[14].set((uint64_t)&exception_entry_14,KERN_GATE | TRAP_GATE);
    desc_table[15].set((uint64_t)&exception_entry_15,KERN_GATE | TRAP_GATE);
    desc_table[16].set((uint64_t)&exception_entry_16,KERN_GATE | TRAP_GATE);
    desc_table[17].set((uint64_t)&exception_entry_17,KERN_GATE | TRAP_GATE);
    desc_table[18].set((uint64_t)&exception_entry_18,KERN_GATE | TRAP_GATE);
    desc_table[19].set((uint64_t)&exception_entry_19,KERN_GATE | TRAP_GATE);
    desc_table[20].set((uint64_t)&exception_entry_20,KERN_GATE | TRAP_GATE);
    desc_table[21].set((uint64_t)&exception_entry_21,KERN_GATE | TRAP_GATE);
    desc_table[22].set((uint64_t)&exception_entry_22,KERN_GATE | TRAP_GATE);
    desc_table[23].set((uint64_t)&exception_entry_23,KERN_GATE | TRAP_GATE);
    desc_table[24].set((uint64_t)&exception_entry_24,KERN_GATE | TRAP_GATE);
    desc_table[25].set((uint64_t)&exception_entry_25,KERN_GATE | TRAP_GATE);
    desc_table[26].set((uint64_t)&exception_entry_26,KERN_GATE | TRAP_GATE);
    desc_table[27].set((uint64_t)&exception_entry_27,KERN_GATE | TRAP_GATE);
    desc_table[28].set((uint64_t)&exception_entry_28,KERN_GATE | TRAP_GATE);
    desc_table[29].set((uint64_t)&exception_entry_29,KERN_GATE | TRAP_GATE);
    desc_table[30].set((uint64_t)&exception_entry_30,KERN_GATE | TRAP_GATE);
    desc_table[31].set((uint64_t)&exception_entry_31,KERN_GATE | TRAP_GATE);

    *(uint16_t *)&desc_table_reg[0] = sizeof(desc_table) - 1;
    *(uint64_t *)&desc_table_reg[2] = (uint64_t)desc_table;

    asm volatile(
      "lidt %0"
      :
      : "m"(desc_table_reg)
      : "memory"
    );
    log_t()<<"Initialize the CPU's IDT successfully.\n";

    asm volatile(
      "xorq %%rax,%%rax\n\t"
      "xorq %%rbx,%%rbx\n\t"
      "xorq %%rcx,%%rcx\n\t"
      "xorq %%rdx,%%rdx\n\t"
      "xorq %%rsi,%%rsi\n\t"
      "xorq %%rdi,%%rdi\n\t"
      "xorq %%r8,%%r8\n\t"
      "xorq %%r9,%%r9\n\t"
      "xorq %%r10,%%r10\n\t"
      "xorq %%r11,%%r11\n\t"
      "xorq %%r12,%%r12\n\t"
      "xorq %%r13,%%r13\n\t"
      "xorq %%r14,%%r14\n\t"
      "xorq %%r15,%%r15\n\t"
      "ud2\n\t"
      :
      :
      :
    );

    return 0;
  }
}
