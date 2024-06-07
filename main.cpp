#include <cstdio>
#include <unistd.h>
#include <sys/wait.h>
#include <cstdlib>
#include <readline/readline.h>
#include "shelpers.hpp"

char **command_completion(const char *text, int start, int end);
char *command_generator(const char *text, int state);

int main(int argc, char* argv[]) {
    rl_attempted_completion_function = command_completion;
    rl_bind_key('\t', rl_complete);
    //Create a string
    char* charCommand;
    // Keep reading in commands till the user exits (types "exit" or used CTRL+C)
    while((charCommand = readline("")) != nullptr ){
        // Call tokenize on the string
        std::vector<std::string> tokenizedStrings = tokenize(charCommand);
        // Call getCommands on the tokenized string
        std::vector<Command> vectorOfCommands = getCommands(tokenizedStrings);

        for(Command& command: vectorOfCommands){
            if( command.exec == "exit" ){ // exit if the command typed is "exit"
                exit(0);
            }
            if( command.exec == "cd" ){
                if( command.argv[1] == nullptr ){
                    char* homeDirectory = (char*)"HOME";
                    chdir( getenv( homeDirectory ) );
                }
                else{
                    chdir( command.argv[1] );
                }
            }
            else {
                pid_t pid = fork(); // fork
                if( pid < 0 ){ // if pid_t is less than zero, there was an error in forking
                    std::perror("Error in fork()");
                    exit(1);
                }
                else if ( pid == 0 ){ // successful fork(); inside child
                    //std::cout << "Hello from inside the child!" << std::endl; // used for debugging
                    if( dup2(command.fdStdin, 0 ) < 0 ){ // dup2 failed
                        std::perror("Error in input dup2()");
                        exit(1);
                    }

                    if( dup2(command.fdStdout, STDOUT_FILENO) < 0 ){ // dup2 failed
                        std::perror("Error in input dup2()");
                        exit(1);
                    }

                    if( execvp( command.argv[0],const_cast<char *const *>( command.argv.data() ) ) < 0){
                        std::perror("Error in execvp()");
                    }
                }
                else{ // parent process
                    pid_t waitPID;
                    // wait for child to execute
                    // if the child has executed properly
                    if( command.fdStdin != 0 ){ // if it's open...
                        if( close(command.fdStdin) < 0 ){ // ... close it
                            std::perror("Error in closing input in parent");
                        }
                    }
                    if( command.fdStdout != 1 ){ // if it's open...
                        if( close(command.fdStdout) < 0 ){ // ... close it
                            std::perror("Error in closing output in parent");
                        }
                    }
                    if( waitpid(pid, &waitPID, 0) < 0 ){ // if there is an error...
                        std::perror("Error in child process");
                    }
                    //std::cout << "Hello from inside the parent!" << std::endl; // used for debugging
                }
            }

        }
    }

    return 0;
}

char *command_names[] = {
        (char*)"cat",
        (char*)"cd",
        (char*)"cp",
        (char*)"echo",
        (char*)"grep",
        (char*)"head",
        (char*)"ls",
        (char*)"ls -a",
        (char*)"ls -l",
        (char*)"man",
        (char*)"mkdir",
        (char*)"mv",
        (char*)"pwd",
        (char*)"rm",
        (char*)"rmdir",
        (char*)"tail",
        (char*)"touch",
        (char*)"wc",
        NULL
};

char **command_completion(const char *text, int start, int end){
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}

char *command_generator(const char *text, int state)
{
    static int list_index, len;
    char *name;

    if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    while ((name = command_names[list_index++])) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);
        }
    }

    return NULL;
}
