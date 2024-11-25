#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "syscall.h"
#include "fs.h"
#include "file.h"

uint64
sys_chmod(void)
{
    char path[MAXPATH];
    int mode;
    struct inode *ip;

    // Obtener argumentos de la syscall
    if (argstr(0, path, MAXPATH) < 0)  // Extraer el nombre del archivo
        return -1;
    argint(1, &mode);                  // Extraer el modo (entero)
    if (mode < 0 || mode > 5)          // Validar que el modo sea válido
        return -1;

    begin_op();

    // Buscar el inodo del archivo especificado por path
    if ((ip = namei(path)) == 0) {
        end_op();
        return -1;
    }

    ilock(ip);

    // Validar si el archivo es inmutable
    if (ip->permissions == 5) {
        iunlockput(ip);
        end_op();
        return -1; // No se pueden cambiar los permisos de un archivo inmutable
    }

    // Actualizar permisos
    ip->permissions = mode;

    // Guardar cambios en el disco
    iupdate(ip);

    iunlockput(ip);
    end_op();

    return 0;
}



uint64
sys_mprotect(void)
{
    uint64 addr;
    int len;

    // Obtener los argumentos directamente
    argaddr(0, &addr);
    argint(1, &len);

    // Validar los valores extraídos
    if (addr % PGSIZE != 0 || len <= 0)
        return -1;

    struct proc *p = myproc();
    uint64 start = addr;
    uint64 end = addr + len * PGSIZE;

    // Validar que la región pertenece al proceso
    if (start >= p->sz || end > p->sz)
        return -1;

    // Recorre las páginas y modifica los permisos
    for (uint64 a = start; a < end; a += PGSIZE) {
        pte_t *pte = walk(p->pagetable, a, 0);
        if (!pte || !(*pte & PTE_V))
            return -1; // Dirección inválida

        *pte &= ~PTE_W; // Deshabilitar escritura
    }

    sfence_vma(); // Invalidar TLB
    return 0;
}



// Implementación de sys_munprotect
uint64
sys_munprotect(void)
{
    uint64 addr;
    int len;

    // Obtener los argumentos directamente
    argaddr(0, &addr);
    argint(1, &len);

    // Validar los valores extraídos
    if (addr % PGSIZE != 0 || len <= 0)
        return -1;

    struct proc *p = myproc();
    uint64 start = addr;
    uint64 end = addr + len * PGSIZE;

    // Validar que la región pertenece al proceso
    if (start >= p->sz || end > p->sz)
        return -1;

    // Recorre las páginas y restaura los permisos
    for (uint64 a = start; a < end; a += PGSIZE) {
        pte_t *pte = walk(p->pagetable, a, 0);
        if (!pte || !(*pte & PTE_V))
            return -1; // Dirección inválida

        *pte |= PTE_W; // Habilitar escritura
    }

    sfence_vma(); // Invalidar TLB
    return 0;
}







uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

// Implementación de getppid
uint64
sys_getppid(void)
{
    struct proc *curproc = myproc();
    if (curproc->parent) // Si el proceso tiene un padre
        return curproc->parent->pid;
    return -1; // Si no tiene padre
}

// Implementación de getancestor
uint64
sys_getancestor(void)
{
    int level;

    // Obtener el argumento del nivel
    argint(0, &level);

    // Validar que el nivel no sea negativo
    if (level < 0)
        return -1;

    struct proc *p = myproc(); // Proceso actual

    // Recorre hacia arriba por los ancestros
    for (int i = 0; i < level; i++) {
        if (!p->parent) // Si no hay más ancestros válidos
            return -1;
        p = p->parent; // Avanzar al proceso padre
    }

    return p->pid; // Retornar el PID del ancestro
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}
