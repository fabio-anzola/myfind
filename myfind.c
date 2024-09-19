#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    int recursive = 0;
    int case_insensitive = 0;
    int opt;
    char *searchpath = NULL;
    
    // Parse option arguments with getopt
    while ((opt = getopt(argc, argv, "Ri")) != EOF) {
        switch (opt) {
            case 'R':
                recursive = 1;
                break;
            case 'i':
                case_insensitive = 1;
                break;
            default:
                fprintf(stderr, "Usage: %s [-R] [-i] searchpath filename1 [filename2 ...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // Chech if searchpath and filenames have been entered
    if (optind >= argc) { // optind is set by getopt
        fprintf(stderr, "Expected searchpath and filenames\n");
        exit(EXIT_FAILURE);
    }

    // Extract searchpath from arguments
    searchpath = argv[optind];
    optind++;
    
    for (int i = optind; i < argc; i++) {
        pid_t pid = fork();
        
        if (pid == -1) { 
            // Error while Forking
            perror("Failed to fork");
            exit(EXIT_FAILURE);
            return 1;
        }
        if (pid == 0) { 
            // Child Process
            // TODO: search for files
            printf("I'm a child");
            exit(0);
        }
        else {
            // Parent process            
        }
    }

}
