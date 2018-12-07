#pragma once
#include <intr.hpp>

namespace time
{
  class local_timer_irq_t :public irq_handler_t
  {
  public:
    local_timer_irq_t(irq_t irq)
      :irq_handler_t(irq),ticks(0)
    {}

    uint64_t get_ticks(void)
    {
      return ticks;
    }

    static local_timer_irq_t *get_instance(irq_t irq);
  protected:
    virtual irqret_t handle(void) override;
  private:
    volatile uint64_t ticks;
  };

  class global_timer_irq_t :public irq_handler_t
  {
  public:
    global_timer_irq_t(irq_t irq)
      :irq_handler_t(irq)
    {}

    static global_timer_irq_t *get_instance(irq_t irq);
  protected:
    virtual irqret_t handle(void) override;
  };
}
