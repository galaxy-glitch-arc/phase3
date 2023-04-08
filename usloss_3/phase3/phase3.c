#include <usloss.h>
#include <phase1.h>
#include <phase2.h>
#include <phase3.h>
#include <usyscall.h>
#include <provided_prototypes.h>
#include <stdlib.h>
#include <libuser.h>
#include <string.h>
#include <stdio.h>
#include "./list.h"

#define IN_USE 0
#define EMPTY 1
#define FREEING 2

void sys_vec_handler(sysargs *sa);
int start2(char *);
int spawn_real(char *name, int (*func)(char *), char *arg,
               int stack_size, int priority);
int wait_real(int *status);

extern int start3(char *arg);

typedef struct _proc
{
  char name[MAXNAME];
  int status;
  int start_mbox; // used for initialization coordian
  int mutex_mbox; //????
  int zero_mbox;  //???
  int (*start_func)(char *);
  char *start_func_args; // doesn't seem to be used
  struct _proc *parent_proc;
  list children_procs;

} proc;

typedef struct _sem
{
  int handle;
  int status;
  int value;
  int priv_mbox; // from teacher notes
  int free_mbox; // from teacher notes
  list waiting_procs;
} sem;

typedef proc *proc_ptr;
typedef sem *sem_ptr;

int _sem_idx = 0;
proc _proc_table[MAXPROC];
sem _sem_table[MAXSEMS];
int _crit_mbox;

int start2(char *arg)
{
  // int pid;
  int status;
  _crit_mbox = MboxCreate(0, 0);
  /*
   * Check kernel mode here.
   */

  /*
   * Data structure initialization as needed...
   */
  // init procs
  for (int i = 0; i < MAXPROC; ++i)
  {
    _proc_table[i].status = EMPTY;
    _proc_table[i].start_mbox = MboxCreate(1, sizeof(int));
    _proc_table[i].zero_mbox = MboxCreate(0, sizeof(int));  // use for signalling
    _proc_table[i].mutex_mbox = MboxCreate(1, sizeof(int)); // use for mutex
    list_init(&_proc_table[i].children_procs);
  }

  // init sems
  for (int i = 0; i < MAXSEMS; ++i)
  {
    _sem_table[i].status = EMPTY;
    _sem_table[i].priv_mbox = MboxCreate(0, sizeof(int));
    _sem_table[i].free_mbox = MboxCreate(0, sizeof(int));
    list_init(&_sem_table[i].waiting_procs);
  }

  // hook up syscalls
  for (int i = 0; i < MAXSYSCALLS; ++i)
  {
    sys_vec[i] = sys_vec_handler;
  }

  /*
   * Create first user-level process and wait for it to finish.
   * These are lower-case because they are not system calls;
   * system calls cannot be invoked from kernel mode.
   * Assumes kernel-mode versions of the system calls
   * with lower-case names.  I.e., Spawn is the user-mode function
   * called by the test cases; spawn is the kernel-mode function that
   * is called by the syscall_handler; spawn_real is the function that
   * contains the implementation and is called by spawn.
   *
   * Spawn() is in libuser.c.  It invokes usyscall()
   * The system call handler calls a function named spawn() -- note lower
   * case -- that extracts the arguments from the sysargs pointer, and
   * checks them for possible errors.  This function then calls spawn_real().
   *
   * Here, we only call spawn_real(), since we are already in kernel mode.
   *
   * spawn_real() will create the process by using a call to fork1 to
   * create a process executing the code in spawn_launch().  spawn_real()
   * and spawn_launch() then coordinate the completion of the phase 3
   * process table entries needed for the new process.  spawn_real() will
   * return to the original caller of Spawn, while spawn_launch() will
   * begin executing the function passed to Spawn. spawn_launch() will
   * need to switch to user-mode before allowing user code to execute.
   * spawn_real() will return to spawn(), which will put the return
   * values back into the sysargs pointer, switch to user-mode, and
   * return to the user code that called Spawn.
   */

  /*
    Start3
    must run in user mode
     must have Priority 3
     must have Stack Size  = 4 * USLOSS_MIN_STACK
  */

  spawn_real("start3", start3, NULL, 4 * USLOSS_MIN_STACK, 3);

  join(&status);

  // TEACHER examples
  //  pid = wait_real(&status);
  // Wait for start 3, then call
  // quit(0)

  return 0;
} /* start2 */

void set_user_mode()
{
  // set user mode
  int psr = psr_get();
  psr &= 0xfe;
  psr_set(psr);
}

static void nullsys3(sysargs *args_ptr)
{
  printf("nullsys3(): Invalid syscall %d\n", args_ptr->number);
  printf("nullsys3(): process %d terminating\n", getpid());
  terminate_real(1);
} /* nullsys3 */

void sys_vec_handler(sysargs *sa)
{

  // Teacher comment, most things return using arg2

  // TODO: MUST change back to user mode before existing
  // psr_set()
  int result;
  int status;
  switch (sa->number)
  {
  case SYS_SPAWN:
    result = spawn_real((char *)sa->arg5, (int (*)(char *))(sa->arg1), (char *)sa->arg2, (int)sa->arg3, (int)sa->arg4);
    sa->arg1 = (void *)result;
    break;
  case SYS_WAIT:
    // status is termination code of the child

    //-1 if no children, else it returns pid of the quiting child
    result = wait_real(&status);
    if (result < 0)
    {
      sa->arg4 = (void *)-1;
    }
    else
    {
      sa->arg1 = (void *)result; // process id of the terminating child
      sa->arg2 = (void *)status; // termination code of the child.
      sa->arg4 = (void *)0;      //- 1 if the process has no children, 0 otherwise.
    }
    break;
  case SYS_TERMINATE:
    terminate_real((int)sa->arg1);
    break;
  case SYS_SEMCREATE:
    result = semcreate_real((int)sa->arg1);
    // printf("!!%d", result);
    if (result < 0)
    {
      sa->arg4 = (void *)-1;
    }
    else
    {
      sa->arg1 = (void *)result;
      sa->arg4 = (void *)0;
    }
    break;
  case SYS_SEMP:
    result = semp_real((int)sa->arg1);
    sa->arg4 = (void *)0;
    break;
  case SYS_SEMV:
    semv_real((int)sa->arg1);
    break;
  case SYS_SEMFREE:
    result = semfree_real((int)sa->arg1);
    sa->arg4 = (void *)result;
    break;
  case SYS_GETTIMEOFDAY:
    gettimeofday_real(&status);
    sa->arg1 = (void *)status;
    break;
  case SYS_CPUTIME:
    cputime_real(&status);
    sa->arg1 = (void *)status;
    break;
  case SYS_GETPID:
    getPID_real(&status);
    sa->arg1 = (void *)status;
    break;
  default:
    nullsys3(sa);
    break;
  }
}

int spawn_launch(char *arg)
{
  int pid = getpid();
  proc_ptr proc = &_proc_table[pid % MAXPROC];

  if (is_zapped())
  {
    printf("!!terminate\n");
    terminate_real(-1);
  }

  // blocking, waiting to make sure data in proc table is ready
  MboxReceive(proc->start_mbox, NULL, 0);

  set_user_mode();

  int result = proc->start_func(arg);
  Terminate(result);

  return 0;

  /*
  TEACHER NOTES:
  //transition to usermode
  //calls start3
  int parent_location = 0;
  int my_location;
  int result;
  int (*start_func)(char *);
  // more to add if you see necc.

  my_location = getpid() % MAXPROC;

  _proc_table[my_location].status = IN_USE;

  you should sync with the parent here
  which function to call?

  Then get the start function and its argument
  */
}

int spawn_real(char *name, int (*func)(char *), char *arg, int stack_size, int priority)
{
  // this must fork1, this is in kernel mode
  // maintain parent child relationship at phase 3 process table

  int pid = getpid() % MAXPROC;
  int child_pid;
  proc_ptr child_proc;
  int result;

  // is this is for storing linked lists?
  // proc_ptr kidptr, prevptr;

  // current proc
  proc_ptr proc = &_proc_table[pid % MAXPROC];

  // create child
  child_pid = fork1(name, spawn_launch, arg, stack_size, priority);

  child_proc = &_proc_table[child_pid % MAXPROC];
  if (name != NULL)
  {
    strncpy(child_proc->name, name, MAXNAME - 1);
  }
  child_proc->parent_proc = proc;
  if (arg != NULL)
  {
    // TODO: is this really needed?
    // strncpy(child_proc->start_func_args, arg, MAXARG - 1);
  }
  // add process data to the user proc table
  child_proc->start_func = func;

  // sending a message to child, so it waits before for me before running
  // NOTE: needs to happen at the end
  result = MboxCondSend(child_proc->start_mbox, NULL, 0);

  // was there a child waiting??
  printf("!!result %d\n", result);

  // TODO: add the new process to list of my children
  list_push_back(&proc->children_procs, child_pid);

  // TODO: if result is fail, like can't send message,
  //  then this function needs to fails
  if (result == -1)
  {
    // do something?
  }
  //  more to add

  return child_pid;
}

int wait_real(int *status)
{
  int pid = getpid();
  // int child_pid = *status;

  proc_ptr proc = &_proc_table[pid % MAXPROC];

  int quit_child = join(status);

  // printf("!!pid %d waits on %d children. %d quits\n", pid, proc->children_procs.length,quit_child );

  if (quit_child > 0)
  {
    // remove the child from the list of my children
    // printf("!!Before: %d", proc->children_procs.length);
    list_remove(&proc->children_procs, quit_child, 0);
    // printf("!!After: %d", proc->children_procs.length);
    return quit_child;
  }
  else
  {
    // TODO: terminate
    return -1;
  }
}

void terminate_real(int exit_code)
{

  // get current proc
  proc_ptr proc = &_proc_table[getpid() % MAXPROC];

  // zapp all children
  for (int i = 0; i < proc->children_procs.length; ++i)
  {
    int child_pid = list_at(&proc->children_procs, i);
    // printf("!!zapping %d : %d\n ", child_pid, exit_code);
    zap(child_pid);
  }

  // free the proc
  proc->status = EMPTY;
  list_clear(&proc->children_procs);
  quit(exit_code);

  /*
 TEACHER NOTES

terminate this process and all of it's children
zap each of the child process ( use existing zap )
invoking process will block, unblocked with the zapped process quits

cleanup process 3 proc table
adjust prent's children list on proc table

*/
}

int semcreate_real(int init_value)
{

  // probably need to do this?
  // if(init_value < 0)
  //  return -1;

  // TODO: protect with critical section
  sem_ptr sem = NULL;
  for (int i = 0; i < MAXSEMS; ++i)
  {
    if (_sem_table[i].status == EMPTY)
    {
      sem = &_sem_table[i];
      sem->status = IN_USE;
      break;
    }
  }

  if (sem == NULL)
    return -1;

  sem->handle = _sem_idx++;
  sem->value = init_value;

  return sem->handle;

  /*
    TEACHER NOTES
  return result of of the creationg to semcreate
  secreate then puts this result back into sysargs
  change to user mode, then return
  */
}

// TODO: use zero slot mailbox to block process
//  another process can wake it up

// decrement
// Sem Pause
int semp_real(int semaphore)
{

  int pid = getpid();
  sem_ptr sem = &_sem_table[semaphore % MAXSEMS];

  if (sem->value <= 0)
  {
    // MboxSend(_crit_mbox, NULL, 0);
    list_push_back(&sem->waiting_procs, pid);
    // MboxReceive(_crit_mbox, NULL, 0);

    // Block, wait for semv to free me.
    int _pid;
    MboxReceive(sem->priv_mbox, &_pid, sizeof(int));

    // if I stopped waiting because the semaphore was being freed
    //  then I need to wait synchronize with freeing process, by waiting
    //  on the free mailbox
    if (_pid == -1)
    {
      // wait for free to finish
      MboxSend(sem->free_mbox, NULL, 0);
      terminate_real(1);
    }
  }
  else
  {
    sem->value--;
  }

  // if sem->status = FREED

  /* TEACHER NOTES

    // find the table enry for the handle

  // if semValue  <= 0
  // Each List needs to be protected by critical section
  // MboxSend(Mutex_mailbox)
  //  add self to waiting block list
  // MboxReceive(mutexMailox)
  //  block with zero slot mailbox
  // else
  //  --semValue;


    what if the semaphore > 0
   decrement and retunr

  else
  Mbox receive used to block on the private mailbox of the semaphore
  after unblocked
  if the semaphore is being freed, need to sycnrhonise with the process that is freeing the semaphored
  HINT: use another zero slot mailbox
  */
  return 0;
}

// increment
int semv_real(int semaphore)
{

  // find the table entry for the handle
  sem_ptr sem = &_sem_table[semaphore % MAXSEMS];
  int pid = getpid();

  sem->value++;

  // if there are wiating process, then MboxSend() to first waiting procs
  if (sem->waiting_procs.length > 0)
  {
    list_pop_front(&sem->waiting_procs);
    MboxSend(sem->priv_mbox, &pid, sizeof(int));
  }

  /* TEACHER NOTES

    what ifthe value > 0
    decrement the value and return

   whatif the value == 0
   need to block the process
   can use MboxCondSend to see if MBox will block or not
   is there any process blocked onthe semaphore because of P operation?
   MboxCondSend can be used to check the semaphores private makebox used for blocking
   no process is blocked on it

  */
  return 0;
}

int semfree_real(int semaphore)
{

  // loop through and terminate any processes that are waiting on this semaphore

  /* TEACHER NOTES
     TWO conditions to consider
  // where there is no process blocked on the semaphore
  // where there are process blocked on the semaphore

  */

  if (semaphore < 0 || semaphore > MAXSEMS)
    return -1;

  sem_ptr sem = &_sem_table[semaphore];

  if (sem->status == EMPTY)
    return -1;

  int process_count = 0;
  int FREE = -1;
  while (MboxCondSend(sem->priv_mbox, &FREE, sizeof(int)) == 0)
  {
    process_count++;
  }
  /*
   TEACHER NOTES
   Each process that we we just release will do a send to the free_mbox.
    We will receive process_count times to get each of those messages.
    After we get all the message the smaphore can be marked as
    avaialable for future allocation
  */
  for (int i = 0; i < process_count; i++)
  {
    MboxReceive(sem->free_mbox, 0 /*NULL*/, 0);
  }
  /*
  TEACHER NOTES
  MORE CODE is needed to conclude the function
  */
  // terminate all processes
  //  for (int i = 0; i < sem->waiting_procs.length; i++){
  //    int waiting_pid = list_at(&sem->waiting_procs,i);
  //    zap(waiting_pid);
  //  }

  sem->status = EMPTY;
  list_clear(&sem->waiting_procs);

  // TODO: needs to retun 1 if there are process blocked on the semaphore.
  if (process_count > 0)
    return 1;
  else
    return 0;
}

int gettimeofday_real(int *time)
{
  *time = sys_clock();
  return 0;
}

int cputime_real(int *time)
{
  *time = readtime();
  return 0;
}

int getPID_real(int *pid)
{
  *pid = getpid();
  return 0;
}
