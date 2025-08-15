/******************************************************************
 *
 * Round for C
 *
 * Copyright (C) Satoshi Konno 2015
 *
 * This is licensed under BSD-style license, see file COPYING.
 *
 ******************************************************************/

#if !defined(WIN32)
#include <signal.h>
#endif

#include <string.h>
#include <round/util/thread.h>
#include <round/util/timer.h>

static void round_sig_handler(int sign);

/****************************************
* Thread Function
****************************************/

static int RoundExecThreadAction(RoundThread* thread)
{
  if (!thread->action)
    return 0;
  
  if (round_thread_isstarttimeenabled(thread)) {
    double sleepTime = round_thread_getstarttime(thread) - round_getcurrentunixtime();
    round_sleep(sleepTime);
  }

  double actionStartTime, actionExecTime;
  
  actionStartTime = round_getcurrentunixtime();
  thread->action(thread);
  actionExecTime = round_getcurrentunixtime() - actionStartTime;
  
  while (round_thread_isloop(thread) && round_thread_isrunnable(thread)) {
    
    if (round_thread_isstoptimeenabled(thread)) {
      if (round_thread_getstoptime(thread) < round_getcurrentunixtime())
        break;
    }
    
    round_sleep(round_thread_getcycleinterval(thread) - actionExecTime);
    
    actionStartTime = round_getcurrentunixtime();
    thread->action(thread);
    actionExecTime = round_getcurrentunixtime() - actionStartTime;
  }
  
  return 0;
}

#if defined(WIN32)
static DWORD WINAPI RoundWin32ThreadProc(LPVOID lpParam)
{
  RoundThread* thread = (RoundThread*)lpParam;

  thread->runningFlag = true;
  RoundExecThreadAction(thread);
  thread->runningFlag = false;
  
  return 0;
}
#else
static void* RoundPosixThreadProc(void* param)
{
  sigset_t set;
  struct sigaction actions;
  RoundThread* thread = (RoundThread*)param;

  sigfillset(&set);
  sigdelset(&set, SIGQUIT);
  pthread_sigmask(SIG_SETMASK, &set, NULL);

  memset(&actions, 0, sizeof(actions));
  sigemptyset(&actions.sa_mask);
  actions.sa_flags = 0;
  actions.sa_handler = round_sig_handler;
  sigaction(SIGQUIT, &actions, NULL);

  RoundExecThreadAction(thread);
  thread->runningFlag = false;
  
  return 0;
}
#endif

/****************************************
 * round_thread_init
 ****************************************/

bool round_thread_init(RoundThread* thread)
{
  round_list_node_init((RoundList*)thread);
  
  thread->runnableFlag = false;
  thread->action = NULL;
  thread->userData = NULL;
  
  round_thread_setloop(thread, false);
  round_thread_setcycleinterval(thread, 0.0);
  round_thread_setstarttime(thread, 0.0);
  round_thread_setstoptime(thread, 0.0);
  
  thread->name = round_string_new();
  if (!thread->name)
    return false;
  
  return true;
}

/****************************************
* round_thread_new
****************************************/

RoundThread* round_thread_new(void)
{
  RoundThread* thread;

  thread = (RoundThread*)malloc(sizeof(RoundThread));

  if (!thread)
    return NULL;

  if (!round_thread_init(thread)) {
    round_thread_delete(thread);
    return NULL;
  }

  return thread;
}

/****************************************
* round_thread_delete
****************************************/

bool round_thread_delete(RoundThread* thread)
{
  if (!thread)
    return false;

  if (thread->runnableFlag) {
    round_thread_stop(thread);
  }

  round_thread_remove(thread);

  free(thread);

  return true;
}

/****************************************
* round_thread_start
****************************************/

bool round_thread_start(RoundThread* thread)
{
  if (!thread)
    return false;

  thread->runnableFlag = true;
  thread->runningFlag = true;

#if defined(WIN32)
  thread->hThread = CreateThread(NULL, 0, RoundWin32ThreadProc, (LPVOID)thread, 0, &thread->threadID);
#else
  pthread_attr_t thread_attr;
  if (pthread_attr_init(&thread_attr) != 0) {
    thread->runnableFlag = false;
    return false;
  }

  if (pthread_attr_setdetachstate(&thread_attr, PTHREAD_CREATE_DETACHED) != 0) {
    thread->runnableFlag = false;
    pthread_attr_destroy(&thread_attr);
    return false;
  }

  if (pthread_create(&thread->pThread, &thread_attr, RoundPosixThreadProc, thread) != 0) {
    thread->runnableFlag = false;
    pthread_attr_destroy(&thread_attr);
    return false;
  }
  pthread_attr_destroy(&thread_attr);
#endif

  return true;
}

/****************************************
* round_thread_stop
****************************************/

bool round_thread_stop(RoundThread* thread)
{
  if (!thread)
    return false;

  if (thread->runningFlag) {
    thread->runnableFlag = false;
#if defined(WIN32)
    TerminateThread(thread->hThread, 0);
    WaitForSingleObject(thread->hThread, INFINITE);
#else
    pthread_kill(thread->pThread, 0);
    /* TODO : Fix to use pthread_join */
    /* Now we wait one second for thread termination instead of using pthread_join */
    round_sleep(ROUND_THREAD_MIN_SLEEP);
#endif
  }

  thread->runnableFlag = false;
  thread->runningFlag = false;
  
  return true;
}

/****************************************
* round_thread_restart
****************************************/

bool round_thread_restart(RoundThread* thread)
{
  round_thread_stop(thread);
  return round_thread_start(thread);
}

/****************************************
* round_thread_isrunnable
****************************************/

bool round_thread_isrunnable(RoundThread* thread)
{
  if (!thread)
    return false;

#if !defined(WIN32)
  pthread_testcancel();
#endif

  return thread->runnableFlag;
}

/****************************************
 * round_thread_isrunning
 ****************************************/

bool round_thread_isrunning(RoundThread* thread)
{
  if (!thread)
    return false;

  return thread->runningFlag;
}

/****************************************
* round_thread_setaction
****************************************/

void round_thread_setaction(RoundThread* thread, RoundThreadFunc func)
{
  if (!thread)
    return;

  thread->action = func;
}

/****************************************
* round_thread_setuserdata
****************************************/

void round_thread_setuserdata(RoundThread* thread, void* value)
{
  if (!thread)
    return;

  thread->userData = value;
}

/****************************************
* round_thread_getuserdata
****************************************/

void* round_thread_getuserdata(RoundThread* thread)
{
  if (!thread)
    return NULL;

  return thread->userData;
}

/****************************************
 * round_sig_handler
 ****************************************/

static void round_sig_handler(int sign) {}
