#pragma once
#include <env.hpp>

namespace efi
{
  enum status_t : size_t
  {
    SUCCESS = 0,

#define EFIERR(x) ((1ull << 63) | x)
    ERR_LOAD_ERROR                   = EFIERR(1),
    ERR_INVALID_PARAMETER            = EFIERR(2),
    ERR_UNSUPPORTED                  = EFIERR(3),
    ERR_BAD_BUFFER_SIZE              = EFIERR(4),
    ERR_BUFFER_TOO_SMALL             = EFIERR(5),
    ERR_NOT_READY                    = EFIERR(6),
    ERR_DEVICE_ERROR                 = EFIERR(7),
    ERR_WRITE_PROTECTED              = EFIERR(8),
    ERR_OUT_OF_RESOURCES             = EFIERR(9),
    ERR_VOLUME_CORRUPTED             = EFIERR(10),
    ERR_VOLUME_FULL                  = EFIERR(11),
    ERR_NO_MEDIA                     = EFIERR(12),
    ERR_MEDIA_CHANGED                = EFIERR(13),
    ERR_NOT_FOUND                    = EFIERR(14),
    ERR_ACCESS_DENIED                = EFIERR(15),
    ERR_NO_RESPONSE                  = EFIERR(16),
    ERR_NO_MAPPING                   = EFIERR(17),
    ERR_TIMEOUT                      = EFIERR(18),
    ERR_NOT_STARTED                  = EFIERR(19),
    ERR_ALREADY_STARTED              = EFIERR(20),
    ERR_ABORTED                      = EFIERR(21),
    ERR_ICMP_ERROR                   = EFIERR(22),
    ERR_TFTP_ERROR                   = EFIERR(23),
    ERR_PROTOCOL_ERROR               = EFIERR(24),
    ERR_INCOMPATIBLE_VERSION         = EFIERR(25),
    ERR_SECURITY_VIOLATION           = EFIERR(26),
    ERR_CRC_ERROR                    = EFIERR(27),
    ERR_END_OF_MEDIA                 = EFIERR(28),
    ERR_END_OF_FILE                  = EFIERR(31),
    ERR_INVALID_LANGUAGE             = EFIERR(32),
    ERR_COMPROMISED_DATA             = EFIERR(33),
#undef EFIERR

#define EFIWARN(x) x
    WARN_UNKOWN_GLYPH            = EFIWARN(1),
    WARN_DELETE_FAILURE          = EFIWARN(2),
    WARN_WRITE_FAILURE           = EFIWARN(3),
    WARN_BUFFER_TOO_SMALL        = EFIWARN(4),
#undef EFIWARN
  };

  static inline bool status_error(status_t s)
  {
    return ((int64_t)s) < 0;
  }

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

  struct runtime_services_t;
  struct text_output_mode_t;
  struct text_input_proto_t;
  struct config_table_t;

  struct guid_t
  {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8];
  };

  struct text_output_proto_t
  {
    EFI_FUNC_PTR(reset,text_output_proto_t *self,uint8_t verification);
    EFI_FUNC_PTR(output_string,text_output_proto_t *self,const uint16_t *string);
    EFI_FUNC_PTR(test_string,text_output_proto_t *self,const uint16_t *string);
    EFI_FUNC_PTR(query_mode,text_output_proto_t *self,
           size_t mode_id,size_t *columns,size_t *rows);
    EFI_FUNC_PTR(set_mode,text_output_proto_t *self,size_t mode_id);
    EFI_FUNC_PTR(set_attr,text_output_proto_t *self,size_t attr);
    EFI_FUNC_PTR(clear_screen,text_output_proto_t *self);
    EFI_FUNC_PTR(set_cursor_pos,text_output_proto_t *self,size_t column,size_t row);
    EFI_FUNC_PTR(enable_cursor,text_output_proto_t *self,uint8_t visible);

    text_output_mode_t *mode;
  };

  enum pixel_format_t
  {
    PIXEL_RGB,
    PIXEL_BGR,
    PIXEL_BITMASK,
    PIXEL_BLT_ONLY
  };

  struct pixel_bitmask_t
  {
    uint32_t red;
    uint32_t green;
    uint32_t blue;
    uint32_t reserved;
  };

  struct graphics_mode_info_t
  {
    uint32_t version;
    uint32_t width;
    uint32_t height;
    pixel_format_t pixel_format;
    pixel_bitmask_t pixel_bitmask;
    uint32_t scanline_pixels;
  };

  struct graphics_output_mode_t
  {
    uint32_t nr_modes;
    uint32_t mode_id;
    graphics_mode_info_t *info;
    size_t info_size;
    uint64_t framebuffer_addr;
    size_t framebuffer_size;
  };

  struct graphics_output_proto_t
  {
    EFI_FUNC_PTR(query_mode,graphics_output_proto_t *self,uint32_t mode_id,
          size_t *size,graphics_output_mode_t **modes);
    EFI_FUNC_PTR(set_mode,graphics_output_proto_t *self,uint32_t mode_id);
    EFI_FUNC_PTR(blt,...);

    graphics_output_mode_t *mode;
  };

  enum alloc_type_t
  {
    ALLOC_ANY,
    ALLOC_MAX_ADDRESS,
    ALLOC_AT_ADDRESS,
  };

  enum memory_type_t
  {
    __RESERVED_MEMTYPE,
    LOADER_CODE,
    LOADER_DATA,
    BOOT_SERVICES_CODE,
    BOOT_SERVICES_DATA,
    RUNTIME_SERVICES_CODE,
    RUNTIME_SERVICES_DATA,
    CONVENTIONAL_MEMORY,
    UNUSABLE_MEMORY,
    ACPI_RECLAIM_MEMORY,
    ACPI_MEMORY_NVS,
    MEMORY_MAPED_IO,
    MEMORY_MAPED_IO_PORT_SPACE,
    PAL_CODE,
    PERSISTENT_MEMORY,
  };

  struct memory_desc_t
  {
    uint32_t type;
    uint32_t __align;
    uint64_t physics_addr;
    uint64_t virtual_addr;
    uint64_t nr_pages;
    uint64_t attributes;
  };

  enum
  {
    OPEN_PROTO_BY_HANDLE_PROTOCOL  = 0x01,
    OPEN_PROTO_GET                 = 0x02,
    OPEN_PROTO_TEST                = 0x04,
    OPEN_PROTO_BY_CHILD_CONTROLLER = 0x08,
    OPEN_PROTO_BY_DRIVER           = 0x10,
    OPEN_PROTO_EXCLUSIVE           = 0x20
  };

  struct boot_services_t
  {
    table_header_t header;

    EFI_FUNC_PTR(raise_tpl,...);
    EFI_FUNC_PTR(restore_tpl,...);

    EFI_FUNC_PTR(allocate_pages,alloc_type_t type,memory_type_t memtype,
              size_t nr_pages,uint64_t *memory);
    EFI_FUNC_PTR(free_pages,uint64_t memory,size_t nr_pages);
    EFI_FUNC_PTR(get_memory_map,size_t *memmap_size,memory_desc_t *memmap,
              size_t *map_key,size_t *desc_size,uint32_t *version);
    EFI_FUNC_PTR(allocate_pool,memory_type_t pooltype,size_t size,void **memory);
    EFI_FUNC_PTR(free_pool,void *memory);

    EFI_FUNC_PTR(create_event,...);
    EFI_FUNC_PTR(set_timer,...);
    EFI_FUNC_PTR(wait_for_event,...);
    EFI_FUNC_PTR(singal_event,...);
    EFI_FUNC_PTR(close_event,...);
    EFI_FUNC_PTR(check_event,...);

    EFI_FUNC_PTR(install_protocol_interface,...);
    EFI_FUNC_PTR(reinstall_protocol_interface,...);
    EFI_FUNC_PTR(uninstall_protocol_interface,...);
    EFI_FUNC_PTR(handle_protocol,...);
    void *reserved;
    EFI_FUNC_PTR(register_protocol_notify,...);
    EFI_FUNC_PTR(locate_handle,...);
    EFI_FUNC_PTR(locate_device_path,...);
    EFI_FUNC_PTR(install_configuration_table,...);

    EFI_FUNC_PTR(load_image,...);
    EFI_FUNC_PTR(start_image,...);
    EFI_FUNC_PTR(exit,handle_t image_handle,status_t exit_status,
                    size_t exit_data_size,uint16_t *exit_data);
    EFI_FUNC_PTR(unload_image,...);
    EFI_FUNC_PTR(exit_boot_services,handle_t image_handle,size_t map_key);

    EFI_FUNC_PTR(get_next_monotonic_count,...);
    EFI_FUNC_PTR(stall,...);
    EFI_FUNC_PTR(set_watchdog_timer,...);

    EFI_FUNC_PTR(connet_controller,...);
    EFI_FUNC_PTR(discount_controller,...);

    EFI_FUNC_PTR(open_protocol,handle_t handle,const guid_t *protocol,
        void **interface,handle_t agent,handle_t controller,uint32_t attr);
    EFI_FUNC_PTR(close_protocol,handle_t handle,const guid_t *protocol,
                                   handle_t agent,handle_t controller);
    EFI_FUNC_PTR(open_protocol_information,...);

    EFI_FUNC_PTR(protocols_per_handle,...);
    EFI_FUNC_PTR(locate_handle_buffer,...);
    EFI_FUNC_PTR(locate_protocol,...);
    EFI_FUNC_PTR(install_multiple_protocol_interfaces,...);
    EFI_FUNC_PTR(uninstall_multiple_protocol_interfaces,...);

    EFI_FUNC_PTR(calcuate_crc32,...);

    EFI_FUNC_PTR(copy_memory,...);
    EFI_FUNC_PTR(set_memory,...);
    EFI_FUNC_PTR(create_event_extended,...);
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

    runtime_services_t *runtime_services;
    boot_services_t *boot_services;

    size_t nr_table_entries;
    config_table_t *config_table;
  };

#undef EFI_FUNC_PTR

  extern const guid_t graphics_output_guid;
}
