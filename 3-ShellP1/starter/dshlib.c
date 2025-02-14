#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "dshlib.h"

/*
 *  build_cmd_list
 *    cmd_line:     the command line from the user
 *    clist *:      pointer to clist structure to be populated
 * 
 *  This function builds the command_list_t structure passed by the caller
 *  It does this by first splitting the cmd_line into commands by spltting
 *  the string based on any pipe characters '|'.  It then traverses each
 *  command.  For each command (a substring of cmd_line), it then parses
 *  that command by taking the first token as the executable name, and
 *  then the remaining tokens as the arguments. 
 * 
 *  NOTE your implementation should be able to handle properly removing
 *  leading and trailing spaces!
 * 
 *  errors returned:
 * 
 *    OK:                      No Error
 *    ERR_TOO_MANY_COMMANDS:   There is a limit of CMD_MAX (see dshlib.h)
 *                             commands.  
 *    ERR_CMD_OR_ARGS_TOO_BIG: One of the commands provided by the user
 *                             was larger than allowed, either the 
 *                             executable name, or the arg string. 
 * 
 *  Standard Library Functions You Might Want To Consider Using
 *      memset(), strcmp(), strcpy(), strtok(), strlen(), strchr()
 */
int build_cmd_list(char *cmd_line, command_list_t *clist){
    char *cmd_token = strtok(cmd_line, PIPE_STRING);
    int  exe_len;
    int  args_len;

    //start by clearing the clist struct
    memset(clist, 0, sizeof(command_list_t));

    clist->num = 0;
    while (cmd_token != NULL){
        if(clist->num >= CMD_MAX){
            return ERR_TOO_MANY_COMMANDS;
            break;
        }

        //trim any leading spaces
        while(*cmd_token && isspace(*cmd_token))
            cmd_token++;
        //trim any trailing spaces
        char *end = cmd_token + strlen(cmd_token) - 1;
        while ((end > cmd_token) && (isspace(*end))){
            *end = '\0';
            end--;
        }

        command_t *cmd = &clist->commands[clist->num];
        char *space = strchr(cmd_token, SPACE_CHAR);
        if (space)
        {
            *space = '\0';
            char *arg_ptr = space+1;
            exe_len = strlen(cmd_token);
            args_len = strlen(arg_ptr);

            if((exe_len >= EXE_MAX) || (args_len > ARG_MAX))
                return ERR_CMD_OR_ARGS_TOO_BIG;
            strcpy(cmd->exe, cmd_token);
            strcpy(cmd->args, arg_ptr);
        }
        else
        {
            exe_len = strlen(cmd_token);
            if(exe_len >= EXE_MAX)
                return ERR_CMD_OR_ARGS_TOO_BIG;
            strcpy(cmd->exe, cmd_token);
            cmd->args[0] = '\0';
        }
        
        clist->num++;
        cmd_token = strtok(NULL, PIPE_STRING); // Get the next command
    }

    if (clist->num == 0)
        return WARN_NO_CMDS;

    return OK;
}

/* WIP - MOVE TO NEXT ASSIGNMENT

int build_argv(command_t *pcmd){
    char *str_args = pcmd->args;    //get args string
    int argc = 0;
    int m_space = 1;
    int m_quote = 0;
    char curr_char;
    char *pcurr_char;

    while(*str_args){
        curr_char = *str_args;
        pcurr_char = str_args;
        str_args++;                 //advance char

        if (curr_char == '"'){
            *pcurr_char = '\0';
            if (m_quote && !m_space){    //we are at teh end of the sequence
                m_space = 1;
            }
            if (!m_quote && m_space){
                m_space = !m_space;     //toggle space mode off
                pcmd->argv[argc] = pcurr_char + 1;
                argc++;
            }
           
            m_quote = !m_quote;   //toggle quote mode
            continue;
        }

        //suck up any chars if in quote mode
        if (m_quote) {
            continue;
        }

        //now lets see if we are at the start of a new command
        //basically we are in space mode but the curren char is
        //not a space
        if (!isspace(curr_char) && m_space){
            m_space = !m_space;     //toggle space mode off
            pcmd->argv[argc] = pcurr_char;
            argc++;
        }

        //suck up extra spaces
        if ((m_space) && (!m_quote)){
            continue;
        }

        //at this point we have the next char to consider
        //we will look for the end of the token
        if (isspace(curr_char) && !m_space){
            *pcurr_char = '\0';     //put a null in at first delimiter
            m_space = !m_space;
        }
    }

    if(m_quote){
        pcmd->argc = 0;
        return ERR_CMD_ARGS_BAD;
    }

    pcmd->argc = argc;
    return OK;
}


int build_cmd_list_ec(char *cmd_line, command_list_t *clist){
    char *cmd_token = strtok(cmd_line, PIPE_STRING);
    int  exe_len;
    int  args_len;

    //start by clearing the clist struct
    memset(clist, 0, sizeof(command_list_t));

    clist->num = 0;
    while (cmd_token != NULL){
        if(clist->num >= N_CMD_MAX){
            return ERR_TOO_MANY_COMMANDS;
            break;
        }

        //trim any leading spaces
        while(*cmd_token && isspace(*cmd_token))
            cmd_token++;
        //trim any trailing spaces
        char *end = cmd_token + strlen(cmd_token) - 1;
        while ((end > cmd_token) && (isspace(*end))){
            *end = '\0';
            end--;
        }

        command_t *cmd = &clist->commands[clist->num];
        char *space = strchr(cmd_token, SPACE_CHAR);
        if (space)
        {
            *space = '\0';
            char *arg_ptr = space+1;
            exe_len = strlen(cmd_token);
            args_len = strlen(arg_ptr);

            if((exe_len >= EXE_MAX) || (args_len > ARG_MAX))
                return ERR_CMD_OR_ARGS_TOO_BIG;
            strcpy(cmd->exe, cmd_token);
            strcpy(cmd->args, arg_ptr);
        }
        else
        {
            exe_len = strlen(cmd_token);
            if(exe_len >= EXE_MAX)
                return ERR_CMD_OR_ARGS_TOO_BIG;
            strcpy(cmd->exe, cmd_token);
            cmd->args[0] = '\0';
        }
        if (build_argv(cmd) != OK)
            return ERR_CMD_ARGS_BAD;
        clist->num++;
        cmd_token = strtok(NULL, PIPE_STRING); // Get the next command
    }

    if (clist->num == 0)
        return WARN_NO_CMDS;

    return OK;
}

*/