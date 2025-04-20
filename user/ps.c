//
// Created by maxim on 19.04.2025.
//
#include "kernel/param.h"
#include "kernel/types.h"
#include "user/user.h"
#include "kernel/procinfo.h"


const char* get_state_name(int state) {
  switch(state) {
    case 0: return "UNUSED";
    case 1: return "USED";
    case 2: return "SLEEPING";
    case 3: return "RUNNABLE";
    case 4: return "RUNNING";
    case 5: return "ZOMBIE";
    default: return "UNKNOWN";
  }
}

int main() {
   int k = 1;

   while(1) {
     struct procinfo proc[k];
     int result = ps_listinfo(proc, k);
     if(result == -1) {
       k *= 2;
       continue;
     } else if(result == -2) {
       printf("%s\n", "Error, user space memory unavailable");
       exit(-1);
     } else {
       printf("%s\t\t%s\t\t%s\t\t%s\t\t%s\n", "PID", "NAME", "STATE", "PPID", "PNAME");
       for(int i = 0;i < result; i++) {
         if(strlen(get_state_name(proc[i].state)) >= 8) {
           printf("%d\t\t%s\t\t%s\t%d\t\t%s\n", proc[i].pid, proc[i].name, get_state_name(proc[i].state), proc[i].parent_pid, proc[i].parent_name);
         } else {
           printf("%d\t\t%s\t\t%s\t\t%d\t\t%s\n", proc[i].pid, proc[i].name, get_state_name(proc[i].state), proc[i].parent_pid, proc[i].parent_name);
         }
       }
       break;
     }
   }
   exit(0);
}

