#pragma once
#include <intr.hpp>

namespace irq
{
  enum { GSI_MAX_COUNT = 24 };

  extern bool need_schedule;
  inline void delayed_task_schedule(void)
  {
    need_schedule = true;
  }

  class manager_t
  {
  public:
    manager_t(irq_t gsi = -1)
    {
      irq_idx = gsi;
    }

    irq_t irq_index(void)
    {
      return irq_idx;
    }

    errno_t register_handler(irq_handler_t *handler);
    void unregister_handler(irq_handler_t *handler);
    errno_t handle_interrupt(void);

    errno_t enroll(void);
    //void unenroll(void) = delete;

    //~manager_t(void) = delete;
  protected:
    virtual void send_eoi(void) = 0;
    //virtual void set_masked(bool masked) = 0;
    //virtual void set_triggered_mode(bool level1_edge0) = 0;

    unsigned int native_intr_index(void)
    {
      return intr_idx;
    }
  private:
    //unsigned int flags;
    unsigned int intr_idx;
    irq_t irq_idx;
    irq_handler_t *handler;
  };

  //TODO:
  // class percpu_manager_t;
}
