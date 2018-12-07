#pragma once
#include <env.hpp>
#include <list.hpp>

class task_t
{
public:
  task_t(void);

  void start(void);

  static void schedule(void);
  static task_t *current(void);

  static void *operator new(size_t);
protected:
  virtual void run(void) = 0;
private:
  uint64_t total_ticks;
  uint64_t current_ticks;
  uint64_t stack_ptr;

  list_node_t list_node;

  static task_t *_current;

  friend class task_private_t;
};

inline task_t *task_t::current(void)
{
  return task_t::_current;
}
