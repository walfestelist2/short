#include "common.h"
#include "run.h"

struct sh_vm {
	sh_byte *mem;
	sh_size mem_size;

	sh_var *label_mem;
	sh_size label_mem_size;
};

static void init (

void sh_run (struct sh_bc *bc) {
	struct sh_vm vm;
}
