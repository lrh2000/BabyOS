#pragma once
#include <env.hpp>

typedef uint64_t irqstat_t;
inline irqstat_t save_clear_intr_flag(void)
{
  irqstat_t ret;
  asm volatile(
      "pushfq\n\t"
      "popq %0\n\t"
      "cli\n\t"
      : "=r"(ret)
    );
  return ret;
}
inline void restore_intr_flag(irqstat_t flag)
{
  asm volatile(
      "pushq %0\n\t"
      "popfq\n\t"
      :
      : "r"(flag)
    );
}
inline void set_intr_flag(void)
{
  asm volatile("sti");
}
inline void clear_intr_flag(void)
{
  asm volatile("cli");
}
