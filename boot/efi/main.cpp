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

  static void print(int num)
  {
    uint16_t s[11];
    int i = 10;

    s[i--] = 0;
    do
      s[i--] = num % 10 + '0';
    while(num /= 10);

    print(s + i + 1);
  }

  static int main(handle_t,system_table_t *) INIT_FUNC(boot,EFI);

  static int main(handle_t img,system_table_t *systab)
  {
    image_handle = img;
    system_table = systab;
    boot_services = systab->boot_services;

    print("[INFO ] Hello,world.\n\r");

    graphics_output_proto_t *graphics;
    status_t status;

    status = (*boot_services->open_protocol)(system_table->con_out_handle,&graphics_output_guid,
        (void **)&graphics,image_handle,0,OPEN_PROTO_BY_HANDLE_PROTOCOL);
    if(status_error(status)) {
      if(status == ERR_UNSUPPORTED)
        print("[INFO ] It's in text mode,so there's no graphics information.\n\r");
      else
        print("[ERROR] Failed to call open_protocol.\n\r");
      return 0;
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
    }
    print("\n\r");

    status = (*boot_services->close_protocol)(system_table->con_out_handle,
        &graphics_output_guid,image_handle,0);
    if(status_error(status))
      print("[WRAN ] Failed to call close_protocol.\n\r");

    return 0;
  }
}
