#pragma once
#include <env.hpp>

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

  acpi_parser_t(uint32_t signature)
    :signature(signature)
  {}

  bool run(void);

  //virtual bool identify(const table_header_t *table);
  virtual bool parse(const table_header_t *table) = 0;
private:
  uint32_t signature;
};
