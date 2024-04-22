/*
This is a program use for launch a test about named pipe.
***read or write functions do not implement here***
*/

#include "library/syscalls.h"
#include "library/string.h"

int main(void)
{
    const char *pipe_name = "test_named_pipe";
    const char *sender_exe = "/bin/sender.exe";
    const char *receiver_exe = "/bin/receiver.exe";
    
    const int fd_sender = syscall_open_file(KNO_STDDIR, sender_exe, 0, 0);
              
    if(fd_sender < 0 ) {
        printf("Failed to open sender\n");
        return 1;
    }

    const int fd_receiver = syscall_open_file(KNO_STDDIR, receiver_exe, 0, 0);
    
    if ( fd_receiver < 0) {
        printf("Failed to open receiver");
        return 1;
    }

    const char *content = "I love CS3103";

    
    const char *argv_sender[] = {pipe_name, content};
    const char *argv_receiver[] = {pipe_name};

    syscall_process_run(fd_receiver, 1, argv_receiver);
    syscall_process_run(fd_sender, 2, argv_sender);
    
    syscall_object_close(fd_receiver);
    syscall_object_close(fd_sender);

    

    return 0;
}