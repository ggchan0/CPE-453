#include "lwp.h"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

schedfun global_scheduler = NULL;
void * og_sp = NULL;

lwp_context lwp_ptable[LWP_PROC_LIMIT];
int lwp_procs = 0;
int lwp_running;
unsigned long next_pid = 0;

int round_robin_scheduling() {
   return 0;
}

int new_lwp(lwpfun fun, void* arg, size_t stacksize) {
   if (lwp_procs == LWP_PROC_LIMIT) {
      return -1;
   }
   lwp_running = lwp_procs;
   ++lwp_procs;
   lwp_ptable[lwp_running].pid = next_pid++ + 1;
   lwp_ptable[lwp_running].stacksize = stacksize;
   lwp_ptable[lwp_running].stack = malloc(sizeof(int *) * stacksize);
   ptr_int_t *temp_sp = lwp_ptable[lwp_running].stack + stacksize;
   ptr_int_t *temp_bp = NULL;

   temp_sp--;
   *temp_sp = (ptr_int_t) arg;
   temp_sp--;
   *temp_sp = (ptr_int_t) lwp_exit;
   temp_sp--;
   *temp_sp = (ptr_int_t) fun;
   temp_sp--;
   *temp_sp = 0xFEEDBEEF;
   temp_bp = temp_sp;
   temp_sp -= 7;
   *temp_sp = (ptr_int_t) temp_bp;

   lwp_ptable[lwp_running].sp = temp_sp;
   return lwp_running;
}

int lwp_getpid() {
   return lwp_ptable[lwp_running].pid;
}

void lwp_yield() {
   SAVE_STATE();
   GetSP(lwp_ptable[lwp_running].sp);
   if (global_scheduler == NULL) {
      lwp_running = (lwp_running + 1) % lwp_procs;
   } else {
      lwp_running = global_scheduler();
   }
   SetSP(lwp_ptable[lwp_running].sp);

   RESTORE_STATE();
}

void lwp_exit() {
   int i;
   free(lwp_ptable[lwp_running].stack);
   for (i = lwp_running; i < lwp_procs; i++) {
      lwp_ptable[i] = lwp_ptable[i + 1];
   }
   --lwp_procs;

   if (lwp_procs == 0) {
      lwp_stop();
   } else {
      if (global_scheduler == NULL) {
         lwp_running = round_robin_scheduling();
      } else {
         lwp_running = global_scheduler();
      }
      SetSP(lwp_ptable[lwp_running].sp);
      RESTORE_STATE();
   }
}

void lwp_start() {
   if (lwp_procs == 0) {
      return;
   }
   SAVE_STATE();
   GetSP(og_sp);
   if (global_scheduler == NULL) {
      lwp_running = round_robin_scheduling();
   } else {
      lwp_running = global_scheduler();
   }
   SetSP(lwp_ptable[lwp_running].sp);
   RESTORE_STATE();
}

void lwp_stop() {
   SAVE_STATE();
   SetSP(og_sp);
   RESTORE_STATE();
}

void lwp_set_scheduler(schedfun scheduler) {
   global_scheduler = scheduler;
}
