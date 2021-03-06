#include "sh61.h"
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <ctype.h>
#include <signal.h>

// struct command
//    Data structure describing a command. Add your own stuff.

typedef struct command command;
struct command {
    command* prev; // prev command in list 
    int argc;      // number of arguments
    char** argv;   // arguments, terminated by NULL
    pid_t pid;     // process ID running this command, -1 if none
    int bg;        // backgrounding process
    int pipe;      // command is piped
    int in_fd;     // input file (pipelines)
    int out_fd;    // output file (pipelines)
    int redir_in;  // command is redirected into file
    int redir_out; // command is redirected out to file
    char* redir_in_file; // in file for redirection
    char* redir_out_file; // out File for redirection
    int status;    // status
    int cmd_chain; // command chaining
    command* next; // next command in list 
};

int needprompt = 0;

// command_alloc()
//    Allocate and return a new command structure.

static command* command_alloc(void) {
    command* c = (command*) malloc(sizeof(command));

    c->prev = NULL;
    c->argc = 0;
    c->argv = NULL;
    c->pid = -1;
    c->bg = 0;
    c->pipe = 0;
    c->in_fd = 0;
    c->out_fd = 0;
    c->redir_in = 0;
    c->redir_out = 0;
    c->redir_in_file = NULL;
    c->redir_out_file = NULL;
    c->status = 0;
    c->cmd_chain = 0;
    c->next = NULL;
    return c;
}

command* head = NULL;

// command_free(c)
//    Free command structure `c`, including all its words.

static void command_free(command* c) {
    for (int i = 0; i != c->argc; ++i)
        free(c->argv[i]);
    free(c->argv);
    free(c);
}


// command_append_arg(c, word)
//    Add `word` as an argument to command `c`. This increments `c->argc`
//    and augments `c->argv`.

static void command_append_arg(command* c, char* word) {
    c->argv = (char**) realloc(c->argv, sizeof(char*) * (c->argc + 2));
    c->argv[c->argc] = word;
    c->argv[c->argc + 1] = NULL;
    ++c->argc;
}


// COMMAND EVALUATION

// start_command(c, pgid)
void error_wrapper(char* msg)
{
    fprintf(stderr, "%s: %s\n", msg, strerror(errno));
    exit(1);
}

pid_t start_command(command* c, pid_t pgid) {
    int pipefd[2];
    int redir_out_fd = -1; 
    int redir_in_fd = -1;

    // Set Parent Process's PID to process group ID
    setpgid(c->pid, pgid);

    // cd Command: Must be executed in current process
    if (strcmp(c->argv[0], "cd") == 0) {
        c->status = chdir(c->argv[1]);
        return c->pid;
    }

    // Redirection - Setup files
    if(c->redir_out >= 1)
        redir_out_fd = creat(c->redir_out_file, S_IRWXU);

    if(c->redir_in >= 1)
        redir_in_fd = open(c->redir_in_file, O_RDWR);

    // Piping - Setup Pipe & in File for next command
    if (c->pipe == 1) {
        if (pipe(pipefd) != 0)
            error_wrapper("Pipe error\n");
        (*c->next).in_fd = pipefd[0];
    }

    // Fork a child to execute command
    if ((c->pid = fork()) < 0)
        error_wrapper("Fork error\n");

    if (c->pid != 0) {
        // Pipe Hygiene for Parent
        if (c->out_fd >= 1) 
            close(pipefd[1]);

        if (c->in_fd >= 1)
            close(c->in_fd);

        // File Cleanup if child is Redirecting
        if (c->redir_out >= 1)
            close(redir_out_fd);

        if (c->redir_in >= 1)
            close(redir_in_fd);
    }

    if (c->pid == 0) {

        // Divorce child process from old group
        setpgid(0, 0);

        // Pipe Hygiene for Parent
        if (c->out_fd != 0) {
            close(pipefd[0]);
            dup2(pipefd[1], STDOUT_FILENO);
            close(pipefd[1]);
        }

        if (c->in_fd != 0) {
            dup2(c->in_fd, STDIN_FILENO);
            close(c->in_fd);
        }

        // Setup Redirection In from File
        if(c->redir_in >= 1) {
            if(redir_in_fd < 0)
                error_wrapper("No such file or directory");
            else if (c->redir_in == 2) {
                dup2(redir_in_fd, STDERR_FILENO);
                close(redir_in_fd);
            }
            dup2(redir_in_fd, STDIN_FILENO);
            close(redir_in_fd);
        }

        // Setup Redirection Out to File
        if(c->redir_out >= 1) {
            if(redir_out_fd < 0)
                error_wrapper("No such file or directory");
            else if (c->redir_out == 2) {
                dup2(redir_out_fd, STDERR_FILENO);
                close(redir_out_fd);
            }

            dup2(redir_out_fd, STDOUT_FILENO);
            close(redir_out_fd);
        }


        // Execute Command in Child
        if (execvp(c->argv[0], c->argv) < 0)
            error_wrapper("Exec error\n");
    }

    return c->pid;
}

// run_list(c)
//    Run the command list starting at `c`.
//    PART 8: - Choose a process group for each pipeline.
//       - Call `set_foreground(pgid)` before waiting for the pipeline.
//       - Call `set_foreground(0)` once the pipeline is complete.
//       - Cancel the list when you detect interruption.

void run_list(command* c) {
    int options = WNOHANG;
    int bg = c->bg;
    pid_t pgid = c->pid;

    start_command(c, 0);

    while(c->next != NULL && (*c->next).argc > 0) {
        if (c->pid != 0) {
            // If command chain, set waitpid option to 0
            if (head->next != NULL)
                options = 0;

            // Check if any command in sequence is backgrounding
            command* bg_check = c;
            while (bg_check->next != NULL) {
                if ((*bg_check->next).bg == 1) {
                    bg = 1;
                    break;
                }

                bg_check = bg_check->next;
            }

            set_foreground(pgid);

            // If Command Chain is a Sequence, set option to 0
            if (c->cmd_chain == 3)
                waitpid(c->pid, &c->status, 0);
            else if (bg != 1)
                // If not a backgrounding process, wait with a WNOHANG option
                waitpid(c->pid, &c->status, options);

            // Set status of command
            if (WIFEXITED(c->status))
                c->status = WEXITSTATUS(c->status);

            // If status is error, stop
            if (c->status < -1)
                break;

            set_foreground(0);

            // Check if command chain logic is met, otherwise skip the next command
            if ((c->cmd_chain == 1 && c->status != 0) || (c->cmd_chain == 2 && c->status == 0)) {
                (*c->next).status = c->status;
                if ((*c->next).next != NULL)
                    c = (*c->next).next;
                else
                    break;
            } else
                c = c->next;
        }

        // Run the next command
        start_command(c, 0);
    }
}


// eval_line(c)
//    Parse the command list in `s` and run it via `run_list`.

void eval_line(const char* s) {
    int type;
    char* token;

    // build the command
    command* c = command_alloc();
    head = c;

    while ((s = parse_shell_token(s, &type, &token)) != NULL) {
        // Check if token is && or || and set cmd_chain and init new command
        if (type == TOKEN_AND || type == TOKEN_OR) {
            if (type == TOKEN_AND)
                c->cmd_chain = 1;
            else if (type == TOKEN_OR)
                c->cmd_chain = 2;

            command* c_new = command_alloc();
            if (c_new != NULL) {
                c_new->prev = c;
                c->next = c_new;
                c = c_new;
            }
        // Check if command is piped and if so, set pipe value/init new command
        } else if (type == TOKEN_PIPE) {
            command* c_new = command_alloc();
            
            c->pipe = 1;
            c->out_fd = 1;
            c_new->in_fd = 1;

            if (c_new != NULL) {
                c_new->prev = c;
                c->next = c_new;
                c = c_new;
            }
        // Check if command is redirected
        } else if (type == TOKEN_REDIRECTION) {
            if (strcmp(token,">") == 0)
                c->redir_out = 1;
            else if (strcmp(token,"<") == 0)
                c->redir_in = 1;
            else if (isdigit((int)token[0])) {
                if (token[1] == '>')
                    c->redir_out = 2;
                if (token[1] == '<')
                    c->redir_in = 2;
            }
        // Check if backgrounding or sequence, and set value/init new command
        } else if (type == TOKEN_BACKGROUND || type == TOKEN_SEQUENCE) {
            if (type == TOKEN_BACKGROUND)
                c->bg = 1;
            else if (type == TOKEN_SEQUENCE)
                c->cmd_chain = 3;

            command* c_new = command_alloc();
            if (c_new != NULL) {
                c_new->prev = c;
                c->next = c_new;
                c = c_new;            
            }
        // If normal, either store file name (for redir) or append command
        } else if (type == TOKEN_NORMAL) {
            if(c->redir_out >= 1) {
                c->redir_out_file = malloc(strlen(token) + 1);
                strcpy(c->redir_out_file, token);
            } else if (c->redir_in >= 1) {
               c->redir_in_file = malloc(strlen(token) + 1);
               strcpy(c->redir_in_file, token);
            } else
               command_append_arg(c, token);
        }
    }

    // execute it
    if (head->argc) {
        c = head;
        run_list(c);

        // Free linked list of commands
        c = head;
        while(c->next != NULL) {
            c = c->next;
            command_free(c);
        }

        command_free(head);
    }
}

int main(int argc, char* argv[]) {
    FILE* command_file = stdin;
    int quiet = 0;

    // Check for '-q' option: be quiet (print no prompts)
    if (argc > 1 && strcmp(argv[1], "-q") == 0) {
        quiet = 1;
        --argc, ++argv;
    }

    // Check for filename option: read commands from file
    if (argc > 1) {
        command_file = fopen(argv[1], "rb");
        if (!command_file) {
            perror(argv[1]);
            exit(1);
        }
    }

    // - Put the shell into the foreground
    // - Ignore the SIGTTOU signal, which is sent when the shell is put back
    //   into the foreground
    set_foreground(0);
    handle_signal(SIGTTOU, SIG_IGN);

    // Signal handling
    handle_signal(SIGINT, SIG_DFL);
    handle_signal(SIGALRM, SIG_DFL);

    char buf[BUFSIZ];
    int bufpos = 0;
    int needprompt = 1;

    while (!feof(command_file)) {
        // Print the prompt at the beginning of the line
        if (needprompt && !quiet) {
            printf("sh61[%d]$ ", getpid());
            fflush(stdout);
            needprompt = 0;
        }

        // Read a string, checking for error or EOF
        if (fgets(&buf[bufpos], BUFSIZ - bufpos, command_file) == NULL) {
            if (ferror(command_file) && errno == EINTR) {
                // ignore EINTR errors
                clearerr(command_file);
                buf[bufpos] = 0;
            } else {
                if (ferror(command_file))
                    perror("sh61");
                break;
            }
        }

        // If a complete command line has been provided, run it
        bufpos = strlen(buf);
        if (bufpos == BUFSIZ - 1 || (bufpos > 0 && buf[bufpos - 1] == '\n')) {
            eval_line(buf);
            bufpos = 0;
            needprompt = 1;
        }

        // Handle zombie processes and/or interrupt requests
        // Your code here!
    }

    return 0;
}