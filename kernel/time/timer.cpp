#include <time.hpp>
#include <memory.hpp>
#include "timer.hpp"
#include "../task/task.hpp"

namespace time
{
  static volatile uint64_t ticks;

  irqret_t global_timer_irq_t::handle(void)
  {
    ++ticks;
    return IRQRET_HANDLED;
  }

  irqret_t local_timer_irq_t::handle(void)
  {
    ++this->ticks;
    task::local_timer_tick();
    return IRQRET_HANDLED;
  }

  global_timer_irq_t *global_timer_irq_t::get_instance(irq_t irq)
  {
    static uint8_t buffer[sizeof(global_timer_irq_t)];
    return new(buffer) global_timer_irq_t(irq);
  }

  local_timer_irq_t *local_timer_irq_t::get_instance(irq_t irq)
  {
    static uint8_t buffer[sizeof(local_timer_irq_t)];
    return new(buffer) local_timer_irq_t(irq);
  }
}

uint64_t timer_t::global_ticks(void)
{
  return time::ticks;
}
