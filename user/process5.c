#include "library/syscalls.h"
#include "library/stdio.h"
#include "library/string.h"

// invoked in test programs
void runForSeconds(int seconds)
{
    unsigned int startTime; // seconds
    syscall_system_time(&startTime);
    unsigned int timeElapsed;
    do
    {
        syscall_system_time(&timeElapsed);
        timeElapsed -= startTime;
    } while (timeElapsed < seconds);
}

int main(int argc, char const *argv[])
{
    
    printf("Process 5 with pid: %d priority: %d start\n", syscall_process_self(), syscall_process_pri());
    runForSeconds(1);
    printf("Process 5 with pid: %d priority: %d exit\n", syscall_process_self(), syscall_process_pri());
    syscall_process_exit(0);

    return 0;
}
