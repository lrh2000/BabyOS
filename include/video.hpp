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

public:
  framebuffer_t(physaddr_t fbaddr,uint32_t height,uint32_t width,
                uint32_t real_width,pixel_format_t px_format);
  virtual ~framebuffer_t(void);

  pixel_t rgb_to_pixel(uint8_t red,uint8_t blue,uint8_t green);
  void fill_rect(uint32_t xmin,uint32_t ymin,uint32_t xmax,uint32_t ymax,pixel_t px);
  void draw_char(char c,uint32_t x,uint32_t y,pixel_t px);
  void draw_string(const char *s,uint32_t x,uint32_t y,pixel_t px);
  void update(void);

protected:
  inline bool validate_region(uint32_t &xmin,uint32_t &ymin,uint32_t &xmax,uint32_t &ymax)
  {
    if(xmin > xmax)
      return false;
    if(ymin > ymax)
      return false;
    if(xmin >= width)
      return false;
    if(ymin >= height)
      return false;
    if(xmax >= width)
      xmax = width - 1;
    if(ymax >= height)
      ymax = height - 1;
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

private:
  pixel_t *vram_fb;
  pixel_t *ram_fb;
  uint32_t height;
  uint32_t width;
  uint32_t real_width;
  pixel_format_t px_format;

  uint32_t update_xmin;
  uint32_t update_xmax;
  uint32_t update_ymin;
  uint32_t update_ymax;
};
