#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

#define MAXTHREADS 10

#define SPAWN_THREAD(CB) {\
  if (threadnum == MAXTHREADS - 1){\
    printf("Too many threads! Max is MAXTHREADS");\
  }\
  if (threadnum == 0){\
    if (!setjmp(after_schedule)){\
      init_thread_start_spot();\
    }\
  }\
  if (!setjmp(after_schedule)){\
    recursive_pad(threadnum, &CB);\
  }\
}\

typedef void (callback)(void);

static jmp_buf thread_start_spot;
static jmp_buf scheduler_buf;
static jmp_buf after_schedule;
static jmp_buf saved_thread;
static int threadnum; // initialized to 0
static callback *cur_callback;


void init_thread_start_spot_after_spacing(void){
  printf("created thread space 1000 bytes beyond thread init\n");
  char space[1000];
  space[1000-1] = 'a';
  run_cb_then_longjump_to(cb, after_schedule);
}

// Allocate n*1000 bytes of stack space
void recursive_pad(int n, callback cb){
  char space[1000];
  space[1000-1] = 'a';
  if (n == 1){
    if (!setjmp(saved_thread)){
      printf("created jmp_buf for thread %d\n", threadnum);
      longjmp(after_schedule, 1);
    } else {
      (*cb)();
    }
  } else {
    recursive_pad(n-1, cb);
  }
}

void init_thread_start_spot(){
  int n = setjmp(thread_start_spot);
  if (n == 0){
    longjmp(after_schedule, 1);
  } else {
    recursive_pad(threadnum, cur_callback);
  }
}

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

void spawn_thread(callback *cb, n){
  cur_callback = cb;
  longjmp(thread_start_spot, n);
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
  SPAWN_THREAD(hello);
  run_threads();
  return 0;
}
