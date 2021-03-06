#pragma once
#include <env.hpp>
#include <errno.hpp>

class acpi_parser_t
{
public:
  struct table_header_t
  {
    uint32_t signature;
    uint32_t length;
    uint8_t revision;
    uint8_t checksum;
    uint8_t oem_id[6];
    uint64_t oem_table_id;
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
  } PACKED_STRUCT;
  struct address_t
  {
    uint8_t type;
    uint8_t bit_width;
    uint8_t bit_offset;
    uint8_t alignment;
    uint64_t address;
  } PACKED_STRUCT;

  acpi_parser_t(uint32_t signature)
    :signature(signature)
  {}

  errno_t run(void);

  // virtual bool identify(const table_header_t *table);
  virtual errno_t parse(const table_header_t *table) = 0;
private:
  uint32_t signature;
};

// TODO:
//  class acpi_aml_value_t;
