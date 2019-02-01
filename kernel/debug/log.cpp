#include <debug.hpp>
#include <video.hpp>
#include <init.hpp>
#include <boot.hpp>
#include <locks.hpp>

char log_t::buffer[4096];
size_t log_t::buffer_current;
console_t *log_t::console;
mutex_t log_lock;

log_t::log_t(log_level_t lvl)
    :loglvl(lvl)
{
  log_lock.lock();
  if(lvl != log_t::NONE)
    *this<<loglvl_to_string[lvl];
}

log_t::~log_t(void)
{
  if(console)
    console->update();
  log_lock.unlock();
}

log_t &log_t::operator <<(const char *s)
{
  if(console)
    console->print(s);
  else
    while(*s && buffer_current < sizeof(buffer))
      buffer[buffer_current++] = *s++;
  return *this;
}

// NOTE: Now be only able to called during the system initialization.
void log_t::set_console(console_t *console)
{
  if(console) {
    console->print("Kernel Log:The OS kernel log will be attached to this console.\n");
    if(buffer_current) {
      console->print("Kernel Log: ------------ Begin of Saved History Log ------------\n");
      if(buffer_current >= sizeof(buffer))
        buffer[sizeof(buffer) - 1] = 0;
      console->print(buffer);
      console->print("Kernel Log: ------------- End of Saved History Log -------------\n");
      buffer_current = 0;
    }
    console->update();
  }

  log_t::console = console;
}

namespace dbglog
{
  static int setup_dbglog(bootinfo_t *bootinfo) INIT_FUNC(kernel,DEBUG_LOG);

  static int setup_dbglog(bootinfo_t *bootinfo)
  {
    log_t::set_console(bootinfo->video.info.console);

    log_t()<<"Initialize the kernel log system successfully.\n";

    return 0;
  }
}
