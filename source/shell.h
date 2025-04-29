#ifndef SHELL_H
#define SHELL_H

extern void shell_receive(fd_set *fdmask);
extern bool close_shell(short unr);
extern bool cmd_shell(short unr, bool transparent);
extern bool cmd_run(short unr, bool transparent, char *command, char *ofi, char *add_environment);
extern bool write_pty(short unr, int len, char *str);
extern void shell_fdset(int *max_fd, fd_set *fdmask);
extern int my_exec1(char *s, int hang);
  
#define my_system(s) my_exec1(s, 1)
#define my_exec(s) my_exec1(s, 0)

#endif

