#include <init.hpp>
#include <boot.hpp>
#include "efi.hpp"

extern void *_head;

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

  static status_t fill_graphics_info(bootinfo_t *bootinfo)
  {
    graphics_output_proto_t *graphics;
    status_t status;
    bool flag = true;

    status = (*boot_services->open_protocol)(system_table->con_out_handle,&graphics_output_guid,
        (void **)&graphics,image_handle,nullptr,OPEN_PROTO_BY_HANDLE_PROTOCOL);
    if(status_error(status)) {
      if(status == ERR_UNSUPPORTED)
        print("[ERROR] It's in text mode,which has not been supported yet.\n\r");
      else
        print("[ERROR] Failed to call open_protocol.\n\r");
      return status;
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
      flag = false;
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

    bootinfo->video.width = info->width;
    bootinfo->video.height = info->height;
    bootinfo->video.vram_width = info->scanline_pixels;
    bootinfo->video.vram_address = graphics->mode->framebuffer_addr;

    status = (*boot_services->close_protocol)(system_table->con_out_handle,
        &graphics_output_guid,image_handle,nullptr);
    if(status_error(status))
      print("[WRAN ] Failed to call close_protocol.\n\r");

    if(flag) {
      print("[ERROR] This pixel format has not been suported yet.\n\r");
      return ERR_UNSUPPORTED;
    }

    return SUCCESS;
  }

  static status_t get_memory_map(memory_desc_t *&memmap,size_t &mapkey,
                                  size_t &desc_size,size_t &memmap_size)
  {
    status_t status;
    uint32_t version;

    memmap_size = 0;
    desc_size = 0;
    memmap = nullptr;
    status = (*boot_services->get_memory_map)(&memmap_size,nullptr,&mapkey,&desc_size,&version);
    if(status != ERR_BUFFER_TOO_SMALL)
      return status;

    memmap_size += (desc_size ? : sizeof(memory_desc_t)) << 1;

    status = (*boot_services->allocate_pool)(LOADER_DATA,memmap_size,(void **)&memmap);
    if(status_error(status))
      return status;

    status = (*boot_services->get_memory_map)(&memmap_size,memmap,&mapkey,&desc_size,&version);
    if(!status_error(status))
      return status;

    (*boot_services->free_pool)(memmap);
    memmap = nullptr;
    return status;
  }

  static void print_memory_map(void)
  {
    memory_desc_t *memmap;
    size_t memmap_size,desc_size,mapkey,size;
    status_t status;

    status = get_memory_map(memmap,mapkey,desc_size,memmap_size);
    if(status_error(status)) {
      print("[ERROR] Failed to call get_memory_map.\n\r");
      return;
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
      print(" ");
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
    }
    memmap = (memory_desc_t *)((uint8_t *)memmap - memmap_size);

    status = (*boot_services->free_pool)(memmap);
    if(status_error(status))
      print("[WARN ] Failed to call free_map.\n\r");
  }

  static status_t fill_memory_info(bootinfo_t *bootinfo,memory_desc_t *memmap,
                                            size_t len,size_t desc_size)
  {
    auto &meminfo = bootinfo->memory;
    meminfo.nr_regions = 0;
    meminfo.usable_mem = (boot::memory_region_t *)bootinfo->tail;

    size_t rest = (uint8_t *)bootinfo->tail - (uint8_t *)bootinfo;
    rest = bootinfo_t::MAX_SIZE - rest;
    rest /= sizeof(meminfo.usable_mem[0]);

    size_t i;
    while(len--)
    {
      switch(memmap->type)
      {
      case CONVENTIONAL_MEMORY:
      case BOOT_SERVICES_CODE:
      case BOOT_SERVICES_DATA:
      case LOADER_DATA:
        break;
      default:
        goto next;
      }

      for(i = meminfo.nr_regions - 1;~i;--i)
      {
        if(meminfo.usable_mem[i].end_address + 1 < memmap->physics_addr)
          break;
        if(meminfo.usable_mem[i].end_address + 1 > memmap->physics_addr)
          continue;
        meminfo.usable_mem[i].end_address += memmap->nr_pages << 12;
        goto next;
      }

      if(!rest) {
        print("[ERROR] Out of memory for the boot information.\n\r");
        return ERR_OUT_OF_RESOURCES;
      }

      for(i = meminfo.nr_regions - 1;~i;--i)
      {
        if(meminfo.usable_mem[i].end_address + 1 < memmap->physics_addr)
          break;
        meminfo.usable_mem[i + 1] = meminfo.usable_mem[i];
      }
      --rest;
      ++i;
      ++meminfo.nr_regions;

      meminfo.usable_mem[i].start_address = memmap->physics_addr;
      meminfo.usable_mem[i].end_address = memmap->physics_addr + (memmap->nr_pages << 12) - 1;

next:
      memmap = (memory_desc_t *)((uint8_t *)memmap + desc_size);
    }

    bootinfo->tail = (void *)(meminfo.usable_mem + meminfo.nr_regions);
    return SUCCESS;
  }

  static status_t exit_boottime(bootinfo_t *bootinfo,bool &can_exit)
  {
    memory_desc_t *memmap;
    size_t desc_size,memmap_size,mapkey;
    status_t status;

    print("[INFO ] Ready to terminate all boot services.\n\r");

    status = get_memory_map(memmap,mapkey,desc_size,memmap_size);
    if(status_error(status)) {
      print("[ERROR] Failed to call get_memory_map.\n\r");
      goto err;
    }

    status = fill_memory_info(bootinfo,memmap,memmap_size / desc_size,desc_size);
    if(status_error(status))
      goto err;

    /* UEFI Specification
     * Firmware implementation may choose to do a partial shutdown of the boot services during
     * the first call to ExitBootServices(). A UEFI OS loader should not make calls to any boot
     * service function other than GetMemoryMap() after the first call to ExitBootServices().
     */
    status = (*boot_services->exit_boot_services)(image_handle,mapkey);
    if(status_error(status))
      can_exit = false;

    return status;
err:
    status_t status0 = (*boot_services->free_pool)(memmap);
    if(status_error(status0))
      print("[WARN ] Failed to call free_pool.\n\r");

    return status;
  }

  static int main(bootinfo_t *,handle_t,system_table_t *) INIT_FUNC(boot,EFI);

  static int main(bootinfo_t *bootinfo,handle_t img,system_table_t *systab)
  {
    status_t status;
    bool can_exit = true;

    image_handle = img;
    system_table = systab;
    boot_services = systab->boot_services;

    bootinfo->tail = (void *)(bootinfo + 1);

    status = fill_graphics_info(bootinfo);
    if(status_error(status))
      goto err;

    print_memory_map();

    status = exit_boottime(bootinfo,can_exit);
    if(status_error(status))
      goto err;

    return 0;
err:
    if(can_exit) {
      (*boot_services->exit)(image_handle,status,0,nullptr);
      print("[ERROR] The function named \"exit\" doesn't work.\n\r");
    }

    return 1;
  }
}
