#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>
#include <sys/proc.h> /* needed for process state constants */

#if defined(PROC_FS)
#include <procfs.h>
#else
#include <sys/procfs.h>
#endif

#ifdef i386
#undef SP
#define SP sp
#endif

/****************************************/
/* Process state strings that we return */
/****************************************/
#define SLEEP  "sleep"
#define WAIT   "wait"       
#define RUN    "run"
#define IDLE   "idle"
#define ZOMBIE "defunct"
#define STOP   "stop"
#define ONPROC "onprocessor" 

/* Solaris is an all-or-nothing deal, all this stuff comes out of 
   one structure, so we don't need to dick around with the format much */
static char Format[] = "iiiiiiiillllllllslssss";

/* Mapping of field to type */
static char* Fields[] = {
  "uid",
  "gid",
  "euid",
  "egid",
  "pid",
  "ppid",
  "pgrp",
  "sess",
  "priority",
  "ttynum",
  "flags",
  "time",
  "ctime",
  "size",
  "rss",
  "wchan",
  "fname",
  "start",
  "pctcpu",
  "state",
  "pctmem",
  "cmndline"
};









