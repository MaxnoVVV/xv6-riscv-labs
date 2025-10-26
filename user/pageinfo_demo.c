#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/riscv.h"
#include "kernel/pageinfo.h"
#include "user/user.h"

int global_value = 123;
char global_buffer[64];

static void
show_table(const char *label)
{
  printf("\n-- %s --\n", label);
  if(pageinfo(0, 0, PAGEINFO_PRINT_TABLE) < 0){
    printf("pageinfo failed\n");
  }
}

int
main(void)
{
  printf("=== pageinfo demo ===\n");

  show_table("initial layout");

  char stack_value = 7;
  char stack_buffer[128];

  stack_value = 8;
  stack_buffer[0] = 1; 
  global_buffer[0] = 3;
  global_value = 456;

  //STACK VALUE
  printf("=== stack value after edit ===\n");
  pageinfo((uint64)&stack_value, sizeof(stack_value), PAGEINFO_PRINT_TABLE);

  pageinfo((uint64)&stack_value, sizeof(stack_value), PAGEINFO_CLEAR_ACCESSED);
  printf("=== stack value after clear accessed ===\n");
  pageinfo((uint64)&stack_value, sizeof(stack_value), PAGEINFO_PRINT_TABLE);

  //GLOBAL VALUE
  printf("=== global value after edit ===\n");
  pageinfo((uint64)&global_value, sizeof(global_value), PAGEINFO_PRINT_TABLE);

  pageinfo((uint64)&global_value, sizeof(global_value), PAGEINFO_CLEAR_ACCESSED);
  printf("=== global value after clear accessed ===\n");
  pageinfo((uint64)&global_value, sizeof(global_value), PAGEINFO_PRINT_TABLE);

  //STACK BUFFER
  printf("=== stack buffer after edit ===\n");
  pageinfo((uint64)stack_buffer, 1, PAGEINFO_PRINT_TABLE);

  pageinfo((uint64)stack_buffer, 1, PAGEINFO_CLEAR_ACCESSED);
  printf("=== stack buffer after clear accessed ===\n");
  pageinfo((uint64)stack_buffer, 1, PAGEINFO_PRINT_TABLE);

  //GLOBAL BUFFER
  printf("=== global buffer after edit ===\n");
  pageinfo((uint64)global_buffer, 1, PAGEINFO_PRINT_TABLE);

  pageinfo((uint64)global_buffer, 1, PAGEINFO_CLEAR_ACCESSED);
  printf("=== global buffer after clear accessed ===\n");
  pageinfo((uint64)global_buffer, 1, PAGEINFO_PRINT_TABLE);


  char *heap_buf = malloc(PGSIZE * 3);
  if(heap_buf == 0){
    printf("malloc failed\n");
    exit(1);
  }

  show_table("after heap allocation");

  for(int i = 0; i < (PGSIZE * 3); i++)
    heap_buf[i] = 0;

  printf("=== heap buffer after edit ===\n");
  pageinfo((uint64)heap_buf, PGSIZE * 3, PAGEINFO_PRINT_TABLE);

  if(pageinfo(0, 0, PAGEINFO_CLEAR_ACCESSED | PAGEINFO_CLEAR_DIRTY) < 0){
    printf("clear flags failed\n");
    free(heap_buf);
    exit(1);
  }

  printf("=== heap buffer after clear accessed ===\n");
  pageinfo((uint64)heap_buf, PGSIZE * 3, PAGEINFO_PRINT_TABLE);

  free(heap_buf);

  show_table("after free");

  exit(0);
}
