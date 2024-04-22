/*
This is receiver, implement read function here
*/
#include "library/syscalls.h"
#include "kernel/error.h"
#include "library/string.h"

int main(int argc, const char** argv)
{
    printf("Receiver with pid: %d start\n",syscall_process_self());

    const char * fname = argv[0];
    int content_size = PAGE_SIZE;

    syscall_make_named_pipe(fname);

    int fd = syscall_open_named_pipe(fname);
    if(fd < 0) {
        printf("Error: pipe not found\n");
        return KERROR_NOT_A_PIPE;
    }

    char buffer[content_size];
    syscall_object_read(fd, buffer, content_size, 0);


    printf("Receiver with pid: %d, receive: %s\n", syscall_process_self(), buffer);

}