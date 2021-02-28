/*
 * Copyright (c) 2018, Yutaka Tsutano
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

/**************************************************************
 * Author: Isaac Werner
 * Title: main.cpp
 * Date: Thursday, February 18
 * Description: Used parsed user input to run the commands
 *************************************************************/

#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "command.hpp"
#include "parser.hpp"

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/wait.h>


int exec(const std::string& cmd, const std::vector<std::string>& args)
{
    // Make an ugly C-style args array.
    std::vector<char*> c_args = {const_cast<char*>(cmd.c_str())};
    for (const auto& a : args) {
        c_args.push_back(const_cast<char*>(a.c_str()));
    }
    c_args.push_back(nullptr);
    return execvp(cmd.c_str(), c_args.data());
}

int main(int argc, char* argv[]) {
    std::string input_line;
    pid_t pid;
    int stdout_copy = dup(STDOUT_FILENO);

    int status = 0;
    int MAX_COMMANDS = 25;
    int SUCCESS_STATUS = 0;
    for (int i = 0; i < MAX_COMMANDS; i++) {
        // Redirect outpt to terminal
        dup2(stdout_copy, STDOUT_FILENO);

        // This means we are doing the -t option
        if (argc <= 1) {
            // Print the prompt.
            std::cout << "osh> " << std::flush;
        } 
        if (!std::getline(std::cin, input_line) || input_line == "exit") {
            break;
        }

        try {
            // Parse the input line.
            std::vector<shell_command> shell_commands = parse_command_string(input_line);

            std::cout << "-------------------------\n";
            for (const auto& cmd : shell_commands) {
                std::cout << cmd;
                std::cout << "-------------------------\n";   
            }
            for(int j = 0; j < shell_commands.size(); j++) {
                shell_command previous_command;
                if (j > 0) {
                    previous_command = shell_commands[j - 1];
                }
                if(j == 0 || previous_command.next_mode == next_command_mode::always || (previous_command.next_mode == next_command_mode::on_success && status == SUCCESS_STATUS) || (previous_command.next_mode == next_command_mode::on_fail && status != SUCCESS_STATUS)) {
                    pid = fork();
                    shell_command command = shell_commands[j];
                    if(command.cin_mode == istream_mode::file) {
                        std::vector<std::string> new_args;
                        new_args.push_back(command.cin_file.c_str());
                        command.args = new_args;
                    }
                    int file_descriptor;
                    if(!command.cout_file.empty()) {
                        if(command.cout_mode == ostream_mode::append ){
                            if ((file_descriptor = open(command.cout_file.c_str(), O_WRONLY|O_APPEND|O_CREAT, 0644)) < 0) {
                                perror(command.cout_file.c_str());	/* open failed */
                            exit(1);
                            }   
                        } else {
                            if ((file_descriptor = open(command.cout_file.c_str(), O_CREAT|O_TRUNC|O_WRONLY, 0644)) < 0) {
                                perror(command.cout_file.c_str());	/* open failed */
                            exit(1);
                            }  
                        }

                        dup2(file_descriptor, STDOUT_FILENO);
                        close(file_descriptor);

                    }
                    if (pid < 0) { /* error occurred */
                        fprintf(stderr, "Fork Failed"); 
                        return 1;
                    }
                    else if (pid == 0) { /* child process */
                        exec(command.cmd, command.args);

                    }
                    else {
                        // Used: https://stackoverflow.com/questions/2667095/how-to-get-the-return-value-of-a-program-ran-via-calling-a-member-of-the-exec-fa
                        wait(&status);
                    }
                }
            }
        }
        catch (const std::runtime_error& e) {
            std::cout << e.what() << "\n";
        }
    }
}