
#include "os/Linux.h"

/* Given a path to a /proc/XXX/stat file and a pointer to a procstat
   struct, fill the struct */
struct procstat* get_procstat( char* path, struct procstat* prs){
  FILE* fp;

  if( (fp = fopen( path, "r" )) != NULL ){ 
    fscanf(fp, 
	   "%d %s %c %d %d %d %d %d %u %u %u %u %u %d %d %d %d %d %d %u %u %d %u %u %u %u %u %u %u %u %d %d %d %d %u",
	   &prs->pid, 
	   &prs->comm, 
	   &prs->state, 
	   &prs->ppid, 
	   &prs->pgrp, 
	   &prs->session, 
	   &prs->tty, 
	   &prs->tpgid,
	   &prs->flags,
	   &prs->minflt,
	   &prs->cminflt,
	   &prs->majflt, 
	   &prs->cmajflt, 
	   &prs->utime,
	   &prs->stime,
	   &prs->cutime,
	   &prs->cstime,
	   &prs->counter,
	   &prs->priority,
	   &prs->timeout,
	   &prs->itrealvalue,
	   &prs->starttime,
	   &prs->vsize,
	   &prs->rss,
	   &prs->rlim,
	   &prs->startcode,
	   &prs->endcode,
	   &prs->startstack,
	   &prs->kstkesp,
	   &prs->kstkeip,
	   &prs->signal,
	   &prs->blocked,
	   &prs->sigignore,
	   &prs->sigcatch,
	   &prs->wchan
	   );
    fclose(fp);
    return(prs);
  }
  else{
    return(NULL);
  }
}

/* Make sure /proc is mounted and initialize some global vars */
char* OS_initialize(){
  char cbuf[1024]; 
  char bbuf[32]; 
  FILE* fp;
  struct statfs sfs;

  static char* no_proc = "/proc unavailable";
  if( statfs("/proc", &sfs) == -1 ){
    return no_proc;
  }

  /* Get boottime from /proc/stat */
  if( (fp = fopen( "/proc/stat", "r" )) != NULL ){ 
    while( !feof(fp) ){
      fgets( cbuf, 1024, fp);
      if(sscanf(cbuf,"btime %s", bbuf)){
	Btime = atol(bbuf);
	break;
      }
    }
    fclose(fp);
  }
  else{
    Btime = 0;
  }

  /* Get total system memory from /proc/meminfo and convert to pages */
  if( (fp = fopen( "/proc/meminfo", "r" )) != NULL ){ 
    while( !feof(fp) ){
      fgets( cbuf, 1024, fp);
      if(sscanf(cbuf,"Mem: %s", bbuf)){
	Sysmem = atol(bbuf)/getpagesize();
	break;
      }
    }
    fclose(fp);
  }
  else{
    Sysmem = 0;
  }

  return NULL;
}

void OS_get_table(){
  DIR *procdir;
  struct dirent *procdirp;
  char pathbuf[PATH_MAX];
  struct stat filestat;
  FILE* fp;

  /* for bless_into_proc */
  struct procstat prs; 
  char fname[NAME_MAX];
  long start;
  char state[20];
  char cmndline[ARG_MAX]; 
  char pctmem[7];
  char pctcpu[7];

  char format[F_LASTFIELD + 1];
  format[F_LASTFIELD + 1] = '\0';

  if( (procdir = opendir("/proc")) == NULL ){
    return;
  }

  /* Iterate through all the process entries (numeric) under /proc */
  while( (procdirp = readdir(procdir)) != NULL ){

    /* Only look at this file if it's a proc id; that is, all numbers */
    if( strtok(procdirp->d_name, "0123456789") == NULL ){
      /* zero our format */
      strcpy(format, Defaultformat);

      /* get puid and pgid from proc file */
      sprintf(pathbuf, "%s%s", "/proc/", procdirp->d_name);
      if( stat( pathbuf, &filestat ) != -1 ){
	format[F_UID] = tolower(format[F_UID]); /* uid */
	format[F_GID] = tolower(format[F_GID]); /* gid */
      }

      /* get stuff out of /proc/PROC_ID/stat */
      memset(&prs, 0, sizeof(prs));
      strcat( pathbuf, "/stat" );
      if( get_procstat(pathbuf, &prs) != NULL ){
	int i;

	/* make all these fields valid */
	for( i = F_PID; i <= F_WCHAN; i++ ){
	  format[i] = tolower(format[i]);
	}

        /* Get rid of the parens. There's probably a better way... */
	fname[0] = '\0';
        strcpy(fname, strtok(prs.comm, "()"));
	format[F_FNAME] = tolower(format[F_FNAME]); /* fname */

	/* start is in 100ths of seconds since boot; convert to unix time */
	if( Btime != 0 ){
	  start = ( prs.starttime / 100 ) + Btime;
	  format[F_START] = tolower(format[F_START]); /* start */
	}

	/* calculate pctcpu */
	pctcpu[0] = '\0';
	sprintf( pctcpu, "%3.2f", (float) ((prs.utime + prs.stime) * 100) / (( time(NULL) - start ) * 100));
	format[F_PCTCPU] =  tolower(format[F_PCTCPU]); /* pctcpu */

	/* convert character state into one of our "official" readable
           states */
	switch(prs.state)
	  {
	  case 'R':
	    strcpy(state, RUN);
	    format[F_STATE] = tolower(format[F_STATE]); /* state */
	    break;
	  case 'S':
	    strcpy(state, SLEEP);
	    format[F_STATE] = tolower(format[F_STATE]); /* state */
	    break;
	  case 'D':
	    strcpy(state, UWAIT);
	    format[F_STATE] = tolower(format[F_STATE]); /* state */
	    break;
	  case 'Z':
	    strcpy(state, ZOMBIE);
	    format[F_STATE] = tolower(format[F_STATE]); /* state */
	    break;
	  case 'T':
	    strcpy(state, STOP);
	    format[F_STATE] = tolower(format[F_STATE]); /* state */
	    break;
	  }
      }

      /* calculate pctmem */
      pctmem[0] = '\0';
      if( Sysmem != 0 ){  
	sprintf( pctmem, "%3.2f", (float) (prs.rss * 100 / Sysmem));
	format[F_PCTMEM] = tolower(format[F_PCTMEM]); /* pctmem */ 
      }

      /* get stuff out of /proc/PROC_ID/cmdline */
      cmndline[0] = '\0';
      sprintf(pathbuf, "%s%s%s", "/proc/", procdirp->d_name, "/cmdline");
      if( (fp = fopen( pathbuf, "r" )) != NULL ){ 
	int i;
	fgets(cmndline, FILENAME_MAX, fp);
	fclose(fp);
	if(strlen(cmndline)) format[F_CMNDLINE] = tolower(format[F_CMNDLINE]); /* cmndline */
      }

      /* Make sure our format is not all x's, which happens
	 when a process is killed before we can read its info 
	 and we get a blank object */
      if( strpbrk(format,"sil" ) != NULL){ 
                                           
	bless_into_proc( format,           
			 Fields,
			 filestat.st_uid,
			 filestat.st_gid,
			 
			 prs.pid,
			 prs.ppid,
			 prs.pgrp,
			 prs.session,
			 prs.priority,
			 prs.tty,
			 prs.flags,
			 prs.minflt,
			 prs.cminflt,
			 prs.majflt,
			 prs.cmajflt,
			 prs.utime,
			 prs.stime,
			 prs.cutime,
			 prs.cstime,
			 prs.vsize,
			 prs.rss,
			 prs.wchan,
			 
			 fname,
			 start,
			 pctcpu,
			 state,
			 pctmem,
			 cmndline
			 );
      }
    }
  }
  closedir(procdir);
}

