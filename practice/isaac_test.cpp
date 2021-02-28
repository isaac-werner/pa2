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

		for(int i=0; i<3; i++)
		{
            if(i == 0 || i == 1) {
                pipe(fdp);
            }

				int pid = fork();


				if (pid == 0) { // child process

						if(i==0) // if first command "ls"
						{
								/* This command is using the write end of the
								 * pipe created. So we should close up the
								 * read end and dup2 the write end to STDOUT. We
								 * don't really need to close up the read end,
								 * however, since it will be closed on exit.
								 */

								
								//cout << "Child 1 = " << getpid() << endl;
								const char **args = new const char * [3];
								args[0] = "ls";
								args[1] = "-l";
								args[2] = NULL;
				
								// close the read end
								close(fdp[READ_END]);
								// use the write end of the pipe
								dup2(fdp[WRITE_END],STDOUT_FILENO);

								// close up the write end after use
								close(fdp[WRITE_END]);
				
								execvp("ls", (char **)args);

								exit(0);
								
						}
						else if (i == 1) // if second command "grep pipes"
						{
								/* This command is using the read end of the
								 * pipe. So we dup2 the read end
								 * of the new pipe to STDIN. We don't
								 * need to close the write end because it
								 * was closed in the parent.
								 */
								
								// cout << "Child 3 = " << getpid() << endl;
								const char **args = new const char * [3];
								args[0] = "grep";
								args[1] = "test";
								args[2] = NULL;

								// write end closed up in parent

								//read end of the pipe
                                close(fdp[READ_END]);
                                dup2(read_end_initial_pipe_in_chain, STDIN_FILENO);
                                close(read_end_initial_pipe_in_chain);
                                dup2(fdp[WRITE_END], STDOUT_FILENO);
                                close(fdp[WRITE_END]);
                                execvp("grep", (char **)args);

                                exit(0);

						} else {
								const char **args = new const char * [3];
								args[0] = "grep";
								args[1] = "_2";
								args[2] = NULL;

                                close(fdp[WRITE_END]);
                                dup2(read_end_initial_pipe_in_chain, STDIN_FILENO);
                                close(read_end_initial_pipe_in_chain);
                                execvp("grep", (char **)args);
                                exit(0);
                        }

				}
				else // parent
				{
						// close the write end of the pipe
						close(fdp[WRITE_END]);
						// NOTE: can't close the read end because it will get
						// transferred to the next child process and needs to
						// remain open						
						
						// now wait on the child process to finish
						// cout << "waited on " << pid << endl;

                        read_end_initial_pipe_in_chain = fdp[READ_END];

						int pid = wait(&status);

						
						if (WEXITSTATUS(status) != 0)
						{
								cout << "bad exit status" << endl;
						}
				}
		}
}
