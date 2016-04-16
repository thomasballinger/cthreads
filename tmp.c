#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

static jmp_buf scheduler_buf;
static jmp_buf after_schedule;
static jmp_buf saved_thread;
static int threadnum; // initialized to 0

typedef void (callback)(void);

void yield(){
  if (!setjmp(saved_thread)){
    printf("created save point in yield\n");
    longjmp(scheduler_buf, 1);
  }
  printf("resuming thread after yield\n");
}

void run_cb_then_longjump_to(callback *cb, jmp_buf jb){
  if (!setjmp(saved_thread)){
    printf("created jump buffer for thread\n");
    longjmp(jb, 1);
  }
  printf("thread resumed for the first time, calling cb\n");
  (*cb)();
  longjmp(scheduler_buf, 2);
}

void spawn_thread_1(callback *cb){
  printf("created thread 1 space 1000 bytes beyond spawn_thread\n");
  char space[1000];
  space[1000-1] = 'a';
  printf("about to schedule thread!\n");
  run_cb_then_longjump_to(cb, after_schedule);
}

void hello(void){
    printf("thread1: Hello, I'm a thread!\n");
    yield();
    printf("thread1: Back to running that thread!\n");
    yield();
    printf("thread1: Running the thread for the last time");
}

void run_threads() {
  switch (setjmp(scheduler_buf)){
    case 0: // means this was the initial save
      printf("scheduler: created initial save\n");
      printf("scheduler: starting to work on thread\n");
      longjmp(saved_thread, 1);
      break;
    case 1: // means not yet done
      printf("scheduler: thread was just paused, now let's resume it\n");
      longjmp(saved_thread, 1);
      printf("this should never happen\n");
    case 2: // means done
      printf("scheduler: all done!");
      return;
  }
}

int main() {
  if (!setjmp(after_schedule)){
    spawn_thread_1(&hello);
  }
  run_threads();
  return 0;
}
