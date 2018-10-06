#include <time.hpp>
#include "timer.hpp"

namespace time
{
  static volatile uint64_t ticks;

  void global_timer_irq(void)
  {
    ++ticks;
  }

  void local_timer_irq(void)
  {
    /* Nothing to do here. */
  }
}

uint64_t timer_t::global_ticks(void)
{
  return time::ticks;
}
