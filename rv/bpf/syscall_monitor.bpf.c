#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_tracing.h>
#include <bpf/bpf_core_read.h>
#include "syscall_monitor.h"


char LICENSE[] SEC("license") = "GPL";

struct {
    __uint(type, BPF_MAP_TYPE_RINGBUF);
    __uint(max_entries, 1 << 24);
} events SEC(".maps");

static __always_inline
int
read_syscall(u32 syscall_nr, s64 ret)
{
    struct syscall_event *e = bpf_ringbuf_reserve(&events, sizeof *e, 0);
    if (!e) {
        bpf_printk("ringbuffer overflow");
        return 0;
    }

    u64 pid_tgid = bpf_get_current_pid_tgid();
    e->pid = pid_tgid & ((1uLL << 32) - 1);
    // e->tgid = pid_tgid >> 32;

    e->syscall_nr = syscall_nr;
    e->ret = ret;
    
    bpf_ringbuf_submit(e, 0);

    return 0;
}

SEC("tracepoint/syscalls/sys_exit_open")
int trace_exit_open(struct trace_event_raw_sys_exit *ctx)
{
    return read_syscall(ctx->id, ctx->ret);
}

SEC("tracepoint/syscalls/sys_exit_openat")
int trace_exit_openat(struct trace_event_raw_sys_exit *ctx)
{
    return read_syscall(ctx->id, ctx->ret);
}

SEC("tracepoint/syscalls/sys_enter_exit")
int trace_enter_exit(struct trace_event_raw_sys_enter *ctx)
{
    return read_syscall(ctx->id, 0);
}

SEC("tracepoint/syscalls/sys_enter_exit_group")
int trace_enter_exit_group(struct trace_event_raw_sys_enter *ctx)
{
    return read_syscall(ctx->id, 0);
}
