/*
 * Copyright (c) 1998 by Mike Romberg ( romberg@fsl.noaa.gov )
 * This file may be distributed under the same terms as Perl.
 *
 * This will probably only work under HPUX-10 or later.
 *
 * 8/26/99 Added "fname" field for consistency with other OS's - D. Urist
 *
 */

#include <stdlib.h>
#include <sys/param.h>
#include <sys/pstat.h>

#define BURST 30 /* How many pstat structs to get per syscall */

extern void bless_into_proc(char* format, char** fields, ...);

static char *Format = 
"llllllllllllllllsllssllllsllslllllllllllllllllllllSSSllllSSSll";

static char *Fields[] = {
"uid",
"pid",
"ppid",
"dsize",
"tsize",
"ssize",
"nice",
"ttynum", /* */
"pgrp",
"pri",
"addr",
"cpu",
"utime",
"stime",
"start",
"flag",
"state",
"wchan",
"procnum",
"cmd",
"fname", 
"time",
"cpticks",
"cptickstotal",
"fss",
"pctcpu",
"rssize",
"suid",
"ucomm", /* char */
"shmsize",
"mmsize",
"usize",
"iosize",
"vtsize",
"vdsize",
"vssize",
"vshmsize",
"vmmsize",
"vusize",
"viosize",
"minorfaults", /* ulong */
"majorfaults", /* ulong */
"nswap", /* ulong */
"nsignals", /* ulong */
"msgrcv", /* ulong */
"msgsnd", /* ulong */
"maxrss",
"sid",
"schedpolicy",
"ticksleft",
"rdir", /* */
"cdir", /* */
"text", /* */
"highestfd",
"euid",
"egid",
"ioch",
"usercycles", /* */
"systemcycles", /* */
"interruptcycles", /* */
"gid",
"lwpid"
};

static char *States[] = { 
"", "sleep", "run", "stop", "zombie", "uwait", "other" 
};

char* OS_initialize()
    {
    return NULL;
    }

void OS_get_table()
    {
    struct pst_status pst[BURST];

    int i, count;
    int idx = 0;
    char buff[256]; /* used to format %cpu which is the only float. */

    while ((count = pstat_getproc(pst, sizeof(pst[0]), BURST, idx)) > 0)
        {
        for (i = 0; i < count; i++)
            {
            sprintf(buff, "%f", pst[i].pst_pctcpu * 100);
            bless_into_proc(Format, Fields,
              pst[i].pst_uid,
              pst[i].pst_pid,
              pst[i].pst_ppid,
              pst[i].pst_dsize,
              pst[i].pst_tsize,
              pst[i].pst_ssize,
              pst[i].pst_nice,
              makedev(pst[i].pst_term.psd_major, pst[i].pst_term.psd_minor),
              pst[i].pst_pgrp,
              pst[i].pst_pri,
              pst[i].pst_addr,
              pst[i].pst_cpu,
              pst[i].pst_utime,
              pst[i].pst_stime,
              pst[i].pst_start,
              pst[i].pst_flag,
              States[pst[i].pst_stat],
              pst[i].pst_wchan,
              pst[i].pst_procnum,
              pst[i].pst_cmd,
              pst[i].pst_cmd,
              pst[i].pst_cptickstotal/100,
              pst[i].pst_cpticks,
              pst[i].pst_cptickstotal,
              pst[i].pst_fss,
              buff,
              pst[i].pst_rssize,
              pst[i].pst_suid,
              pst[i].pst_ucomm,
              pst[i].pst_shmsize,
              pst[i].pst_mmsize,
              pst[i].pst_usize,
              pst[i].pst_iosize,
              pst[i].pst_vtsize,
              pst[i].pst_vdsize,
              pst[i].pst_vssize,
              pst[i].pst_vshmsize,
              pst[i].pst_vmmsize,
              pst[i].pst_vusize,
              pst[i].pst_viosize,
              pst[i].pst_minorfaults,
              pst[i].pst_majorfaults,
              pst[i].pst_nswap,
              pst[i].pst_nsignals,
              pst[i].pst_msgrcv,
              pst[i].pst_msgsnd,
              pst[i].pst_maxrss,
              pst[i].pst_sid,
              pst[i].pst_schedpolicy,
              pst[i].pst_ticksleft,
              "",
              "",
              "",
              pst[i].pst_highestfd,
              pst[i].pst_euid,
              pst[i].pst_egid,
              pst[i].pst_ioch,
              "",
              "",
              "",
              pst[i].pst_gid,
              pst[i].pst_lwpid);
            }
        idx = pst[count-1].pst_idx + 1;
        }
    }


