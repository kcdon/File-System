#ifndef _CMDS_H_
#define _CMDS_H_

/**
 * I pledge my honor that I have abided by the Stevens Honor System.
 * Christopher Hittner
 * James Romph
 */

typedef int (*SimCmd)(char**);

char** str_to_vec(char*, char);
void free_str_vec(char**);
void mergesort_longs(long*, int, int);

/**
 * Executes a command in the file system.
 */
void cmd_exec(char *argv[]);

/**
 * Modifies the working directory.
 */
int cmd_cd(char *argv[]);

int cmd_ls(char *argv[]);

/**
 * File/directory creators.
 */
int cmd_mkdir(char *argv[]);
int cmd_create(char *argv[]);

/**
 * Modifying the size of a file.
 */
int cmd_append(char *argv[]);
int cmd_remove(char *argv[]);

int cmd_delete(char *argv[]);

/**
 * Terminate program.
 */
int cmd_exit(char *argv[]);

/**
 * Breadth-first print of file structure.
 */
int cmd_dir(char *argv[]);

/**
 * Print file/disk info.
 */
int cmd_prfiles(char *argv[]);
int cmd_prdisk(char *argv[]);

int cmd_defrag(char *argv[]);

#endif
