#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/proc.h"

int
main(int argc, char *argv[])
{
  static struct pstat pstat_arr[NPROC_STAT];
  
  // Call getpinfo to get process information
  if(getpinfo(pstat_arr) < 0) {
    fprintf(2, "ps: getpinfo failed\n");
    exit(1);
  }
  
  // Print header
  printf("PID\tPPID\tPriority\tState\t\tName\n");
  
  // Print process information
  for(int i = 0; i < NPROC_STAT; i++) {
    if(pstat_arr[i].pid != 0) {
      const char *state_str;
      switch(pstat_arr[i].state) {
        case 0: state_str = "UNUSED"; break;
        case 1: state_str = "USED"; break;
        case 2: state_str = "SLEEPING"; break;
        case 3: state_str = "RUNNABLE"; break;
        case 4: state_str = "RUNNING"; break;
        case 5: state_str = "ZOMBIE"; break;
        default: state_str = "UNKNOWN"; break;
      }
      
      printf("%d\t%d\t%d\t\t%s\t\t%s\n",
             pstat_arr[i].pid,
             pstat_arr[i].ppid,
             pstat_arr[i].priority,
             state_str,
             pstat_arr[i].name);
    }
  }
  
  exit(0);
}
