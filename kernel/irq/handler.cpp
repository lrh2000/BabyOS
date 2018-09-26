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
  if(this->irq > ::irq::IOAPIC_END)
    return false;
  if(this->irq < 0 && (-this->irq <= ::irq::IOAPIC_END || -this->irq >= ::irq::TOTAL))
    return false;
  irq_t real_irq = this->irq >= 0 ? irq : -irq;
  if(irq::handlers[real_irq])
    return false;
  irq::handlers[real_irq] = this;
  registered = true;
  return true;
}

void irq_handler_t::unenroll(void)
{
  if(!registered)
    return;
  irq_t real_irq = this->irq >= 0 ? irq : -irq;
  irq::handlers[real_irq] = nullptr;
}

extern "C" void do_interrupt(intr_stack_t *data)
{
  if(irq::handlers[data->no_intr])
    irq::handlers[data->no_intr]->handle();
  irq::send_eoi();

  return;
}
