#pragma once
#include <env.hpp>

namespace gdt
{
  enum
  {
    KERNEL_CS = 0x8, // Kernel Code Selector
    KERNEL_DS = 0x0, // Kernel Data Selector
    /* The registers DS,SS,ES are not used by the CPU when it's in long mode.
     * The registers FS,GS can be used, but they are not used in the OS now.
     * So just set all of them to zero.
     */
  };
}
