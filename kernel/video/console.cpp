#include <video.hpp>
#include <init.hpp>
#include <boot.hpp>
#include <debug.hpp>

using fb_pixel_t = framebuffer_t::pixel_t;

console_t::console_t(framebuffer_t *fb,fb_pixel_t foreground,fb_pixel_t background)
  :fb(fb),background(background),foreground(foreground)
{
  {
    uint32_t width = fb->width();
    uint32_t height = fb->height();
    fb->fill_rect(0,0,width - 1,height - 1,background);
    fb->update();

    this->width = width / FONT_WIDTH;
    this->height = height / FONT_HEIGHT;
  }

  margin_ypos = height * FONT_HEIGHT;

  size_t text_size = sizeof(text[0]);
  (text_size *= width) *= height;
  (text_size += PAGE_SIZE - 1) >>= PAGE_SHIFT;
  text = (unsigned char *)alloc_pages(text_size);
  has_error = text == nullptr;

  start_line = 0;
  xpos = ypos = 0;
  last_xpos = last_ypos = 0;

  if(!text)
    return;
  unsigned char *x = text;
  for(uint32_t i = 0;i < height;++i,x += width)
    *x = 0;
}

console_t::~console_t(void)
{
  if(!text)
    return;
  size_t text_size = sizeof(text[0]);
  (text_size *= width) *= height;
  (text_size += PAGE_SIZE - 1) >>= PAGE_SHIFT;
  dealloc_pages((void *)text,text_size);
}

void console_t::print(const char *x)
{
  uint32_t real_ypos = ypos % height;
  unsigned char *now = text + real_ypos * width + xpos;

  while(char c = *x++)
  {
    if(xpos == width || c == '\n') {
      if(xpos != width)
        *now = 0;
      ++ypos;
      ++real_ypos;
      xpos = 0;
      if(real_ypos == height)
        real_ypos = 0;
      now = text + real_ypos * width;
      continue;
    }
    *now++ = c;
    ++xpos;
  }
}

void console_t::update(void)
{
  bool margin_fill = false;

  if(start_line + height <= ypos) {
    if(last_ypos + height > ypos) {
      uint32_t z = ypos - start_line - height + 1;
      fb->set_start_line(z * FONT_HEIGHT);
      uint32_t fb_height = fb->height(),fb_width = fb->width();
      if(fb_height != margin_ypos)
        fb->fill_rect(0,margin_ypos,fb_width - 1,fb_height - 1,background);
    }else {
      last_xpos = 0;
      last_ypos = ypos - height + 1;
    }

    margin_fill = true;
    start_line = ypos - height + 1;
  }

  uint32_t real_ypos = last_ypos % height;
  uint32_t screen_ypos = (last_ypos - start_line) * FONT_HEIGHT;
  uint32_t screen_xpos = last_xpos * FONT_WIDTH;
  unsigned char *now = text + real_ypos * width + last_xpos;
  while(xpos != last_xpos || ypos != last_ypos)
  {
    if(last_xpos == width || !*now) {
      if(last_xpos != width && margin_fill)
        fb->fill_rect(screen_xpos,screen_ypos,
            width * FONT_WIDTH - 1,screen_ypos + FONT_HEIGHT - 1,background);

      last_xpos = 0;
      screen_xpos = 0;
      ++last_ypos;
      screen_ypos += FONT_HEIGHT;
      ++real_ypos;
      if(real_ypos == height)
        real_ypos = 0;
      now = text + real_ypos * width;
      continue;
    }

    fb->draw_char(*now++,screen_xpos,screen_ypos,foreground,background);
    screen_xpos += FONT_WIDTH;
    ++last_xpos;
  }
  if(margin_fill)
    fb->fill_rect(screen_xpos,screen_ypos,
      width * FONT_WIDTH - 1,screen_ypos + FONT_HEIGHT - 1,background);

  fb->update();
}

namespace console
{
  static int setup_videocon(bootinfo_t *bootinfo) INIT_FUNC(kernel,VIDEO_CON);

  static int setup_videocon(bootinfo_t *bootinfo)
  {
    static uint8_t boot_console[sizeof(console_t)];

    auto &info = bootinfo->video.info;
    framebuffer_t *fb = info.fb;

    auto console = new(boot_console) console_t(fb,
        fb->rgb_to_pixel(255,255,255),fb->rgb_to_pixel(0,0,0));
    info.console = console;
    if(!*console) {
      console->~console_t();
      return -1;
    }

    console->print("Initialize the kernel console successfully!\n");
    console->update();

    log_t()<<"Initialize the kernel console successfully.\n";

    return 0;
  }
}
