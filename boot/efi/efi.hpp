#pragma once
#include <env.hpp>

namespace efi
{
  enum class status_t : size_t
  {
    SUCCESS = 0,
    LOAD_ERROR = 1, //TODO
  };

#define EFI_FUNC_PTR(name,...) \
  status_t (__attribute__((ms_abi)) *name)(__VA_ARGS__)

  typedef void *handle_t;

  struct table_header_t
  {
    uint64_t signature;
    uint32_t revision;
    uint32_t size;
    uint32_t crc32;
    uint32_t reserved;
  };

  struct boot_services_t;
  struct runtime_services_t;
  struct text_output_mode_t;
  struct text_input_proto_t;
  struct config_table_t;

  struct text_output_proto_t
  {
    EFI_FUNC_PTR(reset,text_output_proto_t *self,uint8_t verification);
    EFI_FUNC_PTR(output_string,text_output_proto_t *self,uint16_t *string);
    EFI_FUNC_PTR(test_string,text_output_proto_t *self,uint16_t *string);
    EFI_FUNC_PTR(query_mode,text_output_proto_t *self,
           size_t mode_id,size_t *columns,size_t *rows);
    EFI_FUNC_PTR(set_mode,text_output_proto_t *self,size_t mode_id);
    EFI_FUNC_PTR(set_attr,text_output_proto_t *self,size_t attr);
    EFI_FUNC_PTR(clear_screen,text_output_proto_t *self);
    EFI_FUNC_PTR(set_cursor_pos,text_output_proto_t *self,size_t column,size_t row);
    EFI_FUNC_PTR(enable_cursor,text_output_proto_t *self,uint8_t visible);

    text_output_mode_t *mode;
  };

  struct system_table_t
  {
    table_header_t header;

    uint16_t *firmware_vendor;
    uint32_t firmware_revision;
    uint32_t __align;

    handle_t con_in_handle;
    text_input_proto_t *con_in;
    handle_t con_out_handle;
    text_output_proto_t *con_out;
    handle_t con_err_handle;
    text_output_proto_t *con_err;

    boot_services_t *boot_services;
    runtime_services_t *runtime_services;

    size_t nr_table_entries;
    config_table_t *config_table;
  } PACKED_STRUCT;

#undef EFI_FUNC_PTR
}
