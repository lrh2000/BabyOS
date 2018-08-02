#include <init.hpp>
#include "efi.hpp"

static void efi_main(efi::handle_t image_handle,efi::system_table_t *system_table)
                   INIT_FUNC(boot,EFI);

static void efi_main(efi::handle_t image_handle,efi::system_table_t *system_table)
{
  uint16_t string[] = {'H','e','l','l','o',',','w','o','r','l','d','!','\0'};
  (*system_table->con_out->output_string)(system_table->con_out,string);

  (void)image_handle;
}
