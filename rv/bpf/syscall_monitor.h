#ifndef __SYSCALL_MONITOR_H__
#define __SYSCALL_MONITOR_H__

struct syscall_event {
    __u32 syscall_nr;
    __u32 pid;
    __s64 ret;
};

#endif
