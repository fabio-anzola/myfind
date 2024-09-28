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
#include <linux/limits.h>

// Function which is called within each process to search filename in searchpath
void search_file(const char *searchpath, const char *filename, int recursive, int case_insensitive, int fd) {
    DIR *dir;
    struct dirent *entry;
    char path[PATH_MAX];
    char abs_path[PATH_MAX];
    char result[PATH_MAX + 2048]; // Buffer for pipe to parent
    
    // Open the searchpath directory and define dir
    if (!(dir = opendir(searchpath))) {
        fprintf(stderr,"Could not open searchpath directory %s\n", searchpath);
        return;
    }

    // Read files in directory
    while ((entry = readdir(dir)) != NULL) {
        // ignore "." and ".."
        // strcmp returns 0 on equal
        if (strcmp(entry->d_name, ".") == 0 || 
            strcmp(entry->d_name, "..") == 0)
            continue;

        // create full relative path for realpath function
        snprintf(path, sizeof(path), "%s/%s", searchpath, entry->d_name);

        // create absolute path for output
        if (realpath(path, abs_path) == NULL) {
            // printf("%s", path);
            // Enter here if absolute path could not be generated
            // fprintf(stderr,"Could not create absolute path");
            // likely could not find file in the current dir or permission error
            // skip
            continue;
        }
        
        // compare filename with current iteration of directory
        // case insensitive
        if ((case_insensitive && strcasecmp(entry->d_name, filename) == 0)) {
            // if file found then format output to result buffer
            snprintf(result, sizeof(result), "%d: %s: %s\n", getpid(), filename, abs_path);
            ssize_t write_ret = write(fd, result, strlen(result)); // write result to the pipe
            if (write_ret == -1) {
                perror("Could not write to pipe");
            }
        }
        // case sensitive
        else if ((!case_insensitive && strcmp(entry->d_name, filename) == 0)) {
            // if file found then format output to result buffer
            snprintf(result, sizeof(result), "%d: %s: %s\n", getpid(), filename, abs_path);
            ssize_t write_ret = write(fd, result, strlen(result)); // write result to the pipe
            if (write_ret == -1) {
                perror("Could not write to pipe");
            }
        }
        
        // if recursive flag is set
        if (recursive && entry->d_type == DT_DIR) {
            // re-call function with new directory
            search_file(abs_path, filename, recursive, case_insensitive, fd);
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

    // Check if searchpath and filenames have been entered
    if (optind >= argc) { // optind is set by getopt
        fprintf(stderr, "Expected searchpath and filenames\n");
        exit(EXIT_FAILURE);
    }

    // Extract searchpath from arguments
    searchpath = argv[optind];
    optind++;
    
    // Create a pipe for sync output
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("Could not create pipe");
        exit(EXIT_FAILURE);
    }

    for (int i = optind; i < argc; i++) {
        // Create fork for each filename
        pid_t pid = fork();
        
        if (pid == -1) { 
            // Error while forking
            perror("Failed to fork");
            exit(EXIT_FAILURE);
        }
        if (pid == 0) { 
            // Child Process
            close(pipefd[0]); // Close the reading end of the pipe
            search_file(searchpath, argv[i], recursive, case_insensitive, pipefd[1]);
            close(pipefd[1]); // Close the writing end of the pipe
            exit(0);
        }
        else {
            // Parent process
            // The parent process does nothing here
        }
    }

    // Close the writing end of the pipe in the parent process
    close(pipefd[1]);

    // Parent process reads from the pipe and prints the results
    char buffer[PATH_MAX + 2048];
    ssize_t bytes_read;
    while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytes_read] = '\0'; // Null-terminate the string
        printf("%s", buffer); // Output synchronously
    }

    // Close the reading end in the parent process
    close(pipefd[0]);

    // Non-blocking wait for child processes
    pid_t pid;
    while ((pid = waitpid(-1, NULL, WNOHANG))) {
        if ((pid == -1) && (errno != EINTR)) {
            break;
        }
    }
    
    return 0;
}
