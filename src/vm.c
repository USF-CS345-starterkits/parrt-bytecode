/*
The MIT License (MIT)

Copyright (c) 2015 Terence Parr, Hanzhou Shi, Shuai Yuan, Yuanyuan Zhang

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "vm.h"
#include "loader.h"

VM_INSTRUCTION vm_instructions[] = {
	{"HALT",  HALT,  {}, 0},

	{"IADD",  IADD,  {}, 2},
	{"ISUB",  ISUB,  {}, 2},
	{"IMUL",  IMUL,  {}, 2},
	{"IDIV",  IDIV,  {}, 2},
	{"SADD",  SADD,  {}, 2},

	{"OR",    OR,    {}, 2},
	{"AND",   AND,   {}, 2},
	{"INEG",  INEG,  {}, 1},
	{"NOT",   NOT,   {}, 1},

	{"I2S",   I2S,   {}, 1},

	{"IEQ",   IEQ,   {}, 2},
	{"INEQ",  INEQ,  {}, 2},
	{"ILT",   ILT,   {}, 2},
	{"ILE",   ILE,   {}, 2},
	{"IGT",   IGT,   {}, 2},
	{"IGE",   IGE,   {}, 2},
	{"SEQ",   SEQ,   {}, 2},
	{"SNEQ",  SNEQ,  {}, 2},
	{"SGT",   SGT,   {}, 2},
	{"SGE",   SGE,   {}, 2},
	{"SLT",	  SLT,	 {}, 2},
	{"SLE",	  SLE,	 {}, 2},

	{"BR",	  BR,	 {4}, 0}, // branch to absolute address
	{"BRF",	  BRF,	 {4}, 0},

	{"ICONST",	ICONST,	   	{4}, 0},
	{"SCONST",	SCONST,	   	{2}, 0},

	{"LOAD",	LOAD,	    {2}, 0},
	{"STORE",	STORE,	    {2}, 1},
	{"SINDEX", 	SINDEX,		{},  2},

	{"POP",		POP,		{},  0},
	{"CALL",	CALL,	    {4,2}, 0}, // num stack operands is actually a variable and 2nd operand
	{"LOCALS",	LOCALS,	    {2}, 0},
	{"RET",		RET,		{},  1},

	{"PRINT",	PRINT,	   	{},  1},
	{"SLEN",	SLEN,	   	{},  1},
	{"SFREE",	SFREE,	   	{2}, 0} // free a str in a local
};

static void vm_print_instr(VM *vm, addr32 ip);
static void vm_print_stack(VM *vm);
static char *vm_print_element(char *buffer, element el);
static inline int32_t int32(const byte *data, addr32 ip);
static inline int16_t int16(const byte *data, addr32 ip);
static void vm_trace_print_element(VM *vm, element el);

VM *vm_alloc() {
	/* TODO:
	calloc a VM
	make space for the output and trace strings
	return the vm
	*/
}

void vm_init(VM *vm, byte *code, int code_size)
{
	vm->code = code;
	vm->code_size = code_size;
	vm->sp = -1; // grow upwards, stack[sp] is top of stack and valid
	vm->callsp = -1;
}

void vm_free(VM *vm) {
	/* TODO:
	free the strings and func_names
	free the trace and output strings
	free the code memory
	free the VM itself
	*/
}

// You don't need to use/create these functions but I used them
// to help me when there are bugs in my code.
static void inline validate_stack_address(VM *vm, int a) { }
static void inline validate_stack(VM *vm, byte opcode, int sp) { }

void vm_exec(VM *vm, bool trace_to_stderr) {
	// TODO: fill in with your fetch-decode-execute functionality
	// call vm_print_instr(vm, ip), vm_print_stack(vm) as necessary for tracing
}

/* return a 32-bit integer at data[ip] */
static inline int32_t int32(const byte *data, addr32 ip)
{
	return *((int32_t *)&data[ip]);
}

/* return a 16-bit integer at data[ip] */
static inline int16_t int16(const byte *data, addr32 ip)
{
	return *((int16_t *)&data[ip]); // could be negative value
}

void vm_print_instr_opnd0(const VM *vm, addr32 ip) {
	int op_code = vm->code[ip];
	VM_INSTRUCTION *inst = &vm_instructions[op_code];
	print(vm->trace, "%04d:  %-25s", ip, inst->name);
}

void vm_print_instr_opnd1(const VM *vm, addr32 ip) {
	int op_code = vm->code[ip];
	VM_INSTRUCTION *inst = &vm_instructions[op_code];
	int sz = inst->opnd_sizes[0];
	switch (sz) {
		case 2:
			print(vm->trace, "%04d:  %-15s%-10d", ip, inst->name, int16(vm->code, ip + 1));
			break;
		case 4:
			print(vm->trace, "%04d:  %-15s%-10d", ip, inst->name, int32(vm->code, ip + 1));
			break;
		default:
			break;
	}
}

/* currently only a CALL instr */
void vm_print_instr_opnd2(const VM *vm, addr32 ip) {
	int op_code = vm->code[ip];
	VM_INSTRUCTION *inst = &vm_instructions[op_code];
	char buf[100];
	sprintf(buf, "%d, %d", int32(vm->code, ip + 1), int16(vm->code, ip + 5));
	print(vm->trace, "%04d:  %-15s%-10s", ip, inst->name, buf);
}

static void vm_print_instr(VM *vm, addr32 ip)
{
	int op_code = vm->code[ip];
	VM_INSTRUCTION *inst = &vm_instructions[op_code];
	if ( inst->opnd_sizes[1]>0 ) {
		vm_print_instr_opnd2(vm, ip);
	}
	else if ( inst->opnd_sizes[0]>0 ) {
		vm_print_instr_opnd1(vm, ip);
	}
	else {
		vm_print_instr_opnd0(vm, ip);
	}
}

static void vm_print_stack(VM *vm) {
	// stack grows upwards; stack[sp] is top of stack
	print(vm->trace, "calls=[");
	for (int i = 0; i <= vm->callsp; i++) {
		Activation_Record *frame = &vm->call_stack[i];
		print(vm->trace, " %s=[", frame->name);
		for (int j = 0; j < frame->nlocals+frame->nargs; ++j) {
			print(vm->trace, " ");
			vm_trace_print_element(vm, frame->locals[j]);
		}
		print(vm->trace, " ]");
	}
	print(vm->trace, " ]  ");
	print(vm->trace, "stack=[");
	for (int i = 0; i <= vm->sp; i++) {
		print(vm->trace, " ");
		vm_trace_print_element(vm, vm->stack[i]);
	}
	print(vm->trace, " ] sp=%d\n", vm->sp);
}

void vm_trace_print_element(VM *vm, element el) {
	if (el.type == STRING) {
		print(vm->trace, "\"");
		vm_print_element(vm->trace, el);
		print(vm->trace, "\"");
	}
	else {
		vm_print_element(vm->trace, el);
	}
}

char *vm_print_element(char *buffer, element el) {
	switch ( el.type ) {
		case INT :
			return print(buffer, "%d", el.i);
		case BOOLEAN :
			return print(buffer, "%s", el.b ? "true" : "false");
		case STRING :
			return print(buffer, "%s", el.s->str);
		default:
			return print(buffer, "%s", "?");
	}
}

char *print(char *buffer, char *fmt, ...) {
	va_list args;
	char buf[1000];

	size_t n = strlen(buffer);
	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf) - 1, fmt, args);
	strcat(buffer, buf);
	va_end(args);
	return &buffer[n];
}
