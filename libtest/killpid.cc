/*  vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * 
 *  libtest
 *
 *  Copyright (C) 2011 Data Differential, http://datadifferential.com/
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 3 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */


#include <libtest/common.h>

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <signal.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/wait.h>


#include <libtest/killpid.h>
#include <libtest/stream.h>

using namespace libtest;

bool kill_pid(pid_t pid_arg)
{
  assert(pid_arg > 0);
  if (pid_arg < 1)
  {
    Error << "Invalid pid:" << pid_arg;
    return false;
  }

  if ((::kill(pid_arg, SIGTERM) == -1))
  {
    switch (errno)
    {
    case EPERM:
      Error << "Does someone else have a process running locally for " << int(pid_arg) << "?";
      return false;

    case ESRCH:
      Error << "Process " << int(pid_arg) << " not found.";
      return false;

    default:
    case EINVAL:
      Error << "kill() " << strerror(errno);
      return false;
    }
  }

  int status= 0;
  if (waitpid(pid_arg, &status, 0) == -1)
  {
    switch (errno)
    {
      // Just means that the server has already gone away
    case ECHILD:
      {
        return true;
      }
    }

    Error << "Error occured while waitpid(" << strerror(errno) << ") on pid " << int(pid_arg);

    return false;
  }

  return true;
}


pid_t kill_file(const std::string &filename)
{
  pid_t ret= -1;
  FILE *fp;

  if (filename.empty())
    return ret;

  if ((fp= fopen(filename.c_str(), "r")))
  {
    char pid_buffer[1024];

    char *ptr= fgets(pid_buffer, sizeof(pid_buffer), fp);
    fclose(fp);

    if (ptr)
    {
      pid_t pid= (pid_t)atoi(pid_buffer);
      if (pid != 0)
      {
        kill_pid(pid);
        unlink(filename.c_str()); // If this happens we may be dealing with a dead server that left its pid file.
      }
    }
  }
  
  return ret;
}

pid_t get_pid_from_file(const std::string &filename)
{
  pid_t ret= -1;
  FILE *fp;

  if (filename.empty())
  {
    Error << "empty pid file";
    return ret;
  }

  if ((fp= fopen(filename.c_str(), "r")))
  {
    char pid_buffer[1024];

    char *ptr= fgets(pid_buffer, sizeof(pid_buffer), fp);
    fclose(fp);

    if (ptr)
    {
      ret= (pid_t)atoi(pid_buffer);
      if (ret <= 0)
      {
        return ret;
      }
    }
  }
  
  return ret;
}
