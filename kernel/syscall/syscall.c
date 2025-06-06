#include "syscall/syscall.h"

#include <stddef.h>
#include <stdint.h>

#include "abi/syscall/syscall.h"
#include "common/log.h"
#include "cpu/cpu.h"
#include "cpu/gdt.h"
#include "cpu/registers.h"
#include "memory/heap.h"
#include "memory/vm.h"

#define MSR_EFER_SCE (1 << 0)

#define MAX_DEBUG_STR_LEN 512



extern void syscall_entry();

void* copy_buffer_from_user(void* src, size_t length) {
    void* buffer = kmalloc(length);

    VmAddressSpace* as = cpu_current()->scheduler->current_thread->proc->as;
    size_t bytes_read = vm_copy_from(buffer, as, (uintptr_t) src, length);

    if (bytes_read != length) {
        kfree(buffer);
        return nullptr;
    }

    return buffer;
}

void* copy_string_from_user(char* src, size_t length) {
    char* str = copy_buffer_from_user(src, length + 1);

    if (str == nullptr)
        return nullptr;

    str[length] = '\0';
    return str;
}

void syscall_init() {
    write_msr(MSR_EFER, read_msr(MSR_EFER) | MSR_EFER_SCE);
    write_msr(MSR_STAR, ((uint64_t) GDT_SELECTOR_CODE64_RING0 << 32) | ((uint64_t) (GDT_SELECTOR_DATA64_RING3 - 8) << 48));
    write_msr(MSR_LSTAR, (uint64_t) syscall_entry);
    write_msr(MSR_SFMASK, read_msr(MSR_SFMASK) | (1 << 9));
}

void syscall_exit() {

}

SyscallResult syscall_debug(char* str, size_t length) {
    SyscallResult res = {};

    if (length > MAX_DEBUG_STR_LEN)
        logln(LOG_WARN, "SYSCALL_DEBUG", "Debug str limit reached (got: %lu chars, max: %lu)", length, MAX_DEBUG_STR_LEN);

    str = copy_string_from_user(str, length);
    if (str == nullptr) {
        res.error = SYSCALL_ERR_INVALID_VALUE;
        return res;
    }

    logln(LOG_DEBUG, "SYSCALL_DEBUG", "%s", str);

    return res;
}

SyscallResult syscall_set_tcb(void* ptr) {
    SyscallResult res = {};

    logln(LOG_DEBUG, "SYSCALL", "set_tcb(ptr: %#p)", ptr);
    write_msr(MSR_FS_BASE, (uint64_t) ptr);

    return res;
}

SyscallResult syscall_anon_alloc(size_t size) {
    SyscallResult res = {};

    if (size == 0 || size % PAGE_SIZE != 0) {
        res.error = SYSCALL_ERR_INVALID_VALUE;
        return res;
    }

    bool prev_state = cpu_int_mask();
    VmAddressSpace* as = cpu_current()->scheduler->current_thread->proc->as;
    cpu_int_restore(prev_state);

    void* ptr = vm_map_anon(as, nullptr, size, VM_PROT_RW, VM_CACHING_DEFAULT, VM_FLAG_ZERO);
    res.value = (uintptr_t) ptr;

    logln(LOG_DEBUG, "SYSCALL", "anon_alloc(size: %#lx) = %#p", size, ptr);

    return res;
}

SyscallResult syscall_anon_free(void* ptr, size_t size) {
    SyscallResult res = {};

    if (ptr == nullptr || ((uintptr_t) ptr) % PAGE_SIZE != 0 || size == 0 || size % PAGE_SIZE != 0) {
        res.error = SYSCALL_ERR_INVALID_VALUE;
        return res;
    }

    bool prev_state = cpu_int_mask();
    VmAddressSpace* as = cpu_current()->scheduler->current_thread->proc->as;
    cpu_int_restore(prev_state);

    vm_unmap(as, ptr, size);

    logln(LOG_DEBUG, "SYSCALL", "anon_free(ptr: %#p, size: %#lx)", ptr, size);

    return res;
}
