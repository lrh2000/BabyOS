#pragma once
#include <env.hpp>

namespace init_order
{
  namespace boot
  {
    static constexpr uint16_t EFI = 101;
  }

  namespace kernel
  {
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
