#include <acpi.hpp>
#include <init.hpp>
#include <memory.hpp>
#include <debug.hpp>
#include <boot.hpp>

namespace acpi
{
  typedef acpi_parser_t::table_header_t header_t;

  struct rsdp_t
  {
    uint64_t signature;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint8_t revision;
    uint32_t rsdt;

    uint32_t length;
    uint64_t xsdt;
    uint8_t ext_checksum;
    uint8_t reserved[3];
  } PACKED_STRUCT;

  struct rsdt_t
  {
    header_t header;
    uint32_t entries[0];
  } PACKED_STRUCT;
  struct xsdt_t
  {
    header_t header;
    uint64_t entries[0];
  } PACKED_STRUCT;

  struct info_t
  {
    size_t nr_entries32;
    uint32_t *entries32;
    size_t nr_entries64;
    uint64_t *entries64;

    header_t *get_entry(size_t i)
    {
      if(i < nr_entries32)
        return (header_t *)addr_phys2virt(entries32[i]);
      i -= nr_entries32;
      if(i < nr_entries64)
        return (header_t *)addr_phys2virt(entries64[i]);
      return nullptr;
    }
  };
  static info_t basic_info;

  static int setup_acpi(bootinfo_t *) INIT_FUNC(kernel,ACPI_ACPI);

  static int setup_acpi(bootinfo_t *bootinfo)
  {
    if(!bootinfo->acpi_rsdp) {
      log_t(log_t::ERROR)<<"Cannot initialize ACPI without its RSDP address.\n";
      return -1;
    }
    auto rsdp = (rsdp_t *)addr_phys2virt(bootinfo->acpi_rsdp);
    if(rsdp->revision >= 2){
      auto xsdt = (xsdt_t *)addr_phys2virt(rsdp->xsdt);
      basic_info.entries64 = xsdt->entries;
      basic_info.nr_entries64 = (xsdt->header.length - sizeof(*xsdt)) / sizeof(xsdt->entries[0]);

      basic_info.nr_entries32 = 0;
      basic_info.entries32 = nullptr;

      log_t()<<"ACPI XSDT has "<<basic_info.nr_entries64
             <<(basic_info.nr_entries64 >= 2 ? " entries.\n" : " entry.\n");
    }else{
      auto rsdt = (rsdt_t *)addr_phys2virt(rsdp->rsdt);
      basic_info.entries32 = rsdt->entries;
      basic_info.nr_entries32 = (rsdt->header.length - sizeof(*rsdt)) / sizeof(rsdt->entries[0]);

      basic_info.nr_entries64 = 0;
      basic_info.entries64 = nullptr;

      log_t()<<"ACPI RSDT has "<<basic_info.nr_entries64
             <<(basic_info.nr_entries64 >= 2 ? " entries.\n" : " entry.\n");
    }

    return 0;
  }
}

errno_t acpi_parser_t::run(void)
{
  errno_t ret;
  acpi::header_t *header;
  for(size_t i = 0;(header = acpi::basic_info.get_entry(i));++i)
  {
    if(header->signature != signature)
      continue;
    if((ret = this->parse(header)))
      return ret;
  }
  return 0;
}
