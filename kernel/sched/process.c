#include "sched/process.h"
#include "lib/list.h"
#include "memory/vm.h"
#include "memory/heap.h"
#include "lib/string.h"



_Atomic uint64_t next_pid = 0;

Process* process_create(const char* name, VmAddressSpace* as) {
    Process* proc = kmalloc(sizeof(Process));

    *proc = (Process) {
        .pid = next_pid++,
        .as = as,
        .threads = LIST_NEW
    };

    strcpy(proc->name, name);

    return proc;
}
