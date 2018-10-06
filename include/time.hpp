#pragma once
#include <env.hpp>

class timer_t
{
public:
  timer_t(void) = delete; // Unimplemented.

public:
  enum : uint64_t { HZ = 1000 };
  static uint64_t global_ticks(void);
};
