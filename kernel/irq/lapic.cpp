#include <env.hpp>
#include <init.hpp>
#include <debug.hpp>
#include <memory.hpp>
#include <intr.hpp>
#include "irq.hpp"
#include "../cpu/idt.hpp"

namespace local_apic
{
  enum
  {
    REG_EOI                = 0x0b0,
    REG_SPURIOUS_INTR      = 0x0e0,
    REG_TIMER_INTR         = 0x320,
    REG_TIMER_INITIAL_CNT  = 0x380,
    REG_TIMER_CURRENT_CNT  = 0x390,
  };

  enum : uint32_t
  {
    SOFTWARE_ENABLE = 0x00000100,
    ONESHOT_TIMER   = 0x00000000,
    PERIODIC_TIMER  = 0x00020000,
  };

  static uintptr_t mmio_address;

  static inline uint32_t readl(unsigned int reg)
  {
    return *(volatile uint32_t *)(mmio_address + reg);
  }

  static inline void writel(uint32_t data,uint32_t reg)
  {
    *(volatile uint32_t *)(mmio_address + reg) = data;
  }

  static inline uintptr_t get_apicbase(void)
  {
    uint64_t msr_apicbase;

    asm volatile(
      "movl $0x1b,%%ecx\n\t"
      "rdmsr\n\t"
      "shlq $32,%%rdx\n\t"
      "addl %%eax,%%edx\n\t"
      : "=d"(msr_apicbase)
      :
      : "rcx","rax"
    );
    if(msr_apicbase & 0x800)
      return addr_phys2virt(msr_apicbase & ~0xfffull);

    return 0;
  }

  class local_timer_irq_t :public irq_handler_t
  {
    volatile bool flag;
  public:
    local_timer_irq_t(void)
      :irq_handler_t(irq::LOCAL_TIMER),flag(false)
    {}

    virtual bool handle(void) override
    {
      flag = true;
      return true;
    }

    bool get_flag(void)
    {
      return flag ? (flag = false),true : false;
    }
  };

  static int setup_lapic(void) INIT_FUNC(kernel,IRQ_LAPIC);

  static int setup_lapic(void)
  {
    mmio_address = get_apicbase();
    if(!mmio_address) {
      log_t(log_t::ERROR)<<"Failed to initialize the local APIC,which has been disabled.\n";
      return -1;
    }

    writel(SOFTWARE_ENABLE,REG_SPURIOUS_INTR);

    auto no_intr = idt::get_free_entry((uint64_t)irq_entry_table[irq::LOCAL_TIMER]);
    writel(PERIODIC_TIMER | no_intr,REG_TIMER_INTR);

    static uint8_t buffer[sizeof(local_timer_irq_t)];
    local_timer_irq_t *timer = new(buffer) local_timer_irq_t;
    timer->enroll();

    set_intr_flag();
    writel(1000000000,REG_TIMER_INITIAL_CNT);

    for(;;)
    {
      while(!timer->get_flag());
      log_t()<<"Receive the interrupt of the local APIC timer.\n";
    }

    return 0;
  }
}

namespace irq
{
  void send_eoi(void)
  {
    local_apic::writel(0,local_apic::REG_EOI);
  }
}