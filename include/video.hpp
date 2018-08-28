#pragma once
#include <env.hpp>
#include <memory.hpp>

class framebuffer_t
{
public:
  typedef uint32_t pixel_t;
  enum pixel_format_t
  {
    PIXEL32_RGB,
    PIXEL32_BGR,
    // TODO: Support more pixel formats.
  };

  enum : uint32_t { FONT_WIDTH  = 8 };
  enum : uint32_t { FONT_HEIGHT = 16 };

public:
  framebuffer_t(physaddr_t fbaddr,uint32_t height,uint32_t width,
                uint32_t real_width,pixel_format_t px_format);
  virtual ~framebuffer_t(void);

  pixel_t rgb_to_pixel(uint8_t red,uint8_t blue,uint8_t green);
  void fill_rect(uint32_t xmin,uint32_t ymin,uint32_t xmax,uint32_t ymax,pixel_t px);
  void draw_char(char c,uint32_t x,uint32_t y,pixel_t fg,pixel_t bg);
  void update(void);

  inline uint32_t width(void) { return _width; }
  inline uint32_t height(void) { return _height; }

  inline operator bool(void) { return !has_error; }

  inline void set_start_line(uint32_t line)
  {
    start_line += line;
    start_line %= _height;
  }

protected:
  inline bool validate_region(uint32_t &xmin,uint32_t &ymin,uint32_t &xmax,uint32_t &ymax)
  {
    if(xmin > xmax)
      return false;
    if(ymin > ymax)
      return false;
    if(xmin >= _width)
      return false;
    if(ymin >= _height)
      return false;
    if(xmax >= _width)
      xmax = _width - 1;
    if(ymax >= _height)
      ymax = _height - 1;
    return true;
  }
  inline void report_dirty_region(uint32_t xmin,uint32_t ymin,uint32_t xmax,uint32_t ymax)
  {
    if(update_xmin > xmin)
      update_xmin = xmin;
    if(update_ymin > ymin)
      update_ymin = ymin;
    if(update_xmax < xmax)
      update_xmax = xmax;
    if(update_ymax < ymax)
      update_ymax = ymax;
  }

  inline size_t get_offset(uint32_t x,uint32_t y)
  {
    size_t result = start_line + y;
    result -= result >= _height ? _height : 0;
    (result *= real_width) += x;
    return result;
  }
  inline pixel_t *get_next_line(pixel_t *now,uint32_t y)
  {
    if(y + start_line + 1 == _height)
      return now - (size_t)(_height - 1) * real_width;
    return now + real_width;
  }

private:
  pixel_t *vram_fb;
  pixel_t *ram_fb;
  uint32_t _height;
  uint32_t _width;
  uint32_t real_width;
  pixel_format_t px_format;

  uint32_t start_line;
  uint32_t last_start_line;

  uint32_t update_xmin;
  uint32_t update_xmax;
  uint32_t update_ymin;
  uint32_t update_ymax;

  bool has_error;
};

class console_t
{
public:
  enum : uint32_t { FONT_WIDTH  = framebuffer_t::FONT_WIDTH };
  enum : uint32_t { FONT_HEIGHT = framebuffer_t::FONT_HEIGHT };
public:
  console_t(framebuffer_t *fb,framebuffer_t::pixel_t foreground,
                              framebuffer_t::pixel_t background);
  virtual ~console_t(void);

  void print(const char *s);
  void update(void);

  inline operator bool(void) { return !has_error; }
private:
  framebuffer_t *fb;
  unsigned char *text;

  uint32_t width;
  uint32_t height;
  uint32_t start_line;
  uint32_t margin_ypos;

  uint32_t xpos;
  uint32_t ypos;
  uint32_t last_xpos;
  uint32_t last_ypos;

  bool has_error;

  framebuffer_t::pixel_t background;
  framebuffer_t::pixel_t foreground;
};
