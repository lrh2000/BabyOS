#include <env.hpp>
#include <init.hpp>
#include <debug.hpp>
#include <memory.hpp>
#include <intr.hpp>
#include <time.hpp>
#include "irq.hpp"
#include "../time/timer.hpp"

namespace local_apic
{
  enum
  {
    REG_EOI                = 0x0b0,
    REG_LOGICAL_DEST       = 0x0d0,
    REG_DEST_FORMAT        = 0x0e0,
    REG_SPURIOUS_INTR      = 0x0f0,
    REG_TIMER_INTR         = 0x320,
    REG_TIMER_INITIAL_CNT  = 0x380,
    REG_TIMER_CURRENT_CNT  = 0x390,
  };

  enum : uint32_t
  {
    SOFTWARE_ENABLE = 0x00000100,

    TIMER_PERIODIC  = 0x00020000,
    // Default option: TIMER_ONESHOT

    INTR_MASKED     = 0x00010000,

    DFR_FLAT_MODEL    = 0xffffffff,
    DFR_CLUSTER_MODEL = 0x0fffffff,
  };
  static inline constexpr uint32_t LDR_LOGICAL_ID(uint8_t lapic)
  {
    return (uint32_t)lapic << 24;
  }

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

  void send_eoi(void)
  {
    local_apic::writel(0,local_apic::REG_EOI);
  }

  class local_irq_t :public irq::manager_t
  {
  public:
    local_irq_t(unsigned int intr_reg) :intr_reg(intr_reg) {}

    virtual void send_eoi() override
    {
      ::local_apic::send_eoi();
    }

    errno_t enroll(void);
  private:
    unsigned int intr_reg;
  };

  errno_t local_irq_t::enroll(void)
  {
    if(errno_t ret = irq::manager_t::enroll())
      return ret;
    writel(INTR_MASKED | native_intr_index(),intr_reg);
    return 0;
  }

  class local_timer_irq_t :public irq_handler_t
  {
    volatile uint64_t ticks;
  public:
    local_timer_irq_t(irq_t irq)
      :irq_handler_t(irq),ticks(0)
    {}

    virtual irqret_t handle(void) override
    {
      ++ticks;
      time::local_timer_irq();
      return IRQRET_HANDLED;
    }

    uint64_t get_ticks(void)
    {
      return ticks;
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
    log_t()<<"Detected this CPU's local APIC,MMIO address:0x"
           <<&log_t::hex64<<mmio_address<<".\n";

    writel(SOFTWARE_ENABLE,REG_SPURIOUS_INTR);
    writel(DFR_FLAT_MODEL,REG_DEST_FORMAT);
    writel(LDR_LOGICAL_ID(0x1),REG_LOGICAL_DEST);

    return 0;
  }

  static int setup_lapic_timer(void) INIT_FUNC(kernel,TIME_LTIMER);

  static int setup_lapic_timer(void)
  {
    static uint8_t timer_irq[sizeof(local_irq_t)];
    local_irq_t *manager = new(timer_irq) local_irq_t(REG_TIMER_INTR);
    manager->enroll();

    uint32_t reg_val = readl(REG_TIMER_INTR);
    reg_val &= ~INTR_MASKED;
    reg_val &= ~TIMER_PERIODIC;
    writel(reg_val,REG_TIMER_INTR);

    set_intr_flag();
    uint64_t ticks = timer_t::global_ticks() + 10;
    while(timer_t::global_ticks() < ticks);
    writel(0xffffffff,REG_TIMER_INITIAL_CNT);
    while(timer_t::global_ticks() < ticks + timer_t::HZ);
    ticks = readl(REG_TIMER_CURRENT_CNT);
    clear_intr_flag();

    static uint8_t buffer[sizeof(local_timer_irq_t)];
    local_timer_irq_t *timer = new(buffer) local_timer_irq_t(manager->irq_index());
    timer->enroll();

    ticks = 0xffffffff - ticks;
    ticks /= timer_t::HZ;
    writel(reg_val | TIMER_PERIODIC,REG_TIMER_INTR);
    writel(ticks,REG_TIMER_INITIAL_CNT);

    set_intr_flag();
    ticks = timer->get_ticks();
    while(timer->get_ticks() < ticks + 10);
    clear_intr_flag();
    log_t()<<"Initialize the local APIC timer successfully.\n";

    set_intr_flag();
    ticks = timer_t::global_ticks();
    for(;;)
    {
      ticks += timer_t::HZ;
      while(timer_t::global_ticks() < ticks);
      auto local_ticks = timer->get_ticks();

      log_t()<<"Global Ticks:"<<ticks<<" Local Ticks:"<<local_ticks<<"\n";
      log_t()<<"The difference between them is "<<(ticks - local_ticks)<<".\n";
    }

    return 0;
  }
}
