#include <env.hpp>

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
