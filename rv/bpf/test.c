#include <fcntl.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>

int
main(int argc, char *argv[])
{
    open(argv[0], O_RDONLY);
    syscall(SYS_open, argv[0], O_RDONLY);
}
