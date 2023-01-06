#include <os/ioremap.h>
#include <os/mm.h>
#include <pgtable.h>
#include <type.h>

// maybe you can map it to IO_ADDR_START ?
static uintptr_t io_base = IO_ADDR_START;

void *ioremap(unsigned long phys_addr, unsigned long size)
{
    // TODO: [p5-task1] map one specific physical region to virtual address
    uintptr_t va_start = io_base;
    while(size){
        map_page_helper(io_base, phys_addr, pa2kva(PGDIR_PA));
        io_base += PAGE_SIZE;
        phys_addr+= PAGE_SIZE;
        size -=PAGE_SIZE;
    }
    local_flush_tlb_all();
    return va_start;
}

void iounmap(void *io_addr)
{
    // TODO: [p5-task1] a very naive iounmap() is OK
    // maybe no one would call this function?
    PTE* pte = get_pteptr_of(io_addr, current_running[cpu_id]->pgdir);
    if(pte!=0)
        *pte = 0;
}
