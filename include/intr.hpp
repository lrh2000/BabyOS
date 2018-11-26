#pragma once
#include <env.hpp>
#include <errno.hpp>

struct intr_stack_t
{
  uint64_t r15,r14,r13,r12,r11,r10,r9,r8;
  uint64_t rdi,rsi;
  uint64_t rdx,rcx,rbx,rax;
  uint64_t rbp;
  uint64_t no_intr,err_code;
  uint64_t rip,cs;
  uint64_t rflags;
  uint64_t rsp,ss;
};

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

inline void set_intr_flag(void)
{
  asm volatile("sti");
}
inline void clear_intr_flag(void)
{
  asm volatile("cli");
}
