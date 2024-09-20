#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/errno.h>

void search_file(const char *searchpath, const char *filename, int recursive, int case_insensitive) {
    DIR *dir;
    struct dirent *entry;
    char path[1024];
    char abs_path[1024];
    
    // Open the searchpath directory and define dir
    if (!(dir = opendir(searchpath))) {
        perror("Could not open searchpath directory");
        return;
    }

    // Read files in directory
    while ((entry = readdir(dir)) != NULL) {
        // ignore "." and ".."
        // strcomp returns 0 on equal
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0)
            continue;

        // create relative path for realpath function
        snprintf(path, sizeof(path), "%s/%s", searchpath, entry->d_name);

        // create absolute path for output (searchpath + "/" + entry->d_name)
        if (realpath(path, abs_path) == NULL) {
            // Enter here if absolute path could not be entered
            // perror("Could not create absolute path");
            continue;
        }
        
        // compare filename with current iteration of directory
        // case sensitive
        if ((case_insensitive && strcasecmp(entry->d_name, filename) == 0)) {
            // if file found then output to stdout
            printf("%d: %s: %s\n", getpid(), filename, abs_path);
        }
        // compare filename with current iteration of directory
        // case insensitive
        else if ((!case_insensitive && strcmp(entry->d_name, filename) == 0)) {
            // if file found then output to stdout
            printf("%d: %s: %s\n", getpid(), filename, abs_path);
        }
        
        // if recursive flag is set
        if (recursive && entry->d_type == DT_DIR) {
            // re-call function with new directory
            search_file(abs_path, filename, recursive, case_insensitive);
        }
    }

    // Close the directory to free the resource
    closedir(dir);
}

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
            search_file(searchpath, argv[i], recursive, case_insensitive);
            exit(0);
        }
        else {
            // Parent process            
        }
    }

    // Non-Blocking lookup to wait for children
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG))) {
        if ((pid == -1) && (errno != EINTR)) {
            break;
        }
    }
    
    return 0;

}
