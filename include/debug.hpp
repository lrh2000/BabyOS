#include <env.hpp>

class console_t;

class log_t
{
public:
  enum log_level_t
  {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL,
    NONE,
  };
  struct setw_t { unsigned int width; };
  struct setbase_t { unsigned int base; };

public:
  log_t(log_level_t lvl = log_t::INFO)
    :loglvl(lvl),base(10),width(0)
  {
    if(lvl != log_t::NONE)
      *this<<loglvl_to_string[lvl];
  }
  ~log_t(void)
  {
    if(console)
      update();
  }

  log_t &operator <<(setw_t width) { this->width = width.width;return *this; }
  log_t &operator <<(setbase_t base) { this->base = base.base;return *this; }

  log_t &operator <<(const char *s);
  log_t &operator <<(unsigned long num);
private:
  log_level_t loglvl;
  unsigned int base,width;
public:
  static void set_console(console_t *console);

  static setbase_t setbase(unsigned int base) { return (setbase_t){base}; }
  static setw_t setw(unsigned int width) { return (setw_t){width}; }
private:
  static void update(void);
private:
  static char buffer[4096];
  static size_t buffer_current;

  static console_t *console;

  static constexpr const char *loglvl_to_string[] = {
    "Kernel Log(TRACE):",
    "Kernel Log(DEBUG):",
    "Kernel Log(INFO):",
    "Kerenl Log(WARNING):",
    "Kerenl Log(ERROR):",
    "Kernel Log(FATAL):",
  };
};
