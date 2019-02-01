#include <task.hpp>
#include <locks.hpp>

volatile size_t spinlock_t::in_spinlock;

void mutex_t::lock(void)
{
  spinlock.lock();
  while(locked)
  {
    wait_tasks_t wait;
    wait.task = task_t::current();
    wait.next = wait_tasks;
    wait_tasks = &wait;

    task_t::sleep();
    spinlock.unlock();
    task_t::schedule();
    spinlock.lock();
  }
  locked = true;
  spinlock.unlock();
}

void mutex_t::unlock(void)
{
  spinlock.lock();
  locked = false;
  if(wait_tasks) {
    wait_tasks->task->wake_up();
    wait_tasks = wait_tasks->next;
  }
  spinlock.unlock();
}
