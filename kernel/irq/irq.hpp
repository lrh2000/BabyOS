#pragma once

namespace irq
{
  enum { TOTAL = 25 };

  enum { IOAPIC_START = 0  };
  enum { IOAPIC_END   = 23 };
  enum { LOCAL_TIMER  = 24 };

  void send_eoi(void);
}

extern "C" void *irq_entry_table[irq::TOTAL];
