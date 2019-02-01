#include <env.hpp>
#include <asm.hpp>
#include <intr.hpp>
#include <acpi.hpp>
#include <debug.hpp>
#include <init.hpp>
#include <time.hpp>
#include "timer.hpp"

namespace hpet
{
  static uintptr_t mmio_address;

  class hpet_parser_t :public acpi_parser_t
  {
    struct hpet_table_t
    {
      table_header_t header;
      uint32_t hardware_id;
      address_t base_addr;
      uint8_t hpet_num;
      uint16_t min_ticks;
      uint8_t page_protection;
    } PACKED_STRUCT;
  public:
    hpet_parser_t(void)
      :acpi_parser_t(0x54455048 /* HPET */)
    {}

    errno_t parse(const table_header_t *header) override
    {
      auto table = (const hpet_table_t *)header;

      if(table->hpet_num) {
        log_t()<<"Dectected HPET"<<table->hpet_num<<"."
          "Mutiple HPETs are not supported,so ignore it.\n";
        return 0;
      }
      log_t()<<"Dectected HPET0,MMIO Address:0x"<<&log_t::hex64<<
        (mmio_address = table->base_addr.address)<<".\n";

      return 0;
    }
  };

  enum : unsigned int
  {
    REG_CAP_ID      = 0x000,
    REG_CONFIG      = 0x010,
    REG_INTR_STATUS = 0x020,
    REG_COUNTER     = 0x0f0,
  };
  static constexpr inline unsigned int REG_TIMER_CONFIG(unsigned int idx)
  {
    return 0x100 + 0x20 * idx;
  }
  static constexpr inline unsigned int REG_TIMER_CMP_VALUE(unsigned int idx)
  {
    return 0x108 + 0x20 * idx;
  }

  enum : uint64_t
  {
    CNF_ENABLE      = 0x0000000000000001,
    CNF_LEGACY_IRQ  = 0x0000000000000002,

    TIMER_LEVEL_IRQ = 0x0000000000000002,
    TIMER_ENABLE    = 0x0000000000000004,
    TIMER_PERIODIC  = 0x0000000000000008,
    TIMER_SET_VALUE = 0x0000000000000040,
  };
  static inline uint32_t CAP_TICKS_PER_FS(uint64_t data)
  {
    return data >> 32;
  }

  static inline uint32_t readl(unsigned int reg)
  {
    return *(volatile uint32_t *)(mmio_address + reg);
  }
  static inline void writel(uint32_t data,unsigned int reg)
  {
    *(volatile uint32_t *)(mmio_address + reg) = data;
  }
  static inline uint64_t readq(unsigned int reg)
  {
    return *(volatile uint64_t *)(mmio_address + reg);
  }
  static inline void writeq(uint64_t data,unsigned int reg)
  {
    *(volatile uint64_t *)(mmio_address + reg) = data;
  }

  static int setup_hpet(void) INIT_FUNC(kernel,TIME_GTIMER);

  static int setup_hpet(void)
  {
    hpet_parser_t parser;
    parser.run();

    auto irq = time::global_timer_irq_t::get_instance(2);
    irq->enroll();

    uint64_t counter = 1000000000000000ull;
    counter /= CAP_TICKS_PER_FS(readq(REG_CAP_ID));
    counter /= timer_t::HZ;
    writeq(CNF_LEGACY_IRQ,REG_CONFIG);
    writeq(0,REG_COUNTER);
    writeq(TIMER_ENABLE | TIMER_PERIODIC | TIMER_SET_VALUE,REG_TIMER_CONFIG(0));
    writeq(counter,REG_TIMER_CMP_VALUE(0));
    writeq(CNF_LEGACY_IRQ | CNF_ENABLE,REG_CONFIG);

    set_intr_flag();
    auto ticks = timer_t::global_ticks();
    while(timer_t::global_ticks() < ticks + 10);
    clear_intr_flag();
    log_t()<<"Initialize the High Precision Event Timer successfully.\n";

    return 0;
  }
}
