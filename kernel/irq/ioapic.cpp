#include <env.hpp>
#include <acpi.hpp>
#include <init.hpp>
#include <debug.hpp>
#include <memory.hpp>
#include "irq.hpp"
#include "../cpu/idt.hpp"

namespace io_apic
{
  static uintptr_t mmio_address;
  static uint8_t nr_irqs;

  class madt_parser_t :public acpi_parser_t
  {
    struct madt_t
    {
      table_header_t header;
      uint32_t intr_ctrl_addr;
      uint32_t flags;
      uint8_t data[0];
    } PACKED_STRUCT;

    struct ics_header_t
    {
      uint8_t type;
      uint8_t length;
    } PACKED_STRUCT;

    enum
    {
      ICS_LAPIC   = 0,
      ICS_IOAPIC  = 1,
    };

    struct ics_lapic_t
    {
      ics_header_t header;
      uint8_t lapic_uid;
      uint8_t lapic_id;
      uint32_t flags;
    };

    struct ics_ioapic_t
    {
      ics_header_t header;
      uint8_t ioapic_id;
      uint8_t reserved;
      uint32_t ioapic_addr;
      uint32_t intr_base;
    } PACKED_STRUCT;

  public:
    madt_parser_t(void)
      :acpi_parser_t(0x43495041 /* 'APIC' */)
    {}

    virtual bool parse(const table_header_t *table) override;
  };

  typedef acpi_parser_t::table_header_t acpi_header_t;

  bool madt_parser_t::parse(const acpi_header_t *header)
  {
    auto madt = (const madt_t *)header;
    auto sz_left = header->length - sizeof(*madt);
    const uint8_t *data = madt->data;

    log_t()<<"Found ACPI MADT:\n";
    while(sz_left)
    {
      auto ics_header = (const ics_header_t *)data;
      sz_left -= ics_header->length;
      data += ics_header->length;

      if(ics_header->type == ICS_LAPIC) {
        auto lapic = (const ics_lapic_t *)ics_header;
        if(lapic->flags & 0x1 /* Enabled. */)
          log_t()<<"    Detected CPU"<<lapic->lapic_uid<<".\n";
      }else if(ics_header->type == ICS_IOAPIC) {
        auto ioapic = (const ics_ioapic_t *)ics_header;
        if(ioapic->intr_base != 0) {
          log_t(log_t::WARNING)<<"    Mutiple I/O APICs are not supported now,"
                                 "only the first I/O APIC will be enabled.\n";
          continue;
        }
        mmio_address = addr_phys2virt(ioapic->ioapic_addr);
        log_t()<<"    Detected the first I/O APIC,MMIO address:0x"
               <<&log_t::hex64<<mmio_address<<".\n";
      } // TODO: Interrupt Source Override
    }
    return true;
  }

  enum
  {
    IOAPICID  = 0x00,
    IOAPICVER = 0x01,
    IOAPICARB = 0x02,
  };
  static inline constexpr unsigned int IOREDTBL(unsigned int idx)
  {
    return 0x10 + (idx << 1);
  }

  enum : uint64_t
  {
    REDTBL_MASKED          = 0x0000000000010000,
    REDTBL_LEVEL_TRIGGER   = 0x0000000000008000,
    REDTBL_EDGE_TRIGGER    = 0x0000000000000000,
    REDTBL_HIGH_ACTIVE     = 0x0000000000000000,
    REDTBL_LOW_ACTIVE      = 0x0000000000002000,
    REDTBL_PHYSICAL_DEST   = 0x0000000000000000,
    REDTBL_LOGICAL_DEST    = 0x0000000000000800,
    REDTBL_FIXED_DELIVERY  = 0x0000000000000000,
    REDTBL_LOWPRI_DELIVERY = 0x0000000000000010,
    REDTBL_SMI_DELIVERY    = 0x0000000000000020,
    REDTBL_NMI_DELIVERY    = 0x0000000000000400,
    REDTBL_INIT_DELIVERY   = 0x0000000000000500,
    REDTBL_EXTINT_DELIVERY = 0x0000000000000700,
  };
  static inline constexpr uint64_t REDTBL_DEST_FIELD(uint8_t dest)
  {
    return (uint64_t)dest << 56;
  }

  static inline uint32_t readl(uint32_t reg)
  {
    *(volatile uint32_t *)(mmio_address + 0x00) = reg; // IOREGSEL
    return *(volatile uint32_t *)(mmio_address + 0x10); // IOWIN
  }

  static inline void writel(uint32_t data,unsigned int reg)
  {
    *(volatile uint32_t *)(mmio_address + 0x00) = reg; // IOREGSEL
    *(volatile uint32_t *)(mmio_address + 0x10) = data; // IOWIN
  }

  static inline uint64_t readq(unsigned int reg)
  {
    return readl(reg + 0) | ((uint64_t)readl(reg + 1) << 32);
  }

  static inline void writeq(uint64_t data,unsigned int reg)
  {
    writel((uint32_t)data,reg + 0);
    writel(data >> 32,reg + 1);
  }

  static int setup_ioapic(void) INIT_FUNC(kernel,IRQ_IOAPIC);

  static int setup_ioapic(void)
  {
    madt_parser_t parser;
    parser.run();

    if(!mmio_address) {
      log_t(log_t::ERROR)<<"No I/O APIC is detected.\n";
      return -1;
    }

    uint32_t ver = readl(IOAPICVER);
    nr_irqs = (ver >> 16) & 0xff;
    ++nr_irqs;

    // irq::IOAPIC_START == Global System Interrupt Base == 0
    if(nr_irqs > irq::IOAPIC_END + 1) {
      log_t(log_t::WARNING)<<"The I/O APIC has "<<nr_irqs<<" IRQs,which are too many."
                      "So only first "<<irq::IOAPIC_END + 1<<" IRQs will be enabled.\n";
      nr_irqs = irq::IOAPIC_END + 1;
    }

    uint64_t attr = REDTBL_HIGH_ACTIVE | REDTBL_FIXED_DELIVERY | REDTBL_EDGE_TRIGGER;
    attr |= REDTBL_LOGICAL_DEST | REDTBL_DEST_FIELD(0xff);
    for(size_t i = 0;i < nr_irqs;++i)
    {
      uint64_t no_intr = idt::get_free_entry((uint64_t)irq_entry_table[i]);
      writeq(attr | no_intr,IOREDTBL(i));
    }
    log_t()<<"I/O APIC version: 0x"<<&log_t::hex<<(ver & 0xff)<<"\n";
    log_t()<<"Initialize the I/O APIC with "<<nr_irqs<<" IRQs successfully.\n";

    return 0;
  }
};
