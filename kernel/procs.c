#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "procinfo.h"

extern struct proc proc[NPROC];
struct spinlock parent_wait_lock;

int map_to_userprocstate(enum procstate state) {
    switch(state) {
        case UNUSED: return 0;
        case USED: return 1;
        case SLEEPING: return 2;
        case RUNNABLE: return 3;
        case RUNNING: return 4;
        case ZOMBIE: return 5;
        default: return -1;
    }
}

int sys_ps_listinfo_impl (uint64 plist, int lim) {
    int counter = 0;
    int is_addr_valid;

    if(plist == 0) {
        is_addr_valid = 0;
    } else {
        is_addr_valid = 1;
    }
    for(int i = 0;i < NPROC; i++) {
        struct proc *p = &proc[i];
        acquire(&p->lock);

        if(p->state != UNUSED) {
            counter++;
            if(!is_addr_valid) {
                release(&p->lock);
                continue;
            }
            if(counter > lim) {
                release(&p->lock);
                return -1;
            }
            struct procinfo pinfo;
            pinfo.pid = p->pid;
            pinfo.state = map_to_userprocstate(p->state);
            acquire(&parent_wait_lock);

            if(p->parent) {
                pinfo.parent_pid = p->parent->pid;
                safestrcpy(pinfo.parent_name, p->parent->name, 16);
            }
            release(&parent_wait_lock);

            safestrcpy(pinfo.name, p->name, 16);
            if(copyout(myproc()->pagetable, plist + (counter-1)*(sizeof (struct procinfo)), (char *) &pinfo, sizeof(struct procinfo)) == -1) {
                release(&p->lock);
                return -2;
            }

        }
        release(&p->lock);
    }

    if(!is_addr_valid) {
        return counter;
    }

    return counter;
}