#pragma once
#include <env.hpp>
#include <errno.hpp>

typedef signed int irq_t;
  // >=0 for Global System Interrupt
  // <0  for OS-defined IRQ (such as the local APIC timer IRQ)

typedef errno_t irqret_t;
enum { IRQRET_HANDLED = 1 };
enum { IRQRET_UNHANDLED = 0 };
// <0 (errno_t) for IRQRET_ERROR

class irq_handler_t
{
public:
  irq_handler_t(irq_t irq) :irq(irq),registered(false) {}
  virtual ~irq_handler_t(void);

  errno_t enroll(void);
  void unenroll(void);

  virtual irqret_t handle(void) = 0;

  //static irq_t isa_legacy_irq(irq_t irq);
private:
  //unsigned int flags;
  irq_t irq;
  bool registered;
};

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
