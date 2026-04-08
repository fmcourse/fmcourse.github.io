#include <linux/types.h>
#include "syscall_monitor.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <ctype.h>
#include <bpf/libbpf.h>
#include <bpf/bpf.h>
#include "syscall_monitor.skel.h"
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/stat.h>

#define sys_entry(e) [SYS_##e] = #e
static const char *syscall_names[500] = {
    sys_entry(open),     sys_entry(openat),     sys_entry(creat),
    sys_entry(mkdir),    sys_entry(mkdirat),    sys_entry(chdir),
    sys_entry(fchdir),   sys_entry(chmod),      sys_entry(fchmod),
    sys_entry(chown),    sys_entry(fchown),     sys_entry(close),
    sys_entry(exit),     sys_entry(exit_group), sys_entry(umask),
    sys_entry(unlink),   sys_entry(rmdir),      sys_entry(getdents),
    sys_entry(link),     sys_entry(symlink),    sys_entry(getxattr),
    sys_entry(setxattr), sys_entry(execve),     sys_entry(execveat),
};
#undef sys_entry

static pid_t child = 0;
static struct syscall_monitor_bpf *skel = 0;
static int exited = 0;

static int handle_event(void *ctx, void *data, size_t len)
{
    struct syscall_event *e = data;

    if (e->pid == child) {
        printf("%s() = %d\n", syscall_names[e->syscall_nr], (int)e->ret);
        if (e->syscall_nr == SYS_exit || e->syscall_nr == SYS_exit_group) {
            exited = 1;
        }
    }

    return 0;
}

int
run(int argc, char *argv[])
{
    struct ring_buffer *rb = ring_buffer__new(bpf_map__fd(skel->maps.events), handle_event, 0, 0);

    if ((child = fork()) == 0) {
        ring_buffer__free(rb);
        execvp(argv[1], &argv[1]);
        fprintf(stderr, "%s: %s\n", argv[1], strerror(errno));
        return 127;
    }

    while (!exited) {
        ring_buffer__poll(rb, 100);
    }

    ring_buffer__free(rb);

    return 0;
}

int
main(int argc, char *argv[])
{
    if (!(skel = syscall_monitor_bpf__open_and_load())) {
        return 1;
    }

    if (syscall_monitor_bpf__attach(skel)) {
        return 1;
    }

    run(argc, argv);

    syscall_monitor_bpf__destroy(skel);

    return 0;
}
