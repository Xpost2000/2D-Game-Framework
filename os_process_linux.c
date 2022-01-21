#include <sys/types.h>
#include <sys/wait.h> //wait
#include <signal.h>   //kill
#include <unistd.h>

#define FORK_CHILD_PROCESS (0)
#define FORK_BAD           (-1)
// Linux doesn't expose threads... Weirdo.
struct os_process_information {
    pid_t forked_process_id;
};

struct os_process_information os_process_shell_start(char* shell_command) {
    pid_t new_pid = fork();

    if (new_pid == FORK_CHILD_PROCESS) {
        execl("/usr/bin/sh", "sh", "-c", shell_command);
    }

    return (struct os_process_information) {
        .forked_process_id = new_pid
    };
}

void os_process_await_multiple(struct os_process_information* processes, int* return_codes, size_t count) {
    for (size_t process_index = 0; process_index < count; ++process_index) {
        if (processes[process_index].forked_process_id != FORK_CHILD_PROCESS) {
            wait(&return_codes[process_index]);
            kill(processes[process_index].forked_process_id, 9);
        }
    }
}

int os_process_shell_start_and_run_synchronously(char* shell_command) {
    int return_code = 0;

    struct os_process_information process = os_process_shell_start(shell_command);
    os_process_await_multiple(&process, &return_code, 1);

    return return_code;
}
