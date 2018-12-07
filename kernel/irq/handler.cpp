#include <intr.hpp>
#include <task.hpp>
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
  static unsigned int cnt_nests;
  bool need_schedule;

  static manager_t **get_manager(irq_t irq)
  {
    size_t cnt;
    manager_t **managers;
    if(irq >= 0)
      cnt = GSI_MAX_COUNT,managers = gsi_managers;
    else
      irq = -(irq + 1),cnt = osi_real_count,managers = osi_managers;
    if((size_t)irq >= cnt)
      return nullptr;
    return managers + irq;
  }

  errno_t manager_t::enroll(void)
  {
    if(irq_idx < 0) {
      if(osi_real_count >= OSI_MAX_COUNT)
        return -ENOSPC;
      osi_managers[osi_real_count] = this;
      intr_idx = idt::get_free_entry((uintptr_t)irq_entry_table[GSI_MAX_COUNT + osi_real_count]);
      ++osi_real_count;
      irq_idx = -osi_real_count;
      return 0;
    }
    manager_t **manager = get_manager(irq_index());
    if(!manager)
      return -EINVAL;
    if(*manager)
      return -EBUSY;
    *manager = this;
    intr_idx = idt::get_free_entry((uintptr_t)irq_entry_table[irq_index()]);
    return 0;
  }

  errno_t manager_t::register_handler(irq_handler_t *arg)
  {
    if(handler)
      return -EBUSY;
    handler = arg;
    return 0;
  }

  void manager_t::unregister_handler(irq_handler_t *arg)
  {
    if(handler == arg)
      handler = nullptr;
  }

  errno_t manager_t::handle_interrupt(void)
  {
    irqret_t ret = IRQRET_UNHANDLED;
    if(handler)
      ret = handler->handle();
    send_eoi();
    return ret < 0 ? ret : 0;
  }
}

extern "C" void do_interrupt(uint64_t no_irq)
{
  ++irq::cnt_nests;
  if(no_irq < irq::GSI_MAX_COUNT)
    irq::gsi_managers[no_irq]->handle_interrupt();
  else
    irq::osi_managers[no_irq - irq::GSI_MAX_COUNT]->handle_interrupt();
  if(--irq::cnt_nests == 0 && irq::need_schedule)
    task_t::schedule(),irq::need_schedule = false;
  return;
}

irq_handler_t::~irq_handler_t(void)
{
  if(registered)
    unenroll();
}

errno_t irq_handler_t::enroll(void)
{
  irq::manager_t **manager = irq::get_manager(this->irq);
  if(!manager || !*manager)
    return -EINVAL;
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
