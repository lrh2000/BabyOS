#include <env.hpp>
#include <boot.hpp>
#include <memory.hpp>
#include <init.hpp>
#include <video.hpp>
#include <debug.hpp>

namespace framebuffer
{
  extern const uint8_t fontdata_8x16[4096];
}

framebuffer_t::framebuffer_t(physaddr_t fbaddr,uint32_t height,uint32_t width,
                     uint32_t real_width,pixel_format_t px_format)
  :_height(height),_width(width),real_width(real_width),px_format(px_format)
{
  vram_fb = (pixel_t *)addr_phys2virt(fbaddr);

  size_t fb_sz = sizeof(ram_fb[0]);
  (fb_sz *= real_width) *= _height;
  fb_sz = (fb_sz + PAGE_SIZE - 1) >> PAGE_SHIFT;
  ram_fb = (pixel_t *)alloc_pages(fb_sz);
  has_error = ram_fb == nullptr;

  start_line = last_start_line = 0;

  update_xmin = update_ymin = ~(uint32_t)0;
  update_xmax = update_ymax = 0;
}

framebuffer_t::~framebuffer_t(void)
{
  if(!ram_fb)
    return;
  size_t fb_sz = sizeof(ram_fb[0]);
  (fb_sz *= real_width) *= _height;
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

  pixel_t *offset = ram_fb + get_offset(xmin,ymin);
  uint64_t pixels = pixel;
  (pixels <<= 32) |= pixel;

  while(ymin <= ymax)
  {
    uint32_t t = xmax - xmin + 1;
    uint64_t *now;

    *offset = pixel;
    now = (uint64_t *)(offset + (t & 1));
    t >>= 1;
    while(t--)
      *now++ = pixels;

    offset = get_next_line(offset,ymin++);
  }
}

void framebuffer_t::draw_char(char c,uint32_t x,uint32_t y,pixel_t fg,pixel_t bg)
{
  if(x + FONT_WIDTH > _width)
    return;
  if(y + FONT_HEIGHT > _height)
    return;

  const uint8_t *font = &framebuffer::fontdata_8x16[(size_t)c << 4];
  pixel_t *buf = ram_fb + get_offset(x,y);

  report_dirty_region(x,y,x + FONT_WIDTH - 1,y + FONT_HEIGHT - 1);
  for(size_t dy = 0;dy < FONT_HEIGHT;buf = get_next_line(buf,y + dy),++dy,++font)
    for(size_t dx = 0;dx < FONT_WIDTH;++dx)
      buf[FONT_WIDTH - dx] = (*font & (1 << dx)) ? fg : bg;
}

void framebuffer_t::update(void)
{
  if(last_start_line != start_line) {
    update_xmin = update_ymin = 0;
    update_xmax = _width - 1;
    update_ymax = _height - 1;
    last_start_line = start_line;
  }else {
    if(!validate_region(update_xmin,update_ymin,update_xmax,update_ymax))
      return;
  }

  pixel_t *offset_ram = ram_fb,*offset_vram = vram_fb;
  offset_ram += get_offset(update_xmin,update_ymin);
  offset_vram += update_ymin * real_width + update_xmin;

  while(update_ymin <= update_ymax)
  {
    uint32_t t = update_xmax - update_xmin + 1;
    uint64_t *from,*to;

    *offset_vram = *offset_ram;
    from = (uint64_t *)(offset_ram + (t & 1));
    to = (uint64_t *)(offset_vram + (t & 1));
    t >>= 1;
    while(t--)
      *to++ = *from++;

    offset_ram = get_next_line(offset_ram,update_ymin);
    offset_vram += real_width;
    ++update_ymin;
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

    auto &video = bootinfo->video.early;
    framebuffer_t *fb = new(boot_fb) framebuffer_t(video.vram_address,
        video.height,video.width,video.vram_width,video.pixel_format);
    if(!*fb) {
      fb->~framebuffer_t();
      return -1;
    }

    bootinfo->video.info.fb = fb;

    log_t()<<"Initialize the video framebuffer successfully.\n";

    return 0;
  }
}
