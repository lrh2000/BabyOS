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
  };
public:
  inline log_t(log_level_t lvl = log_t::INFO)
    :loglvl(lvl)
  {
    *this<<loglvl_to_string[lvl];
  }
  inline ~log_t(void)
  {
    if(console)
      update();
  }

  log_t &operator <<(const char *s);
private:
  log_level_t loglvl;
public:
  static void set_console(console_t *console);
private:
  static void update(void);
private:
  static char buffer[4096];
  static size_t buffer_current;

  static console_t *console;

  static constexpr const char *loglvl_to_string[] =
    {
      "Kernel Log(TRACE):",
      "Kernel Log(DEBUG):",
      "Kernel Log(INFO):",
      "Kerenl Log(WARNING):",
      "Kerenl Log(ERROR):",
      "Kernel Log(FATAL):"
    };
};
