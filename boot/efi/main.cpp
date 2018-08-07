#include <init.hpp>
#include "efi.hpp"

namespace efi
{
  const guid_t graphics_output_guid =
      {0x9042a9de,0x23dc,0x4a38,{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}};
}

namespace efi
{
  static handle_t image_handle;
  static system_table_t *system_table;
  static boot_services_t *boot_services;

  static inline void print(const uint16_t *s)
  {
    (*system_table->con_out->output_string)(system_table->con_out,s);
  }

  static void print(const char *s)
  {
    int len = 0;
    while(*s++)
      ++len;
    s -= len + 1;

    uint16_t ss[len + 1];
    len = 0;
    while((ss[len++] = *s++));

    print(ss);
  }

  static void print(uint64_t num,int base = 10,int figure = 0)
  {
    uint16_t s[21];
    int i = 20;

    s[i--] = 0;
    do
      ((s[i--] = num % base + '0') <= '9') ? 0 : (s[i + 1] += 'A' - '0' - 10);
    while((num /= base) | (figure && --figure));

    print(s + i + 1);
  }

  static void print_graphics_info(void)
  {
    graphics_output_proto_t *graphics;
    status_t status;

    status = (*boot_services->open_protocol)(system_table->con_out_handle,&graphics_output_guid,
        (void **)&graphics,image_handle,nullptr,OPEN_PROTO_BY_HANDLE_PROTOCOL);
    if(status_error(status)) {
      if(status == ERR_UNSUPPORTED)
        print("[INFO ] It's in text mode,so there's no graphics information.\n\r");
      else
        print("[ERROR] Failed to call open_protocol.\n\r");
      return;
    }

    graphics_mode_info_t *info = graphics->mode->info;
    print("[INFO ] It's in graphics mode,here's some graphics information.\n\r");
    print("[INFO ] Horizontal Resolution: ");
    print(info->width);
    print("px Vertical Resolution: ");
    print(info->height);
    print("px\n\r");
    print("[INFO ] Pixels Per Scan Line: ");
    print(info->scanline_pixels);
    print("\n\r");
    print("[INFO ] Pixel Format: ");
    switch(info->pixel_format)
    {
    case PIXEL_RGB:
      print("RGB");
      break;
    case PIXEL_BGR:
      print("BGR");
      break;
    case PIXEL_BITMASK:
      print("Bit Mask");
      break;
    case PIXEL_BLT_ONLY:
      print("BLT Only");
      break;
    default:
      print("Unknow");
      break;
    }
    print("\n\r");

    status = (*boot_services->close_protocol)(system_table->con_out_handle,
        &graphics_output_guid,image_handle,nullptr);
    if(status_error(status))
      print("[WRAN ] Failed to call close_protocol.\n\r");
  }

  static void print_memory_map(void)
  {
    memory_desc_t *memmap;
    size_t memmap_size,desc_size,mapkey,size;
    uint32_t version;
    status_t status;

    memmap_size = 0;
    desc_size = 0;
    status = (*boot_services->get_memory_map)(&memmap_size,nullptr,&mapkey,&desc_size,&version);
    if(status != ERR_BUFFER_TOO_SMALL) {
      print("[ERROR] Failed to call get_memory_map.\n\r");
      return;
    }

    memmap_size += (desc_size ? : sizeof(memory_desc_t)) << 1;

    status = (*boot_services->allocate_pool)(LOADER_DATA,memmap_size,(void **)&memmap);
    if(status_error(status)) {
      print("[ERROR] Failed to call allocate_pool.\n\r");
      return;
    }

    status = (*boot_services->get_memory_map)(&memmap_size,memmap,&mapkey,&desc_size,&version);
    if(status_error(status)) {
      print("[ERROR] Failed to call get_memmory_map.\n\r");
      goto end;
    }

    size = memmap_size / desc_size;

    print("[INFO ] Here is the memory map which contains ");
    print(size);
    print(" entries.\n\r");
    while(size--)
    {
      print("[INFO ] 0x");
      print(memmap->physics_addr,16,16);
      print("-0x");
      print(memmap->physics_addr + (memmap->nr_pages << 12) - 1,16,16);
      print("\n\r[INFO ] Attributes:0x");
      print(memmap->attributes,16,16);
      print(" Type:");
      switch(memmap->type)
      {
      case LOADER_CODE:
        print("EFI Loader Code");
        break;
      case LOADER_DATA:
        print("EFI Loader Data");
        break;
      case BOOT_SERVICES_CODE:
        print("EFI Boot Services Code");
        break;
      case BOOT_SERVICES_DATA:
        print("EFI Boot Services Data");
        break;
      case RUNTIME_SERVICES_CODE:
        print("EFI Runtime Services Code");
        break;
      case RUNTIME_SERVICES_DATA:
        print("EFI Runtime Services Data");
        break;
      case CONVENTIONAL_MEMORY:
        print("Conventional Memory");
        break;
      case ACPI_RECLAIM_MEMORY:
        print("ACPI Reclaim Memory");
        break;
      case ACPI_MEMORY_NVS:
        print("ACPI Memory NVS");
        break;
      case MEMORY_MAPED_IO:
        print("Memory Maped IO");
        break;
      case MEMORY_MAPED_IO_PORT_SPACE:
        print("Memory Mapped IO Port Space");
        break;
      case PAL_CODE:
        print("PAL Code");
        break;
      case PERSISTENT_MEMORY:
        print("Persistent Memory");
        break;
      default:
        print("Unknown");
        break;
      }
      print("\n\r");

      memmap = (memory_desc_t *)((uint8_t *)memmap + desc_size);

      //for(int i = 0;i <= 1000000000;++i)
      //  asm volatile("nop");
    }
    memmap = (memory_desc_t *)((uint8_t *)memmap - memmap_size);

  end:
    status = (*boot_services->free_pool)(memmap);
    if(status_error(status))
      print("[WARN ] Failed to call free_map.\n\r");
  }

  static int main(handle_t,system_table_t *) INIT_FUNC(boot,EFI);

  static int main(handle_t img,system_table_t *systab)
  {
    image_handle = img;
    system_table = systab;
    boot_services = systab->boot_services;

    print("[INFO ] Hello,world.\n\r");

    print_graphics_info();

    print_memory_map();

    return 0;
  }
}
