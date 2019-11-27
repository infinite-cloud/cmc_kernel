// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr;
	uint32_t err;
	pte_t pte;
	envid_t envid;
	int panic_err;

	addr = (void *) utf->utf_fault_va;
	err = utf->utf_err;
	pte = uvpt[PGNUM(addr)];
	envid = sys_getenvid();

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 9: Your code here.

	if (!(err & FEC_WR) || !(pte & PTE_COW))
	{
		panic("pgfault: invalid address");
	}

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.
	//   No need to explicitly delete the old page's mapping.
	//   Make sure you DO NOT use sanitized memcpy/memset routines when using UASAN.

	// LAB 9: Your code here.

	if ((panic_err = sys_page_alloc(envid, PFTEMP, 
		PTE_W | PTE_U | PTE_P)) < 0)
	{
		panic("pgfault: sys_page_alloc: %i", panic_err);
	}

#ifdef SANITIZE_USER_SHADOW_BASE
	__nosan_memcpy(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
#else
	memcpy(PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);
#endif

	if ((panic_err = sys_page_map(envid, PFTEMP, envid,
		ROUNDDOWN(addr, PGSIZE), PTE_W | PTE_U | PTE_P)) < 0)
	{
		panic("pgfault: sys_page_map: %i", panic_err);
	}

	if ((panic_err = sys_page_unmap(envid, PFTEMP)) < 0)
	{
		panic("pgfault: sys_page_unmap: %i", panic_err);
	}

	//panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	// LAB 9: Your code here.
	envid_t penvid;
	void *va;
	int err;

	penvid = sys_getenvid();
	va = (void *) (pn * PGSIZE);

	if ((uvpt[pn] & PTE_SHARE) == PTE_SHARE)
	{
		// Don't map child's memory copy-on-write.
		if ((err = sys_page_map(penvid, va, envid, va,
			uvpt[pn] & PTE_SYSCALL)) < 0)
		{
			panic("duppage: sys_page_map: %i", err);
		}
	}
	else if (((uvpt[pn] & PTE_W) == PTE_W) ||
		((uvpt[pn] & PTE_COW) == PTE_COW))
	{
		// Map child's memory copy-on-write.
		if ((err = sys_page_map(penvid, va, envid, va,
			PTE_COW | PTE_U | PTE_P)) < 0)
		{
			panic("duppage: sys_page_map: %i", err);
		}

		// Map our own memory COW again.
		if ((err = sys_page_map(penvid, va, penvid, va,
			PTE_COW | PTE_U | PTE_P)) < 0)
		{
			panic("duppage: sys_page_map: %i", err);
		}
	}
	else
	{
		// Map child's memory non-COW.
		if ((err = sys_page_map(penvid, va, envid, va,
			PTE_U | PTE_P)) < 0)
		{
			panic("duppage: sys_page_map: %i", err);
		}
	}

	//panic("duppage not implemented");
	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	// LAB 9: Your code here.
	void _pgfault_upcall();
	envid_t envid;
	uint32_t addr;
	int err;

	set_pgfault_handler(pgfault);

	if ((envid = sys_exofork()) < 0)
	{
		panic("sys_exofork: %i", envid);
	}

	// Child.
	if (!envid)
	{
		// Get the child's ENVID.
		thisenv = envs + ENVX(sys_getenvid());
		return 0;
	}

	for (addr = 0; addr < USTACKTOP; addr += PGSIZE)
	{
		// If the page is present, copy our address space to the child.
#ifdef SANITIZE_USER_SHADOW_BASE
		if (addr >= SANITIZE_USER_SHADOW_BASE &&
			addr <= SANITIZE_USER_SHADOW_BASE +
			SANITIZE_USER_SHADOW_SIZE)			
		{
			continue;
		}

		if (addr >= SANITIZE_USER_EXTRA_SHADOW_BASE &&
			 addr <= SANITIZE_USER_EXTRA_SHADOW_BASE +
			 SANITIZE_USER_EXTRA_SHADOW_SIZE)
		{
			continue;
		}

		if (addr >= SANITIZE_USER_FS_SHADOW_BASE &&
			 addr <= SANITIZE_USER_FS_SHADOW_BASE +
			 SANITIZE_USER_FS_SHADOW_SIZE)
		{
			continue;
		}
#endif
		if (((uvpd[PDX(addr)] & PTE_P) == PTE_P) &&
			((uvpt[PGNUM(addr)] & PTE_P) == PTE_P))
		{
			duppage(envid, PGNUM(addr));
		}
	}

	if ((err = sys_env_set_pgfault_upcall(envid, _pgfault_upcall)) < 0)
	{
		panic("fork: sys_env_set_pgfault_upcall: %i", err);
	}

	// Allocate the child's exception stack.
	if ((err = sys_page_alloc(envid, (void *) (UXSTACKTOP - PGSIZE),
		PTE_W | PTE_U | PTE_P)) < 0)
	{
		panic("fork: sys_page_alloc: %i", err);
	}

// Duplicating shadow addresses is insane. Make sure to skip shadow addresses in COW above.

#ifdef SANITIZE_USER_SHADOW_BASE
	for (addr = SANITIZE_USER_SHADOW_BASE; addr < SANITIZE_USER_SHADOW_BASE +
		SANITIZE_USER_SHADOW_SIZE; addr += PGSIZE)
		if (sys_page_alloc(envid, (void *) addr, PTE_P | PTE_U | PTE_W))
			panic("Fork: failed to alloc shadow base page");
	for (addr = SANITIZE_USER_EXTRA_SHADOW_BASE; addr < SANITIZE_USER_EXTRA_SHADOW_BASE +
		SANITIZE_USER_EXTRA_SHADOW_SIZE; addr += PGSIZE)
		if (sys_page_alloc(envid, (void *) addr, PTE_P | PTE_U | PTE_W))
			panic("Fork: failed to alloc shadow extra base page");
	for (addr = SANITIZE_USER_FS_SHADOW_BASE; addr < SANITIZE_USER_FS_SHADOW_BASE +
		SANITIZE_USER_FS_SHADOW_SIZE; addr += PGSIZE)
		if (sys_page_alloc(envid, (void *) addr, PTE_P | PTE_U | PTE_W))
			panic("Fork: failed to alloc shadow fs base page");
#endif

	if ((err = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
	{
		panic("fork: sys_env_set_status: %i", err);
	}

	return envid;
	//panic("fork not implemented");
}

// Challenge!
int
sfork(void)
{
	panic("sfork not implemented");
	return -E_INVAL;
}
