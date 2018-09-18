#include <env.hpp>

struct intr_stack_t
{
  uint64_t r15,r14,r13,r12,r11,r10,r9,r8;
  uint64_t rdi,rsi;
  uint64_t rdx,rcx,rbx,rax;
  uint64_t rbp;
  uint64_t no_intr,err_code;
  uint64_t rip,cs;
  uint64_t rflags;
  uint64_t rsp,ss;
};
