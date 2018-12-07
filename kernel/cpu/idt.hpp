#pragma once
#include <env.hpp>

namespace cpu
{
  struct exception_stack_t
  {
    uint64_t r15,r14,r13,r12,r11,r10,r9,r8;
    uint64_t rdi,rsi;
    uint64_t rdx,rcx,rbx,rax;
    uint64_t rbp;
    uint64_t no_excep,err_code;
    uint64_t rip,cs;
    uint64_t rflags;
    uint64_t rsp,ss;
  };
}

namespace idt
{
  enum : uint64_t
  {
    TRAP_GATE = 0x00000f0000000000,
    INTR_GATE = 0x00000e0000000000,
  };
  enum : uint64_t
  {
    KERN_GATE = 0x0000000000000000,
    USER_GATE = 0x0000600000000000,
  };

  unsigned int get_free_entry(uint64_t offset,
        uint64_t attributes = INTR_GATE | KERN_GATE);
}
