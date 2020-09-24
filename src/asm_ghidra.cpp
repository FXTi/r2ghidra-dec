/* radare - LGPL - Copyright 2020 - FXTi */

#include <r_lib.h>
#include <r_asm.h>
#include "SleighAsm.h"

static SleighAsm sasm;

//#define DEBUG_EXCEPTIONS

static int disassemble(RAsm *a, RAsmOp *op, const ut8 *buf, int len)
{
	int r = 0;
	RIO *rio = nullptr;
	if(!a->cpu)
		return r;
#ifndef DEBUG_EXCEPTIONS
	try
	{
#endif
		RBin *bin = a->binb.bin;
		if(!bin)
		{
			rio = r_io_new();
			RBuffer *tmp_buf = r_buf_new_with_bytes(buf, len);
			r_io_open_buffer(rio, tmp_buf, R_PERM_RWX, 0);
			r_buf_free(tmp_buf);
			sasm.sleigh_id.clear(); // For newly created RIO
			sasm.init(a->cpu, rio, SleighAsm::getConfig(a));
			r = sasm.disassemble(op, 0);
		}
		else
		{
			sasm.init(a->cpu, bin->iob.io, SleighAsm::getConfig(a));
			sasm.check(a->pc, buf, len);
			r = sasm.disassemble(op, a->pc);
		}
#ifndef DEBUG_EXCEPTIONS
	}
	catch(const LowlevelError &e)
	{
		r_strbuf_set(&op->buf_asm, e.explain.c_str());
		r = 1;
	}
#endif
	if(rio)
		r_io_free(rio);
	op->size = r;
	return r;
}

RAsmPlugin r_asm_plugin_ghidra = {
	/* .name = */ "r2ghidra",
	/* .arch = */ "sleigh",
	/* .author = */ "FXTi",
	/* .version = */ nullptr,
	/* .cpus = */ nullptr,
	/* .desc = */ "SLEIGH Disassembler from Ghidra",
	/* .license = */ "GPL3",
	/* .user = */ nullptr,
	/* .bits = */ 0,
	/* .endian = */ 0,
	/* .init = */ nullptr,
	/* .fini = */ nullptr,
	/* .disassemble = */ &disassemble,
	/* .assemble = */ nullptr,
	/* .modify */ nullptr,
	/* .mnemonics = */ nullptr,
	/* .features = */ nullptr
};

#ifndef CORELIB
#ifdef __cplusplus
extern "C"
#endif
R_API RLibStruct radare_plugin = {
	/* .type = */ R_LIB_TYPE_ASM,
	/* .data = */ &r_asm_plugin_ghidra,
	/* .version = */ R2_VERSION,
	/* .free = */ nullptr
#if R2_VERSION_MAJOR >= 4 && R2_VERSION_MINOR >= 2
	, "r2ghidra-dec"
#endif
};
#endif
