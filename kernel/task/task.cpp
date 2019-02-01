#include <time.hpp>
#include <memory.hpp>
#include <debug.hpp>
#include <init.hpp>
#include <locks.hpp>
#include <task.hpp>
#include "task.hpp"
#include "../irq/irq.hpp"

struct task_reg_t
{
  uint64_t rbp/*,rsp*/;
  uint64_t r15,r14,r13,r12,r11,r10,r9,r8;
  uint64_t rdi,rsi;
  uint64_t rdx,rcx,rbx,rax;
  uint64_t rip/*,rflags*/;
};

#define STORE_REGISTERS(rip,rsp) \
      "pushq $0\n\t" \
      "pushq %%rax\n\t" \
      "leaq " rip ",%%rax\n\t" \
      "movq %%rax,8(%%rsp)\n\t" \
      "pushq %%rbx\n\t" \
      "pushq %%rcx\n\t" \
      "pushq %%rdx\n\t" \
      "pushq %%rsi\n\t" \
      "pushq %%rdi\n\t" \
      "pushq %%r8\n\t" \
      "pushq %%r9\n\t" \
      "pushq %%r10\n\t" \
      "pushq %%r11\n\t" \
      "pushq %%r12\n\t" \
      "pushq %%r13\n\t" \
      "pushq %%r14\n\t" \
      "pushq %%r15\n\t" \
      "pushq %%rbp\n\t" \
      "movq %%rsp," rsp "\n\t"
  // WARNING: %rax is modified.

#define LOAD_REGISTERS(rsp) \
      "movq " rsp ",%%rsp\n\t" \
      "popq %%rbp\n\t" \
      "popq %%r15\n\t" \
      "popq %%r14\n\t" \
      "popq %%r13\n\t" \
      "popq %%r12\n\t" \
      "popq %%r11\n\t" \
      "popq %%r10\n\t" \
      "popq %%r9\n\t" \
      "popq %%r8\n\t" \
      "popq %%rdi\n\t" \
      "popq %%rsi\n\t" \
      "popq %%rdx\n\t" \
      "popq %%rcx\n\t" \
      "popq %%rbx\n\t" \
      "popq %%rax\n\t" \
      "retq\n\t"

struct task_private_t
{
  static void local_timer_tick(void);
  static void first_schedule(void);
  static void task_entry(task_t *task);

  static list_head_t<task_t,&task_t::list_node> tasks;
};

list_head_t<task_t,&task_t::list_node> task_private_t::tasks;
volatile size_t task_t::preempt_count;
task_t *volatile task_t::_current;
static spinlock_noirq_t task_lock;

void task_private_t::local_timer_tick(void)
{
  task_t *task = task_t::current();
  if(!task)
    return;
  ++task->current_ticks;
  ++task->total_ticks;

  if(task_t::preempt_count || spinlock_t::in_spinlock)
    return;
  if(task->state != task_t::STATE_RUNNING)
    return;

  if(task->total_ticks > timer_t::HZ / 20)
    irq::delayed_task_schedule();
}

void task_private_t::first_schedule(void)
{
  task_lock.lock();
  task_t *task = tasks.first();
  task->state = task_t::STATE_RUNNING;

  task_t::_current = task;
  asm volatile(
      LOAD_REGISTERS("(%0)")
      :
      : "r"(&task->stack_ptr)
    );
  __builtin_unreachable();
}

void task_private_t::task_entry(task_t *task)
{
  task_lock.unlock();

  task->run();
  for(;;);
}

void *task_t::operator new(size_t)
{
  return alloc_pages(3);
}

task_t::task_t(void)
{
  total_ticks = current_ticks = 0;
  stack_ptr = (uintptr_t)this + 3 * PAGE_SIZE;
  state = STATE_NOT_STARTED;
}

void task_t::start(void)
{
  // TODO: assert(state == STATE_NOT_STARTED);

  stack_ptr -= sizeof(task_reg_t);
  task_reg_t *reg = (task_reg_t *)stack_ptr;
  reg->rdi = (uintptr_t)this;
  reg->rip = (uintptr_t)&task_private_t::task_entry;
  state = STATE_SLEEPING;

  wake_up();
}

void task_t::wake_up(void)
{
  irqstat_t irqf;
  task_lock.lock(irqf);

  switch(state)
  {
  case STATE_WILL_SLEEP:
    state = STATE_RUNNING;
    break;
  case STATE_SLEEPING:
    task_private_t::tasks.insert(*this);
    state = STATE_READY;
    break;
  default:
    break;
  }

  task_lock.unlock(irqf);
}

void task_t::sleep(void)
{
  irqstat_t irqf;
  task_lock.lock(irqf);

  current()->state = STATE_WILL_SLEEP;

  task_lock.unlock(irqf);
}

void task_t::schedule(void)
{
  task_t *older,*newer;
  irqstat_t irqf;
  task_lock.lock(irqf);
  older = task_t::current();

  newer = task_private_t::tasks.next(*older);
  if(!newer)
    newer = task_private_t::tasks.first();

  older->current_ticks = 0;
  newer->current_ticks = 1;
  switch(older->state)
  {
  case STATE_RUNNING:
    older->state = STATE_READY;
    break;
  case STATE_WILL_SLEEP:
    older->list_node.remove();
    older->state = STATE_SLEEPING;
    break;
  default:
    // TODO: assert(0);
    break;
  }
  newer->state = STATE_RUNNING;

  _current = newer;
  asm volatile(
      STORE_REGISTERS("0f(%%rip)","(%0)")
      LOAD_REGISTERS("(%1)")
      "0:"
      :
      : "r"(&older->stack_ptr),"r"(&newer->stack_ptr)
      : "memory","rax"
    );

  task_lock.unlock(irqf);
}

namespace task
{
  class happy_task_t :public task_t
  {
  public:
    happy_task_t(bool type)
      :type(type)
    {}
  protected:
    void run(void) override;
  private:
    bool type;
  };

  void happy_task_t::run(void)
  {
    for(;;)
    {
      asm volatile("hlt");
      log_t()<<(type ? "Happy New Year!\n" : "Happy Spring Festival!\n");
    }
  }

  static int setup_task(void) INIT_FUNC(kernel,TASK_TASK);

  static int setup_task(void)
  {
    new(&task_private_t::tasks) typeof(task_private_t::tasks);
    new(&task_lock) typeof(task_lock);

    happy_task_t *happy_task1 = new happy_task_t(true);
    happy_task_t *happy_task2 = new happy_task_t(false);

    happy_task1->start();
    happy_task2->start();

    task_private_t::first_schedule();

    return 0;
  }

  void local_timer_tick(void)
    __attribute__((alias("_ZN14task_private_t16local_timer_tickEv")));
}
