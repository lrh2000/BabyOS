#include <env.hpp>

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

class irq_handler_t
{
public:
  irq_handler_t(irq_t irq) :irq(irq),registered(false) {}
  virtual ~irq_handler_t(void);

  bool enroll(void);
  void unenroll(void);

  virtual bool handle(void) = 0;

  //static irq_t isa_legacy_irq(irq_t irq);
private:
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
