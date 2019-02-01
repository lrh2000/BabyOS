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
  log_t(log_level_t lvl = log_t::INFO);
  ~log_t(void);

  void hex(void);
  void hex64(void);

  log_t &operator <<(setw_t width) { this->width = width.width;return *this; }
  log_t &operator <<(setbase_t base) { this->base = base.base;return *this; }
  log_t &operator <<(void (log_t::*func)(void)) { (this->*func)();return *this; }

  log_t &operator <<(const char *s);
  log_t &operator <<(unsigned long num)
  {
    char s[21];
    int i = 20;
    int figure = this->width;

    s[i--] = 0;
    do
      ((s[i--] = num % base + '0') <= '9') ? 0 : (s[i + 1] += 'A' - '0' - 10);
    while((num /= base) | (figure && --figure));

    return *this<<s + i + 1;
  }

private:
  log_level_t loglvl;
  unsigned int base = 10,width = 0;
public:
  static void set_console(console_t *console);

  static setbase_t setbase(unsigned int base) { return (setbase_t){base}; }
  static setw_t setw(unsigned int width) { return (setw_t){width}; }
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

inline void log_t::hex(void) { *this<<setbase(16); }
inline void log_t::hex64(void) { *this<<setbase(16)<<setw(16); }
