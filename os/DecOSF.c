
#include "os/DecOSF.h"

/* Make sure /proc is mounted */
char* OS_initialize()
{
  struct statvfs svfs;

  static char* no_proc = "/proc unavailable";
  if( statvfs("/proc", &svfs) == -1 )
  {
    return no_proc;
  }

  return NULL;
}


/* FIXME we should get minimum info like process ID and ownership from
   file stat-- does this work for IOCTL-proc? Does it for FS-proc? It
   does on linux... */

void OS_get_table()
{
  DIR *procdir;
  struct dirent *procdirp;
  int psdata;
  char pathbuf[MAXPATHLEN];

  /* defined in <sys/procfs.h> */
  struct prpsinfo psbuf;
  
  /* variables to hold some values for bless_into_proc */
  char state[20]; 
  char pctcpu[4];
  char pctmem[7];
  
  if( (procdir = opendir( "/proc" )) == NULL ) return;
  
  /* loop over all "files" in procdir */
  while( (procdirp = readdir(procdir)) != NULL )
  {
    /* Only look at this file if it's a proc id; that is, all numbers */
    if( strtok(procdirp->d_name, "0123456789") != NULL )
    { 
      continue; 
    }
      
    /* Construct path of the form /proc/proc_number */
    strcpy( pathbuf, "/proc/"); 
    strcat( pathbuf, procdirp->d_name );
      
    if( (psdata = open( pathbuf, O_RDONLY )) == -1 ) continue;

    if( ioctl(psdata, PIOCPSINFO, &psbuf) == -1 ) continue; 

    close(psdata);

    /* translate process state, makros defined in <sys/proc.h> */
    switch( psbuf.pr_state)
    {
      case SSLEEP: 
        strcpy(state, SLEEP);
        break;
      case SWAIT:
        strcpy(state, WAIT);
        break;
      case SRUN:
        strcpy(state, RUN);
        break;
      case SZOMB:
        strcpy(state, ZOMBIE);
        break;
      case SSTOP:
        strcpy(state, STOP);
        break;
      case SIDL:
        strcpy(state, IDLE);
        break;
      }

    /* This seems to be 1 byte */
    sprintf( pctcpu,  "%3d", psbuf.pr_cpu  );

    bless_into_proc( Format,           
                     Fields,
                     
                     psbuf.pr_uid,           /* uid, uid_t is int */
                     psbuf.pr_gid,           /* gid, gid_t is int */
                     psbuf.pr_pid,           /* pid, pid_t is int */
                     psbuf.pr_ppid,          /* ppid, pid_t is int */
                     psbuf.pr_pgrp,          /* pgrp, pid_t is int */ 
                     psbuf.pr_sid,           /* sess, pid_t is int */
                     psbuf.pr_pri,           /* priority, long, */   
                     psbuf.pr_ttydev,        /* ttynum, dev_t is int */
                     psbuf.pr_flag,          /* flags, u_long */
                     psbuf.pr_time.tv_sec,   /* time, long */
                     psbuf.pr_size * getpagesize(),   /* size (bytes) */
                     psbuf.pr_rssize * getpagesize(), /* rss (bytes)  */
                     psbuf.pr_start.tv_sec,  /* start */
                     psbuf.pr_fname,         /* fname */
                     pctcpu,                 /* pctcpu */
                     state,                  /* state */
                     pctmem,                 /* pctmem */
                     psbuf.pr_psargs         /* cmndline */ 
                   );
    
  }
  closedir(procdir);
}
