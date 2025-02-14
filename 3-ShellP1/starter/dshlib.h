#ifndef __DSHLIB_H__
    #define __DSHLIB_H__


//Constants for command structure sizes
#define EXE_MAX 64
#define ARG_MAX 256
#define CMD_MAX 8
// Longest command that can be read from the shell
#define SH_CMD_MAX EXE_MAX + ARG_MAX

typedef struct command
{
    char exe[EXE_MAX];
    char args[ARG_MAX];
} command_t;

/* WIP - Move to next assignment 
#define N_ARG_MAX    15     //MAX number of args for a command
typedef struct command{
    char exe [EXE_MAX];
    char args[ARG_MAX];
    int  argc;
    char *argv[N_ARG_MAX + 1];  //last argv[LAST] must be \0
}command_t;
*/

typedef struct command_list{
    int num;
    command_t commands[CMD_MAX];
}command_list_t;

//Special character #defines
#define SPACE_CHAR  ' '
#define PIPE_CHAR   '|'
#define PIPE_STRING "|"

#define SH_PROMPT "dsh> "
#define EXIT_CMD "exit"

//Standard Return Codes
#define OK                       0
#define WARN_NO_CMDS            -1
#define ERR_TOO_MANY_COMMANDS   -2
#define ERR_CMD_OR_ARGS_TOO_BIG -3
#define ERR_CMD_ARGS_BAD        -4      //for extra credit

//prototypes
int build_cmd_list(char *cmd_line, command_list_t *clist);

//extra credit prototypes
int build_argv(command_t *pcmd);
int build_cmd_list_ec(char *cmd_line, command_list_t *clist);

//output constants
#define CMD_OK_HEADER       "PARSED COMMAND LINE - TOTAL COMMANDS %d\n"
#define CMD_WARN_NO_CMD     "warning: no commands provided\n"
#define CMD_ERR_PIPE_LIMIT  "error: piping limited to %d commands\n"

#endif