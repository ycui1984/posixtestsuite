/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 *
 * MPR An implementation may permit accesses other than those specified by prot; 
 * however, if the Memory Protection option is supported, the implementation 
 * shall not permit a write to succeed where PROT_WRITE has not been set or 
 * shall not permit any access where PROT_NONE alone has been set. 
 * The implementation shall support at least the following values of prot:
 * PROT_NONE, PROT_READ, PROT_WRITE, and the bitwise-inclusive OR of PROT_READ and
 * PROT_WRITE. 
 * 
 * Test Step:
 * If Memory Protection option is suppored:
 * 1. Spawn a child process.
 * 2. The child process mmap a memory region setting prot as PROT_NONE.
 * 3. Try to write the mapped memory. 
 * 4. If the write will triger SIGSEGV, the PASS. 
 *
 * Please refer to IEEE_1003.1-2001. 2.8.3.3 Memory Protection.
 */


#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include "posixtest.h"
 
#define TNAME "mmap/6-3.c"

int main()
{

#ifdef _POSIX_MEMORY_PROTECTION
  char tmpfname[256];
  int total_size = 1024; 
  
  void *pa = NULL; 
  void *addr = NULL;
  size_t size = total_size;
  int flag = MAP_SHARED;
  int fd;
  off_t off = 0;
  int prot;

  char * ch;

  pid_t child;
  int status;
  int sig_num;

  snprintf(tmpfname, sizeof(tmpfname), "/tmp/pts_mmap_6_3_%d",
           getpid());
  unlink(tmpfname);
  fd = open(tmpfname, O_CREAT | O_RDWR | O_EXCL,
            S_IRUSR | S_IWUSR);
  if (fd == -1)
  {  
    printf(TNAME " Error at open(): %s\n", 
           strerror(errno));    
    exit(PTS_UNRESOLVED);
  }
  unlink(tmpfname);
  
  child = fork();

  if (child == 0)
  {
    fflush (NULL); 
    if (ftruncate(fd, total_size) == -1)
    {
      printf(TNAME "Error at ftruncate(): %s\n", 
              strerror(errno));    
      exit(PTS_UNRESOLVED);
    }
  
    prot = PROT_NONE;
    pa = mmap(addr, size, prot, flag, fd, off);
    if (pa == MAP_FAILED)
    {
      printf("Test Fail: " TNAME " Error at mmap: %s\n", 
            strerror(errno));    
      exit(PTS_FAIL);
    }
    
    ch = pa;

    /* Write acess */
    *ch = 'b';
    exit(0);
  }
  else if (child > 0)
  {
    waitpid(child, &status, 0);
    close(fd);
    if (WIFSTOPPED(status))
    {
      sig_num = WSTOPSIG(status);
      if (sig_num == SIGSEGV)
      {
        printf("Test Pass: " TNAME 
                " Got SIGSEGV when writing the mapped memory, "
                "setting PROT_NONE\n");
        return PTS_PASS;
      }
      printf("Child process stopped by signal %d\n", sig_num); 
    }
    if (WIFSIGNALED(status))
    {
      sig_num = WTERMSIG(status);
      if (sig_num == SIGSEGV)
      {
        printf ("Test Pass: " TNAME 
                " Got SIGSEGV when writing the mapped memory, "
                " setting PROT_NOTE\n");
        return PTS_PASS;
      }
      printf("Child process terminated by signal %d\n", sig_num); 
    }
    if (WIFEXITED(status))
    {
      if (WEXITSTATUS(status) == 0)
      {
        printf ("Test FAIL: " TNAME 
                " Did not got SIGSEGV when writing the mapped memory,"
                " setting PROT_NOTE\n");
        return PTS_FAIL;
      }
    
    }
    printf ("Test Unresolved\n");
    return PTS_UNRESOLVED;
  }

#else
    printf ("Test Unresolved: " TNAME 
            " _POSIX_MEMORY_PROTECTION not defined\n");    
    return PTS_UNRESOLVED;
#endif
}
