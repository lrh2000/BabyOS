#include <intr.hpp>
#include "irq.hpp"
#include "../cpu/idt.hpp"

enum { NR_IRQ_ENTRIES = 25 };
extern "C" void *irq_entry_table[NR_IRQ_ENTRIES];

namespace irq
{
  enum { OSI_MAX_COUNT = 1 };

  static manager_t *gsi_managers[GSI_MAX_COUNT];
  static manager_t *osi_managers[OSI_MAX_COUNT];
  static size_t osi_real_count;

  manager_t **get_manager(irq_t irq)
  {
    size_t cnt;
    manager_t **managers;
    if(irq > 0)
      cnt = GSI_MAX_COUNT,managers = gsi_managers;
    else
      irq = -(irq + 1),cnt = osi_real_count,managers = osi_managers;
    if((size_t)irq >= cnt)
      return nullptr;
    return managers + irq;
  }

  bool manager_t::enroll(void)
  {
    if(irq_idx < 0) {
      if(osi_real_count >= OSI_MAX_COUNT)
        return false;
      osi_managers[osi_real_count] = this;
      intr_idx = idt::get_free_entry((uintptr_t)irq_entry_table[GSI_MAX_COUNT + osi_real_count]);
      ++osi_real_count;
      irq_idx = -osi_real_count;
      return true;
    }
    manager_t **manager = get_manager(irq_index());
    if(!manager)
      return false;
    if(*manager)
      return false;
    *manager = this;
    intr_idx = idt::get_free_entry((uintptr_t)irq_entry_table[irq_index()]);
    return true;
  }

  bool manager_t::register_handler(irq_handler_t *arg)
  {
    if(handler)
      return false;
    handler = arg;
    return true;
  }

  void manager_t::unregister_handler(irq_handler_t *arg)
  {
    if(handler == arg)
      handler = nullptr;
  }

  bool manager_t::handle_interrupt(void)
  {
    bool ret = false;
    if(handler)
      ret = handler->handle();
    send_eoi();
    return ret;
  }
}

extern "C" void do_interrupt(intr_stack_t *data)
{
  if(data->no_intr < irq::GSI_MAX_COUNT)
    irq::gsi_managers[data->no_intr]->handle_interrupt();
  else
    irq::osi_managers[data->no_intr - irq::GSI_MAX_COUNT]->handle_interrupt();
  return;
}

irq_handler_t::~irq_handler_t(void)
{
  if(registered)
    unenroll();
}

bool irq_handler_t::enroll(void)
{
  irq::manager_t **manager = irq::get_manager(this->irq);
  if(!manager || !*manager)
    return false;
  return (*manager)->register_handler(this);
}

void irq_handler_t::unenroll(void)
{
  if(!registered)
    return;
  irq::manager_t **manager = irq::get_manager(this->irq);
  if(manager && *manager)
    (*manager)->unregister_handler(this);
}
