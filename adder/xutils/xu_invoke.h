// GENERATED FILE

#ifndef _XU_INVOKE_H_
#define _XU_INVOKE_H_

#include <stdbool.h>
#include <sh_value.h>

typedef struct vm_t vm_t;
typedef struct xu_caller_t xu_caller_t;

val_t xu_int(int v);
val_t xu_float(float v);
val_t xu_bool(bool v);
val_t xu_char(char v);
val_t xu_string(vm_t* vm, char* str);

int   icall0(vm_t* vm, xu_caller_t* caller);
float fcall0(vm_t* vm, xu_caller_t* caller);
bool  bcall0(vm_t* vm, xu_caller_t* caller);
char  ccall0(vm_t* vm, xu_caller_t* caller);
char* scall0(vm_t* vm, xu_caller_t* caller);
void  vcall0(vm_t* vm, xu_caller_t* caller);

int   icall1(vm_t* vm, xu_caller_t* caller, val_t arg1);
float fcall1(vm_t* vm, xu_caller_t* caller, val_t arg1);
bool  bcall1(vm_t* vm, xu_caller_t* caller, val_t arg1);
char  ccall1(vm_t* vm, xu_caller_t* caller, val_t arg1);
char* scall1(vm_t* vm, xu_caller_t* caller, val_t arg1);
void  vcall1(vm_t* vm, xu_caller_t* caller, val_t arg1);

int   icall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2);
float fcall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2);
bool  bcall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2);
char  ccall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2);
char* scall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2);
void  vcall2(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2);

int   icall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3);
float fcall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3);
bool  bcall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3);
char  ccall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3);
char* scall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3);
void  vcall3(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3);

int   icall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4);
float fcall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4);
bool  bcall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4);
char  ccall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4);
char* scall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4);
void  vcall4(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4);

int   icall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5);
float fcall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5);
bool  bcall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5);
char  ccall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5);
char* scall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5);
void  vcall5(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5);

int   icall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6);
float fcall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6);
bool  bcall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6);
char  ccall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6);
char* scall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6);
void  vcall6(vm_t* vm, xu_caller_t* caller, val_t arg1, val_t arg2, val_t arg3, val_t arg4, val_t arg5, val_t arg6);

#define xu_callv(V, C) vcall0((V), (C))
#define xu_callb(V, C) bcall0((V), (C))
#define xu_callb_i(V, C, A1) bcall1((V), (C), xu_int((A1)))
#define xu_callv_f(V, C, A1) vcall1((V), (C), xu_float((A1)))

#endif // _XU_INVOKE_H_