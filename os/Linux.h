#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <sys/types.h>
#include <sys/param.h>
#include <linux/limits.h>

/****************************************/
/* Process state strings that we return */
/****************************************/
#define SLEEP  "sleep"
#define WAIT   "wait"       
#define RUN    "run"
#define IDLE   "idle"
#define ZOMBIE "defunct"
#define STOP   "stop"
#define UWAIT  "uwait"      

/***********************************************************/
/* Some global variables we only need to get once          */
/***********************************************************/
long Btime;
unsigned Sysmem;


/***********************************************************/
/* stuff stored in /proc/xxx/stat it would be nice if this */
/* was in a system header file somewhere                   */
/***********************************************************/
struct procstat {
  int pid;
  char comm[FILENAME_MAX];
  char state;
  int ppid;
  int pgrp;
  int session;
  int tty;
  int tpgid;
  unsigned flags;
  unsigned minflt;
  unsigned cminflt;
  unsigned majflt;
  unsigned cmajflt;
  int utime;
  int stime;
  int cutime;
  int cstime;
  int counter;
  int priority;
  unsigned timeout;
  unsigned itrealvalue;
  int starttime;
  unsigned vsize;
  unsigned rss;
  unsigned rlim;
  unsigned startcode;
  unsigned endcode;
  unsigned startstack;
  unsigned kstkesp;
  unsigned kstkeip;
  int signal;
  int blocked;
  int sigignore;
  int sigcatch;
  unsigned wchan;
};

/* We need to pass in a cap for ignore, lower for store on object */
/* We can just lc these! */
static char Defaultformat[] = "IIIIIIIIIIIIIIIIIIIISLSSSS";

/* Mapping of field to type */
static char* Fields[] = {
  "uid",
#define F_UID 0   /* C is the stuuuuuupidest fucking language ... */

  "gid",
#define F_GID 1

  "pid",
#define F_PID 2

  "ppid",
#define F_PPID 3

  "pgrp",
#define F_PGRP 4

  "sess",
#define F_SESS 5

  "priority",
#define F_PRIORITY 6

  "ttynum",
#define F_TTYNUM 7

  "flags",
#define F_FLAGS 8

  "minflt",
#define F_MINFLT 9

  "cminflt",
#define F_CMINFLT 10

  "majflt",
#define F_MAJFLT 11

  "cmajflt",
#define F_CMAJFLT 12

  "utime",
#define F_UTIME 13

  "stime",
#define F_STIME 14

  "cutime",
#define F_CUTIME 15

  "cstime",
#define F_CSTIME 16

  "size",
#define F_SIZE 17

  "rss",
#define F_RSS 18

  "wchan",
#define F_WCHAN 19

  "fname",
#define F_FNAME 20

  "start",
#define F_START 21

  "pctcpu",
#define F_PCTCPU 22

  "state",
#define F_STATE 23

  "pctmem",
#define F_PCTMEM 24

  "cmndline"
#define F_CMNDLINE 25

#define F_LASTFIELD 25
};
