#pragma once
#include <env.hpp>
#include <list.hpp>

class task_t
{
public:
  task_t(void);

  void start(void);
  void wake_up(void);

  static void schedule(void);
  static task_t *current(void);
  static void enable_preempt(void);
  static void disable_preempt(void);
  static void sleep(void);

  static void *operator new(size_t);
protected:
  virtual void run(void) = 0;
private:
  enum state_t
  {
    STATE_RUNNING,
    STATE_READY,
    STATE_WILL_SLEEP,
    STATE_SLEEPING,
    STATE_NOT_STARTED,
  };

  state_t state;

  uint64_t total_ticks;
  uint64_t current_ticks;
  uint64_t stack_ptr;

  list_node_t list_node;

  static task_t *volatile _current;
  static volatile size_t preempt_count;

  friend class task_private_t;
};

inline task_t *task_t::current(void)
{
  return task_t::_current;
}

inline void task_t::enable_preempt(void)
{
  ++preempt_count;
}
inline void task_t::disable_preempt(void)
{
  --preempt_count;
}
