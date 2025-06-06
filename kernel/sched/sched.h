#pragma once

#include "events/event.h"
#include "lib/list.h"
#include "sched/thread.h"

typedef struct {
    Thread* current_thread;
    List ready_queue;
    Thread* idle_thread;
    Event preemption_event;
} Scheduler;

void sched_yield(ThreadStatus target_status);
void sched_init();
void sched_start();
