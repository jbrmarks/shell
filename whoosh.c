#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>


    int
    main(int argc, char *argv[]) {


        bool running = true;
        //int MAX_LINE_INPUT = sizeof(char)*129;
        char input[132];
        char *args[132];
        int numArgs;
        char *paths[100];
        int numPaths;
        char d[3] = " \n";
        char error_message[30] = "An error has occurred\n";
        bool redirect;
        int redirIndex;

        // check for correct number of args
        if(argc != 1){
            write(STDERR_FILENO, error_message, strlen(error_message));
            //write(STDERR_FILENO, "13\n", 3);
            exit(1);
        }

        // set a default path
        paths[0] = "/bin";
        numPaths = 1;

        while (running) {

            //print console prompt

            char *msg = ("whoosh> ");
            write(STDOUT_FILENO, msg, strlen(msg));

            numArgs = 0;
            redirect = false;
            redirIndex = 0;

            input[0] = '\0';


            //wait for user input
            if (fgets(input, 132, stdin) != NULL) {

                //change newline to null
                char *pos;
                if ((pos = strchr(input, '\n')) != NULL) {
                    *pos = '\0';
                }else{
                    //if no newline, line is too long
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    //write(STDERR_FILENO, "11\n", 3);
                    while (fgets(input, 132, stdin) != NULL) {
                        //clear to end of input
                        if ((strchr(input, '\n')) != NULL) {break;}
                    }
                    continue;
                }

                //if line size too large, print error and continue
                if(strlen(input) > 128){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    //write(STDERR_FILENO, "1\n", 2);
                    continue;
                }

                char *token;

                if ((token = strtok(input, d)) != NULL) {
                    args[0] = strdup(token);
                    numArgs++;

                    int i = 1;
                    for (i = 1; i < 10; i++) {

                        token = strtok(NULL, d);
                        if (token != NULL) {
                            args[i] = strdup(token);
                            if (strcmp(token, ">") == 0) {
                                redirect = true;
                                if (redirIndex == 0) {
                                    redirIndex = i;
                                }
                            }
                            numArgs++;
                        } else {
                            args[i] = NULL;
                            break;
                        }

                    }

                    //check for bad redirect
                    if (redirect) {
                        if (redirIndex != numArgs - 2) {
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            //write(STDERR_FILENO, "12\n", 3);
                            continue;
                        }
                    }

                    // end reading input

                    //BUILT-IN COMMANDS

                    // path command to set paths
                    if(strcmp(args[0], "path") == 0) {

                        numPaths = numArgs - 1;

                        //read in paths

                        int i = 0;
                        for (i = 1; i < numArgs; i++) {
                            paths[i - 1] = strdup(args[i]);
                        }

                        continue;

                    }

                    // cd command
                    if(strcmp(args[0], "cd") == 0){
                        // cd with no arguments
                        if(numArgs == 1){
                            // get pointer to HOME
                            char *home;
                            if((home = getenv("HOME")) != NULL){
                                if(chdir(home) == 0){

                                }else{
                                    write(STDERR_FILENO, error_message, strlen(error_message));
                                    //write(STDERR_FILENO, "2\n", 2);
                                }
                            }else{
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                //write(STDERR_FILENO, "3\n", 2);
                            }

                        }else if(numArgs == 2){


                            char *cwd;
                            cwd = strdup(args[1]);
                            if(chdir(cwd) == 0){

                            }else{
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                //write(STDERR_FILENO, "4\n", 2);
                            }


                        }else{
                            //too many args
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            //write(STDERR_FILENO, "5\n", 2);
                        }
                        continue;
                    }

                    // pwd command
                     if(strcmp(args[0], "pwd") == 0){
                        char cwd[200];
                        if((getcwd(cwd, 200)) != NULL){
                            strcat(cwd, "\n");
                            write(STDOUT_FILENO, cwd, strlen(cwd));
                        }else{
                            write(STDERR_FILENO, error_message, strlen(error_message));
                            //write(STDERR_FILENO, "6\n", 2);
                        }

                        continue;
                    }

                    // exit command
                     if(strcmp(args[0], "exit") == 0) {
                         exit(0);
                     }



                    // NOT A BUILT IN COMMAND


                    //holds path to check
                    char* thisPath;


                    // attempt each path
                    int rc;
                    rc = fork();

                    if (rc == 0) {
                        // child

                        //check for redirect
                        if(redirect){

                            char *filename = strdup(args[numArgs - 1]);

                            //check for invalid characters
                            if ((strchr(filename, '/')) != NULL) {
                                write(STDERR_FILENO, error_message, strlen(error_message));
                                //write(STDERR_FILENO, "14\n", 3);
                                exit(1);
                            }


                                char *cpy;
                                cpy = strdup(args[numArgs-1]);
                                args[numArgs-1] = NULL;
                                args[numArgs-2] = NULL;
                                close(STDOUT_FILENO);
                                open(strcat(filename,".out"),
                                     O_CREAT | O_TRUNC | O_WRONLY,
                                     S_IRUSR | S_IWUSR);
                                close(STDERR_FILENO);
                                open(strcat(cpy,".err"),
                                     O_CREAT | O_TRUNC | O_WRONLY,
                                     S_IRUSR | S_IWUSR);

                        }



                       // char* tempArg = strdup(args[0]);
                        int i = 0;
                        int pathSize = 0;
                        for(i = 0; i < numPaths; i++) {

                            //calculate path size
                            pathSize += strlen(paths[i]);
                            pathSize += sizeof(char);
                            pathSize += strlen(args[0]);
                            //set path
                            thisPath = (char *) malloc(pathSize);
                            strcpy(thisPath, paths[i]);
                            strcat(thisPath, "/");
                            strcat(thisPath, args[0]);
                            //set args[0] to thisPath
                            args[0] = strdup(thisPath);


                            struct stat filestat;

                            if(stat(thisPath, &filestat) == 0) {

                                execv(thisPath, args);
                                // if execv returns, the process path failed
                            }
                        }
                        // if it gets to this point, all execv attempts failed
                        //print error and exit child process
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        //write(STDERR_FILENO, "8\n", 2);
                        exit(1);

                    } else if (rc > 0) {
                        // parent
                        // wait until a child returns
                        wait(NULL);



                    } else {
                        //fprintf(stderr, "fork failed\n");
                        write(STDERR_FILENO, error_message, strlen(error_message));
                        //write(STDERR_FILENO, "9\n", 2);
                        continue;

                    }

                } // end not build in command


            }else{
                write(STDERR_FILENO, error_message, strlen(error_message));
                //write(STDERR_FILENO, "10\n", 3);
            }


        }
        return 0;
    }
