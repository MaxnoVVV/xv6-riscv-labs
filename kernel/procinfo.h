
struct procinfo {
  int pid;
  int parent_pid;
  char name[16];
  int state;
  char parent_name[16];
};