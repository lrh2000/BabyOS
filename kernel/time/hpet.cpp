#include <env.hpp>
#include <intr.hpp>
#include <acpi.hpp>
#include <debug.hpp>
#include <init.hpp>
#include <memory.hpp>

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

    bool parse(const table_header_t *header) override
    {
      auto table = (const hpet_table_t *)header;

      if(table->hpet_num) {
        log_t()<<"Dectected HPET"<<table->hpet_num<<"."
          "Mutiple HPETs are not supported,so ignore it.\n";
        return true;
      }
      log_t()<<"Dectected HPET0,MMIO Address:0x"<<&log_t::hex64<<
        (mmio_address = table->base_addr.address)<<".\n";

      return true;
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

  class hpet_irq_t :public irq_handler_t
  {
    volatile bool flag;
  public:
    hpet_irq_t(void)
      :irq_handler_t(2),flag(false)
    {}

    bool handle(void) override
    {
      flag = true;
      return true;
    }

    bool get_flag(void)
    {
      if(flag) {
        flag = false;
        return true;
      }
      return false;
    }
  };

  static int setup_hpet(void) INIT_FUNC(kernel,TIME_HPET);

  static int setup_hpet(void)
  {
    hpet_parser_t parser;
    parser.run();

    static uint8_t buffer[sizeof(hpet_irq_t)];
    auto irq = new(buffer) hpet_irq_t;
    irq->enroll();

    writeq(CNF_LEGACY_IRQ,REG_CONFIG);
    writeq(0,REG_COUNTER);
    writeq(TIMER_ENABLE | TIMER_PERIODIC | TIMER_SET_VALUE,REG_TIMER_CONFIG(0));
    writeq(1000000000000000ull / CAP_TICKS_PER_FS(readq(REG_CAP_ID)),REG_TIMER_CMP_VALUE(0));
    writeq(CNF_LEGACY_IRQ | CNF_ENABLE,REG_CONFIG);

    set_intr_flag();

    for(;;)
    {
      while(!irq->get_flag());
      log_t()<<"Receive an IRQ from the HPET.\n";
    }

    return 0;
  }
}
