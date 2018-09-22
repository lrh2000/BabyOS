#include <intr.hpp>
#include "irq.hpp"

namespace irq
{
  static irq_handler_t *handlers[irq::TOTAL];
}

irq_handler_t::~irq_handler_t(void)
{
  if(registered)
    unenroll();
}

bool irq_handler_t::enroll(void)
{
  if(this->irq >= ::irq::TOTAL)
    return false;
  if(irq::handlers[this->irq])
    return false;
  irq::handlers[this->irq] = this;
  registered = true;
  return true;
}

void irq_handler_t::unenroll(void)
{
  if(!registered)
    return;
  irq::handlers[this->irq] = nullptr;
}

extern "C" void do_interrupt(intr_stack_t *data)
{
  if(irq::handlers[data->no_intr])
    irq::handlers[data->no_intr]->handle();
  irq::send_eoi();

  return;
}
