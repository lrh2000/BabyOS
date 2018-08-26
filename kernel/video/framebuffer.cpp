#include <env.hpp>
#include <boot.hpp>
#include <memory.hpp>
#include <init.hpp>
#include <video.hpp>

extern const uint8_t fontdata_8x16[4096];

framebuffer_t::framebuffer_t(physaddr_t fbaddr,uint32_t height,uint32_t width,
                     uint32_t real_width,pixel_format_t px_format)
  :height(height),width(width),real_width(real_width),px_format(px_format)
{
  vram_fb = (pixel_t *)addr_phys2virt(fbaddr);

  size_t fb_sz = 4;
  (fb_sz *= real_width) *= height;
  fb_sz = (fb_sz + PAGE_SIZE - 1) >> PAGE_SHIFT;
  ram_fb = (pixel_t *)alloc_pages(fb_sz);
  // FIXME: Check whether ram_fb is nullptr.

  update_xmin = update_ymin = ~(uint32_t)0;
  update_xmax = update_ymax = 0;
}

framebuffer_t::~framebuffer_t(void)
{
  if(!ram_fb)
    return;
  size_t fb_sz = 4;
  (fb_sz *= real_width) *= height;
  fb_sz = (fb_sz + PAGE_SIZE - 1) >> PAGE_SHIFT;
  dealloc_pages((void *)ram_fb,fb_sz);
}

framebuffer_t::pixel_t framebuffer_t::rgb_to_pixel(uint8_t red,uint8_t green,uint8_t blue)
{
  pixel_t pixel = 0;
  switch(this->px_format)
  {
  case PIXEL32_BGR:
    pixel += red;
    pixel <<= 8;
    pixel += green;
    pixel <<= 8;
    pixel += blue;
    break;
  case PIXEL32_RGB:
    pixel += blue;
    pixel <<= 8;
    pixel += green;
    pixel <<= 8;
    pixel += red;
    break;
  }
  return pixel;
}

void framebuffer_t::fill_rect(uint32_t xmin,uint32_t ymin,
      uint32_t xmax,uint32_t ymax,framebuffer_t::pixel_t pixel)
{
  if(!validate_region(xmin,ymin,xmax,ymax))
    return;
  report_dirty_region(xmin,ymin,xmax,ymax);

  pixel_t *offset = ram_fb;
  offset += (size_t)ymin * real_width + xmin;

  while(ymin++ <= ymax)
  {
    uint32_t x = xmin;
    pixel_t *now = offset;
    while(x++ <= xmax)
      *now++ = pixel;
    offset += real_width;
  }
}

void framebuffer_t::draw_char(char c,uint32_t x,uint32_t y,pixel_t px)
{
  const uint8_t *font = &::fontdata_8x16[(size_t)c << 4];
  pixel_t *buf = ram_fb;
  buf += (size_t)y * real_width + x;

  if(x + 8ull >= width)
    return;
  if(y + 16ull >= height)
    return;

  report_dirty_region(x,y,x + 8,y + 16);
  for(size_t dy = 0;dy < 16;++dy,++font,buf += real_width)
    for(size_t dx = 0;dx < 8;++dx)
      if(*font & (1 << dx))
        buf[8 - dx] = px;
}

void framebuffer_t::draw_string(const char *s,uint32_t x,uint32_t y,pixel_t px)
{
  while(*s)
    draw_char(*s++,x,y,px),x += 8;
}

void framebuffer_t::update(void)
{
  if(!validate_region(update_xmin,update_ymin,update_xmax,update_ymax))
    return;
  pixel_t *offset_ram = ram_fb,*offset_vram = vram_fb;
  size_t offset = (size_t)update_ymin * real_width + update_xmin;
  offset_ram += offset;
  offset_vram += offset;

  while(update_ymin++ <= update_ymax)
  {
    uint32_t x = update_xmin;
    pixel_t *from = offset_ram,*to = offset_vram;
    while(x++ <= update_xmax)
      *to++ = *from++;
    offset_ram += real_width;
    offset_vram += real_width;
  }

  update_xmin = update_ymin = ~(uint32_t)0;
  update_xmax = update_ymax = 0;
}

namespace framebuffer
{
  static int setup_videofb(bootinfo_t *bootinfo) INIT_FUNC(kernel,VIDEO_FB);

  static int setup_videofb(bootinfo_t *bootinfo)
  {
    static uint8_t boot_fb[sizeof(framebuffer_t)];

    auto &video = bootinfo->video;
    framebuffer_t *fb = new(boot_fb) framebuffer_t(video.vram_address,
        video.height,video.width,video.vram_width,video.pixel_format);

    uint32_t string_x = video.width / 2 - 48;
    uint32_t string_y = video.height / 2 - 8;

    uint8_t r = 0,g = 127,b = 255;
    int dr = 1,dg = 1,db = -1;
    for(;;)
    {
      fb->fill_rect(0,0,video.width - 1,video.height - 1,fb->rgb_to_pixel(r,g,b));
      fb->draw_string("Hello,world!",string_x,string_y,fb->rgb_to_pixel(~r,~g,~b));
      fb->update();

      r += dr;
      if(r == 0)
        dr = 1;
      else if(r == 255)
        dr = -1;

      g += dg;
      if(g == 0)
        dg = 1;
      else if(g == 255)
        dg = -1;

      b += db;
      if(b == 0)
        db = 1;
      else if(b == 255)
        db = -1;
    }

    return 0;
  }
}
