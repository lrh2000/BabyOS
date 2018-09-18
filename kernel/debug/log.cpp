#include <debug.hpp>
#include <video.hpp>
#include <init.hpp>
#include <boot.hpp>

char log_t::buffer[4096];
size_t log_t::buffer_current;
console_t *log_t::console;

log_t &log_t::operator <<(const char *s)
{
  while(*s && buffer_current < sizeof(buffer))
    buffer[buffer_current++] = *s++;
  return *this;
}

log_t &log_t::operator <<(unsigned long num)
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

void log_t::set_console(console_t *console)
{
  log_t::console = console;
  if(!console)
    return;

  console->print("Kernel Log:The future OS kernel log will be printed on this console.\n");
  if(buffer_current) {
    console->print("Kernel Log:Some of the past OS kernel log will be printed below.\n");
    update();
    console->print("Kernel Log:The past OS kernel log has been printed above.\n");
    console->update();
  }
}

void log_t::update(void)
{
  if(buffer_current >= sizeof(buffer)) {
    buffer[sizeof(buffer) - 1] = 0;
    console->print(buffer);
    console->print("[This message is too long to be contained in log_t::buffer,"
                        "the rest of the message is omitted.]\n");
    buffer_current = 0;
    console->update();
    return;
  }
  buffer[buffer_current] = '\0';
  console->print(buffer);
  buffer_current = 0;
  console->update();
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
