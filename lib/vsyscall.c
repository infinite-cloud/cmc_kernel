#include <inc/vsyscall.h>
#include <inc/lib.h>

static inline int32_t
vsyscall(int num)
{
	// LAB 12: Your code here.
	// cprintf("vsyscall() is not implemented yet!");
	return (num == VSYS_gettime) ? vsys[num] : -E_INVAL;
}

int vsys_gettime(void)
{
	return vsyscall(VSYS_gettime);
}
