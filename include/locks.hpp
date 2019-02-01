#pragma once
#include <env.hpp>
#include <asm.hpp>

class task_t;

class spinlock_t
{
public:
  spinlock_t(void) {}

  void lock(void)
  {
    ++in_spinlock;
  }

  void unlock(void)
  {
    --in_spinlock;
  }

private:
  static volatile size_t in_spinlock;
  friend class task_private_t;
};
// NOTE: No support for multiple CPUs, so just deal with preempting now.

class spinlock_noirq_t :public spinlock_t
{
public:
  spinlock_noirq_t(void) {}

  void lock_only(void)
  {
    spinlock_t::lock();
  }

  void unlock_only(void)
  {
    spinlock_t::unlock();
  }

  void lock(void)
  {
    clear_intr_flag();
    spinlock_t::lock();
  }

  void lock(irqstat_t &irqf)
  {
    irqf = save_clear_intr_flag();
    spinlock_t::lock();
  }

  void unlock(void)
  {
    spinlock_t::unlock();
    set_intr_flag();
  }

  void unlock(irqstat_t irqf)
  {
    spinlock_t::unlock();
    restore_intr_flag(irqf);
  }
};

class mutex_t
{
public:
  void lock(void);
  void unlock(void);

private:
  struct wait_tasks_t
  {
    task_t *task;
    wait_tasks_t *next;
  };

  spinlock_t spinlock;
  wait_tasks_t *volatile wait_tasks = nullptr;
  volatile bool locked = false;
};
