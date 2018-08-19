#pragma once
#include <env.hpp>

namespace init_order
{
  namespace boot
  {
    enum { EFI   = 101 };
    enum { STACK = 102 };
  }

  namespace kernel
  {
    enum { CPU_GDT     = 1000 };
    enum { MEM_PAGING  = 1001 };
    enum { MEM_MMPAGES = 1002 };
    enum { MEM_MMALLOC = 1003 };
  }

  namespace fs
  {
  }

  namespace driver
  {
  }
}

#define INIT_FUNC(module,func) \
  __attribute__((constructor(::init_order::module::func)))
