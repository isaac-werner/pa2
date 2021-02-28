/****************************************************************
 *
 * Author: Justin Bradley
 * Title: test_pipes_2.cpp
 * Date: Thursday, February 20, 2020
 * Description: Meant to help clarify pipes. This forks 2 children and connects
 * them with a pipe (1 total pipe).
 *
 ****************************************************************/
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#define READ_END 0
#define WRITE_END 1

using namespace std;

int main(int argc, char ** argv)
{
		int status = 0; // get the exit status (least significant 8 bits)
		int fdp[2]; // file descriptors for the pipe
        int read_end_initial_pipe_in_chain = 0;				
		// cout << "Parent = " << getpid() << endl;

        const char** args0 = new const char * [3];
        args0[0] = "ls";
        args0[1] = "-l";
        args0[2] = NULL;

        const char** args1 = new const char * [3];
        args1[0] = "grep";
        args1[1] = "test";
        args1[2] = NULL;

        const char** args2 = new const char * [3];
        args2[0] = "grep";
        args2[1] = "_2";
        args2[2] = NULL;

        const char** args3 = new const char * [3];
        args3[0] = "grep";
        args3[1] = "Feb";
        args3[2] = NULL;

        const char ** commands = new const char * [3];
        commands[0] = "ls";
        commands[1] = "grep";
        commands[2] = "grep";
        commands[3] = "grep";

        const char** args[] = {args0, args1, args2, args3};

		for(int i=0; i<4; i++)
		{
            if(i == 0 || i == 1 || i == 2) {
                pipe(fdp);
            }

				int pid = fork();

				if (pid == 0) {

						if(i==0) 
						{
								close(fdp[READ_END]);
								dup2(fdp[WRITE_END],STDOUT_FILENO);
								close(fdp[WRITE_END]);
						}
						else if (i == 1 || i == 2)
						{
                                close(fdp[READ_END]);
                                dup2(read_end_initial_pipe_in_chain, STDIN_FILENO);
                                close(read_end_initial_pipe_in_chain);
                                dup2(fdp[WRITE_END], STDOUT_FILENO);
                                close(fdp[WRITE_END]);

						} else {
                                close(fdp[WRITE_END]);
                                dup2(read_end_initial_pipe_in_chain, STDIN_FILENO);
                                close(read_end_initial_pipe_in_chain);
                        }
                        execvp(commands[i], (char **)args[i]);
                        exit(0);
        }
				else // parent
				{
						close(fdp[WRITE_END]);

                        read_end_initial_pipe_in_chain = fdp[READ_END];

						int pid = wait(&status);

						if (WEXITSTATUS(status) != 0)
						{
								cout << "bad exit status" << endl;
						}
				}
		}
}
