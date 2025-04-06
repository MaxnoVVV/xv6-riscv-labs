#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include <stddef.h>

//returns index of delimiter or -1 on error
int find_delimiter(char* buf, size_t buf_size) {
  int num_of_spaces = 0;
  int index_of_delimiter = -1;
  for(int i = 0;i < buf_size;i++){
    if(buf[i] == ' '){
      num_of_spaces++;
      if(num_of_spaces > 1) {
        return -1;
      }
      index_of_delimiter = i;
    }
  }
  return index_of_delimiter;
}

int get_int(char* buf, size_t buf_size) {
  int is_negative = 0;
  if(buf[0] == '-') {
    buf++;
    buf_size--;
    is_negative = 1;
  }
  int result = atoi(buf);

  if(result == 0) {
    if(!(buf_size == 1) || !(buf[0] == '0')) {
      printf("Invalid number\n");
      return exit(1);
    }
  }

  return !is_negative ? result : 0 - result;
}

int main(int argc, char** argv) {
  char buf[512];
  int n = 0;
  int a,b;
  long sum;

  n = read(0, buf, sizeof(buf));
  if(n > 0) {
     buf[n-1] = 0;
  }
  printf("|%s|\n", buf);
  if(n < 4) {
    printf("%s", "Usage: sum_a <int64 number> <int64 number>\n");
    return exit(1);
  }

  if(n == 512 && buf[511] != '\0') {
    printf("%s", "Input is too large");
  }

  int delimiter_idx = find_delimiter(buf, sizeof(buf));
  if(delimiter_idx == -1){
    printf("%s", "Usage: sum_a <int64 number> <int64 number>\n");
    return exit(1);
  }
  buf[delimiter_idx] = '\0';
  a = get_int(buf, delimiter_idx);
  b = get_int(buf+delimiter_idx+1, n - delimiter_idx - 2);

  sum = (long)a + b;
  printf("The sum is %ld\n", sum);
  exit(0);
}


