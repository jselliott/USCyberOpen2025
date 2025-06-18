#define _GNU_SOURCE
#include <sys/mman.h>
#include <caml/mlvalues.h>
#include <caml/memory.h>

CAMLprim value caml_mprotect(value addr, value len, value prot) {
  CAMLparam3(addr, len, prot);
  int res = mprotect((void *)Nativeint_val(addr), Int_val(len), Int_val(prot));
  CAMLreturn(Val_int(res));
}
