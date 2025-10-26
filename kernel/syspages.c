#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "pageinfo.h"

struct page_analyze_ctx {
  int action;
  int print_table;
  int print_dirty;
  int print_access;
  int clear_dirty;
  int clear_access;
  int include_all;
  uint64 start;
  uint64 end;
  int modified;
};

static uint64
level_span(int level)
{
  return 1ULL << (PGSHIFT + level * 9);
}

static void
format_pte_flags(pte_t pte, char out[8])
{
  out[0] = (pte & PTE_R) ? 'R' : '_';
  out[1] = (pte & PTE_W) ? 'W' : '_';
  out[2] = (pte & PTE_X) ? 'X' : '_';
  out[3] = (pte & PTE_U) ? 'U' : '_';
  out[4] = (pte & PTE_G) ? 'G' : '_';
  out[5] = (pte & PTE_A) ? 'A' : '_';
  out[6] = (pte & PTE_D) ? 'D' : '_';
  out[7] = '\0';
}

static void
print_indent(int depth)
{
  for(int i = 0; i < depth * 9; i++)
    printf(".");
}

static void
analyze_walk(pagetable_t pagetable, int level, uint64 base_va, int depth,
             struct page_analyze_ctx *ctx)
{
  for(int idx = 0; idx < 512; idx++){
    pte_t *pte_ptr = &pagetable[idx];
    pte_t pte = *pte_ptr;

    if((pte & PTE_V) == 0)
      continue;

    uint64 entry_va = base_va | ((uint64)idx << PXSHIFT(level));
    uint64 span = level_span(level);

    if(!ctx->include_all){
      if(entry_va + span <= ctx->start || entry_va >= ctx->end)
        continue;
    }


    if(ctx->print_table) {
      if((!ctx->print_access && !ctx->print_dirty) || ((!ctx->print_access || (pte & PTE_A)) && (!ctx->print_dirty || (pte & PTE_D)))) {
      char flags[8];
      format_pte_flags(pte, flags);
      print_indent(depth);
      printf("0x%x -> 0x%lx %s\n", idx, PTE2PA(pte), flags);
      }
    }

    if(level == 0){
      if(ctx->clear_dirty)
        pagetable[idx] &= ~PTE_D;
      if(ctx->clear_access)
        pagetable[idx]&= ~PTE_A;
    } else {
      analyze_walk((pagetable_t)PTE2PA(pte), level - 1, entry_va, depth + 1, ctx);
    }
  }
}

int
analyze_proc_pagetable(struct proc *p, uint64 addr, uint64 len, int action)
{
  if(action <= 0 || (action & ~PAGEINFO_VALID_MASK))
    return -1;

  struct page_analyze_ctx ctx;
  memset(&ctx, 0, sizeof(ctx));
  ctx.action = action;
  ctx.print_table = (action & PAGEINFO_PRINT_TABLE) != 0;
  ctx.print_dirty = (action & PAGEINFO_SHOW_DIRTY) != 0;
  ctx.print_access = (action & PAGEINFO_SHOW_ACCESSED) != 0;
  ctx.clear_dirty = (action & PAGEINFO_CLEAR_DIRTY) != 0;
  ctx.clear_access = (action & PAGEINFO_CLEAR_ACCESSED) != 0;
  ctx.include_all = (addr == 0 || len == 0);

  if(ctx.include_all == 0){
    uint64 end = addr + len;
    if(end < addr)
      return -1;
    ctx.start = PGROUNDDOWN(addr);
    ctx.end = PGROUNDUP(end);
  }

  if(ctx.include_all){
    ctx.start = 0;
    ctx.end = 0;
  }

  if(ctx.print_table)
    printf("PAGETABLE 0x%lx\n", (uint64)p->pagetable);

  analyze_walk(p->pagetable, 2, 0, 0, &ctx);

  return 0;
}


uint64
sys_pageinfo(void)
{
  uint64 addr, len;
  int action;

  argaddr(0, &addr);
  argaddr(1, &len);
  argint(2, &action);

  if(analyze_proc_pagetable(myproc(), addr, len, action) < 0)
    return -1;
  return 0;
}

