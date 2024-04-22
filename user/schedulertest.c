/*
    This is a program for launching priority scheduling
*/


#include "library/syscalls.h"
#include "library/stdio.h"
#include "library/errno.h"

int main()
{

    const char * path_list[] = {"bin/process1.exe","bin/process2.exe","bin/process3.exe","bin/process4.exe","/bin/process5.exe"};
    const char * priority_list[] = {"9","7","2","1","5"};
    int argc = 3;
    int pid[] = {};

    for(int i = 0; i < 5 ;i++)
    {
        const char *argv[3];
        argv[0] = path_list[i];
        argv[1] = (const char *) "priority";
        argv[2] = priority_list[i];

        const int pfd = syscall_open_file(KNO_STDDIR,argv[0],0,0);
        pid[i] = syscall_process_run(pfd, argc, &argv[0]);
        syscall_object_close(pfd);

    }


    struct process_info info;
    syscall_process_wait(&info, 0);

    return 0;

}