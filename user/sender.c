/*
This is receiver, implement write function here
*/
#include "library/syscalls.h"
#include "kernel/error.h"
#include "library/string.h"

int main(int argc, const char** argv)
{
    
    printf("Sender with pid: %d start\n",syscall_process_self());

    const char * fname = argv[0];
    const char * content = argv[1];

    int fd = syscall_open_named_pipe(fname);
    if(fd < 0) {
        printf("Error: pipe not found\n");
        return KERROR_NOT_A_PIPE;
    }

    int content_size = strlen(content) + 1;
    syscall_object_write(fd, content, content_size, 0);

    
    printf("Sender with pid: %d, send: %s\n", syscall_process_self(), content);
}