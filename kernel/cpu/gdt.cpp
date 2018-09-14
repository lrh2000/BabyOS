#include <env.hpp>
#include <init.hpp>
#include <debug.hpp>

namespace gdt
{
  static const uint64_t desc_table[] =
    {
      0x0000000000000000, // Null Descriptor
      0x0030980000000000, // Kernel Code Descriptor
    };

  static uint8_t desc_table_reg[10];

  enum
  {
    KERNEL_CS = 0x8, // Kernel Code Selector
    KERNEL_DS = 0x0, // Kernel Data Selector
    /* The registers DS,SS,ES are not used by the CPU when it's in long mode.
     * The registers FS,GS can be used, but they are not used in the OS now.
     * So just set all of them to zero.
     */
  };

  static int setup_gdt(void) INIT_FUNC(kernel,CPU_GDT);

  static int setup_gdt(void)
  {
    *(uint16_t *)&desc_table_reg[0] = sizeof(desc_table) - 1;
    *(uint64_t *)&desc_table_reg[2] = (uint64_t)desc_table;

    asm volatile(
      "movw %2,%%ax\n\t"
      "movw %%ax,%%ds\n\t"
      "movw %%ax,%%es\n\t"
      "movw %%ax,%%ss\n\t"
      "movw %%ax,%%gs\n\t"
      "movw %%ax,%%fs\n\t"
      "pushq %1\n\t"
      "leaq 0f(%%rip),%%rax\n\t"
      "pushq %%rax\n\t"
      "lgdt %0\n\t"
      "lretq\n\t"
      "0:\n\t"
      :
      : "m"(desc_table_reg),"i"(KERNEL_CS),"i"(KERNEL_DS)
      : "rax"
    );

    log_t()<<"Initialize the CPU's GDT successfully.\n";

    return 0;
  }
}
