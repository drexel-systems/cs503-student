#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include "dshlib.h"

int alloc_cmd_buff(cmd_buff_t *cmd_buff)
{
    cmd_buff->_cmd_buffer = malloc(SH_CMD_MAX);
    if (cmd_buff->_cmd_buffer == NULL)
    {
        perror("malloc fail");
        return ERR_MEMORY;
    }
    cmd_buff->argc = 0;
    cmd_buff->argv[0] = cmd_buff->_cmd_buffer;
    return OK;
}

int free_cmd_buff(cmd_buff_t *cmd_buff)
{
    if (cmd_buff->_cmd_buffer != NULL)
    {
        free(cmd_buff->_cmd_buffer);
        cmd_buff->_cmd_buffer = NULL;
        cmd_buff->argc = 0;
        memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    }
    return OK;
}

int clear_cmd_buff(cmd_buff_t *cmd_buff)
{
    if (cmd_buff->_cmd_buffer == NULL)
        return ERR_MEMORY;

    cmd_buff->argc = 0;
    memset(cmd_buff->argv, 0, sizeof(cmd_buff->argv));
    memset(cmd_buff->_cmd_buffer, 0, SH_CMD_MAX);
    cmd_buff->argv[0] = cmd_buff->_cmd_buffer;

    return OK;
}

int build_cmd_buff(char *cmd_line, cmd_buff_t *cmd_buff)
{
    char *cl_ptr = cmd_line;
    char *cb_ptr = cmd_buff->_cmd_buffer;
    char curr_char;
    // note students can use ints for true/false booleans
    bool is_space = true;
    bool is_quote = false;

    if (strlen(cmd_line) >= SH_CMD_MAX)
        return ERR_CMD_OR_ARGS_TOO_BIG;

    if (clear_cmd_buff(cmd_buff) != OK)
    {
        return ERR_MEMORY;
    }

    while (*cl_ptr)
    {
        curr_char = *cl_ptr;

        // chew up whitespace
        if ((curr_char == ' ') && is_space && !is_quote)
        {
            cl_ptr++;
            continue;
        }

        // hanlde quote ending
        if ((curr_char == '\"') && is_quote)
        {
            is_quote = false;
            is_space = false;
            *cb_ptr = '\0';
            cb_ptr++;
            cl_ptr++;
            continue;
        }

        // handle quote starting
        if ((curr_char == '\"') && !is_quote)
        {
            is_quote = true;
            is_space = true;
            cl_ptr++; // eat up the quote

            if (*cl_ptr)
            { // not at end, starting quote token
                // add token to argv array
                if (cmd_buff->argc < CMD_MAX)
                {
                    *cb_ptr = *cl_ptr;
                    cmd_buff->argv[cmd_buff->argc] = cb_ptr;
                    cmd_buff->argc++;
                }
                else
                {
                    free(cmd_buff->_cmd_buffer);
                    return ERR_TOO_MANY_COMMANDS;
                }
            }
            continue;
        }

        // if in quote mode just copy whatever is there
        if (is_quote)
        {
            *cb_ptr = curr_char;
            cb_ptr++;
            cl_ptr++;
            continue;
        }

        // start of a token
        if ((curr_char != ' ') && is_space)
        {
            is_space = false;

            // add token to argv array
            if (cmd_buff->argc < CMD_MAX)
            {
                cmd_buff->argv[cmd_buff->argc] = cb_ptr;
                cmd_buff->argc++;
            }
            else
            {
                free(cmd_buff->_cmd_buffer);
                return ERR_TOO_MANY_COMMANDS;
            }
        }

        // end of a token
        if ((curr_char == ' ') && !is_space)
        {
            is_space = true;
            *cb_ptr = '\0';
            cb_ptr++;
            cl_ptr++;
            continue;
        }

        // this is just a regular character, copy it
        *cb_ptr = *cl_ptr;
        cb_ptr++;
        cl_ptr++;
    }

    // Now we just have to set the final element in argv[] to a null
    cmd_buff->argv[cmd_buff->argc] = '\0';
    return OK;
}

Built_In_Cmds match_command(const char *input)
{
    if (strcmp(input, "exit") == 0)
        return BI_CMD_EXIT;
    if (strcmp(input, "dragon") == 0)
        return BI_CMD_DRAGON;
    if (strcmp(input, "cd") == 0)
        return BI_CMD_CD;
    // extra credit
    if (strcmp(input, "rc") == 0)
        return BI_CMD_RC;
    return BI_NOT_BI;
}

extern void print_dragon();

Built_In_Cmds exec_built_in_cmd(cmd_buff_t *cmd)
{
    Built_In_Cmds ctype = BI_NOT_BI;
    ctype = match_command(cmd->argv[0]);

    switch (ctype)
    {
    case BI_CMD_DRAGON:
        print_dragon();
        return BI_EXECUTED;
    case BI_CMD_EXIT:
        return BI_CMD_EXIT;
    case BI_CMD_CD:
        chdir(cmd->argv[1]);
        return BI_EXECUTED;
    // extra credit
    case BI_CMD_RC:
        printf("%d\n", cmd->lastRc);
        return BI_EXECUTED;
    // end extra credit
    default:
        return BI_NOT_BI;
    }
}

int exec_cmd(cmd_buff_t *cmd)
{
    int c_result;
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return ERR_EXEC_CMD;
    }

    if (pid == 0)
    { // Child process
        execvp(cmd->argv[0], cmd->argv);
        // perror("execvp"); // we'll catch this higher up and print a message
        exit(errno); 
    }

    // wait for child in parent
    wait(&c_result);
    c_result = WEXITSTATUS(c_result); // get exit code

    return c_result;
}


/*
 * Implement your exec_local_cmd_loop function by building a loop that prompts the 
 * user for input.  Use the SH_PROMPT constant from dshlib.h and then
 * use fgets to accept user input.
 * 
 *      while(1){
 *        printf("%s", SH_PROMPT);
 *        if (fgets(cmd_buff, ARG_MAX, stdin) == NULL){
 *           printf("\n");
 *           break;
 *        }
 *        //remove the trailing \n from cmd_buff
 *        cmd_buff[strcspn(cmd_buff,"\n")] = '\0';
 * 
 *        //IMPLEMENT THE REST OF THE REQUIREMENTS
 *      }
 * 
 *   Also, use the constants in the dshlib.h in this code.  
 *      SH_CMD_MAX              maximum buffer size for user input
 *      EXIT_CMD                constant that terminates the dsh program
 *      SH_PROMPT               the shell prompt
 *      OK                      the command was parsed properly
 *      WARN_NO_CMDS            the user command was empty
 *      ERR_TOO_MANY_COMMANDS   too many pipes used
 *      ERR_MEMORY              dynamic memory management failure
 * 
 *   errors returned
 *      OK                     No error
 *      ERR_MEMORY             Dynamic memory management failure
 *      WARN_NO_CMDS           No commands parsed
 *      ERR_TOO_MANY_COMMANDS  too many pipes used
 *   
 *   console messages
 *      CMD_WARN_NO_CMD        print on WARN_NO_CMDS
 *      CMD_ERR_PIPE_LIMIT     print on ERR_TOO_MANY_COMMANDS
 *      CMD_ERR_EXECUTE        print on execution failure of external command
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 1+)
 *      malloc(), free(), strlen(), fgets(), strcspn(), printf()
 * 
 *  Standard Library Functions You Might Want To Consider Using (assignment 2+)
 *      fork(), execvp(), exit(), chdir()
 */
int exec_local_cmd_loop()
{
    char *cmd_buff;
    int rc = 0;
    int lastRc = 0;
    cmd_buff_t cmd;
    Built_In_Cmds bi_cmd;

    cmd_buff = malloc(SH_CMD_MAX);
    if (cmd_buff == NULL)
    {
        perror("cant alloc cmd buffer");
        return ERR_MEMORY;
    }

    if (alloc_cmd_buff(&cmd) != OK)
    {
        printf("error allocating command buffer!\n");
        free(cmd_buff);
        return ERR_MEMORY;
    }

    while (1)
    {
        printf("%s", SH_PROMPT);
        if (fgets(cmd_buff, SH_CMD_MAX, stdin) == NULL)
        {
            printf("\n");
            break;
        }
        cmd_buff[strcspn(cmd_buff, "\n")] = '\0';

        if (cmd_buff[0] == '\0')
        {
            continue;
        }

        rc = build_cmd_buff(cmd_buff, &cmd);
        switch (rc)
        {
        case ERR_MEMORY:
            return rc;
        case WARN_NO_CMDS:
            printf(CMD_WARN_NO_CMD);
            continue;
        case ERR_TOO_MANY_COMMANDS:
            printf(CMD_ERR_PIPE_LIMIT, CMD_MAX);
            continue;
        default:
            break;
        }

        // extra credit set rc from last run 
        cmd.lastRc = lastRc;

        // see if its built in
        bi_cmd = exec_built_in_cmd(&cmd);
        if (bi_cmd == BI_CMD_EXIT)
        {
            printf("exiting...\n");
            break; // we are done
        }
        if (bi_cmd == BI_EXECUTED)
        {
            continue; // done get next command
        }

        // we now have an external command to execute here
        rc = exec_cmd(&cmd);

        // if (rc) {
        //     // non-extra credit
        //     printf("error: command execution\n");
        // }

        // // extra credit
        if (rc) {
            // extra credit
            if (rc == EPERM) {
                printf("Not permitted\n");
            } else if (rc == ENOENT) {
                printf("Command not found in PATH\n");
            } else if (rc == EACCES) {
                printf("Permission denied\n");
            } else {
                printf("Command failed with exit code: %d\n", rc);
            }
        } 
        
        lastRc = rc;
    }

    free(cmd_buff);
    free_cmd_buff(&cmd);
    return OK;
}
