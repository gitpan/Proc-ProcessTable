
#include "os/IRIX.h"

/* Make sure /proc is mounted */
char* OS_initialize(){
  struct statvfs svfs;

  static char* no_proc = "/proc unavailable";
  if( statvfs("/proc", &svfs) == -1 ){
    return no_proc;
  }

  return NULL;
}


/* FIXME we should get minimum info like process ID and ownership from
   file stat-- does this work for IOCTL-proc? Does it for FS-proc? It
   does on linux... */

void OS_get_table(){
  DIR *procdir;
  struct dirent *procdirp;
  int psdata;
  int pagesize;
  char pathbuf[MAXPATHLEN];

  struct prpsinfo psbuf;
  struct rminfo rminfo;
  
  /* variables to hold some values for bless_into_proc */
  char state[20]; 
  char pctcpu[7];
  char pctmem[7];

  if ( (pagesize = getpagesize()) == NULL ) return;
    
  if( (procdir = opendir( "/proc" )) == NULL ) return;

  sysmp(MP_SAGET, MPSA_RMINFO, &rminfo, sizeof(rminfo));
  
  while( (procdirp = readdir(procdir)) != NULL ){
    
    /* Only look at this file if it's a proc id; that is, all numbers */
    if( strtok(procdirp->d_name, "0123456789") != NULL ){ continue; }
      
    /* Construct path of the form /proc/proc_number */
    strcpy( pathbuf, "/proc/"); 
    strcat( pathbuf, procdirp->d_name );
      
    if( (psdata = open( pathbuf, O_RDONLY )) == -1 ) continue;
	
    if( ioctl(psdata, PIOCPSINFO, &psbuf) == -1 ) continue; 

    close(psdata);

    /* translate process state */
    switch( psbuf.pr_state)
      {
      case SSLEEP: 
	strcpy(state, SLEEP);
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
      case SXBRK:
	strcpy(state, XBRK);
	break;
      }

    /* These seem to be 2 bytes, 1st byte int part, 2nd byte fractional */
    /* Perl can handle stringy numbers of the form 1.5 */

    sprintf( pctcpu, "%3.2f", 
	     (float) ((psbuf.pr_time.tv_sec) * 100) / (( time(NULL) - psbuf.pr_start.tv_sec ) * 100));

    sprintf( pctmem, "%5.2f", 
	     ((double)psbuf.pr_size)/rminfo.physmem*100);

    bless_into_proc( Format,           
		     Fields,
		     
		     psbuf.pr_uid,           /* uid */
		     psbuf.pr_gid,           /* gid */
		     psbuf.pr_pid,           /* pid */
		     psbuf.pr_ppid,          /* ppid */
		     psbuf.pr_spid,          /* spid */
		     psbuf.pr_pgrp,          /* pgrp */ 
		     psbuf.pr_sid,           /* sess */
		     psbuf.pr_sonproc,       /* cpuid */

		     psbuf.pr_pri,           /* priority */   
		     psbuf.pr_ttydev,        /* ttynum */
		     psbuf.pr_flag,          /* flags */
		     psbuf.pr_time.tv_sec,   /* time */
		     psbuf.pr_ctime.tv_sec,  /* ctime */
		     psbuf.pr_qtime.tv_sec,  /* qtime */
		     psbuf.pr_size * pagesize,   /* size (bytes) */
		     psbuf.pr_rssize * pagesize, /* rss (bytes)  */
		     psbuf.pr_wchan,         /* wchan */

		     psbuf.pr_fname,         /* fname */

		     psbuf.pr_start.tv_sec,  /* start */

		     pctcpu,                 /* pctcpu */
		     state,                  /* state */
		     pctmem,                 /* pctmem */
		     psbuf.pr_psargs,        /* cmndline */ 
		     psbuf.pr_clname         /* clname */
		    );
    
  }
  closedir(procdir);
}
