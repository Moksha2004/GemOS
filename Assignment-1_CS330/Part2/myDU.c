#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include<string.h>
#include<sys/wait.h>
//int calculate_directory_size(const char *path);
int calculate_directory_size(const char *path) {
    DIR *directory = opendir(path);
      int total = 0;
    if (directory== NULL) {
        perror("opendir");
        return -1;
    }
      struct stat s1;
    
    if (lstat(path, &s1) != 0) {
        perror("lstat");
        closedir(directory);
        return -1;
    }
    total += s1.st_size;
     struct dirent *start;
     
    // Looping through all entries in the directory
    while ((start = readdir(directory)) != NULL) {
        if (strcmp(start->d_name, ".") == 0) continue;
        if( strcmp(start->d_name, "..") == 0) continue;

        char entry[PATH_MAX];
        snprintf(entry, sizeof(entry), "%s/%s", path, start->d_name);
        struct stat st;
        // Getting information about the entry
        if (lstat(entry, &st) != 0) {
            perror("lstat");
            closedir(directory);
            return -1;
        }
         // Handling symbolic links.
        if (S_ISLNK(st.st_mode)) {
            char visited[PATH_MAX];
            ssize_t size = readlink(entry, visited, sizeof(visited) - 1);
            if (size == -1) {
                perror("readlink");
                closedir(directory);
                return -1;
            }
            visited[size] = '\0';

            struct stat target;
            if (lstat(visited, &target) != 0) {
                perror("lstat");
                closedir(directory);
                return -1;
            }
             // If the target of the symbolic link is a directory
            if (S_ISDIR(target.st_mode)) {
                int size1 = calculate_directory_size(visited);
                total += size1;
            } 
            else if (S_ISREG(target.st_mode)) {   //  regular file, add its size to the total.
               total += target.st_size;
            }
        }
         // Handle subdirectories.
        else if (S_ISDIR(st.st_mode)) {
            int pipe_fd[2];
            if (pipe(pipe_fd) == -1) {
                perror("pipe");
                closedir(directory);
                return -1;
            }
            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                closedir(directory);
                return -1;
                
            }
            if (!pid) { // Child 
            // calculate the size of the subdirectory.
                close(pipe_fd[0]);
                int size_dir = calculate_directory_size(entry);
                write(pipe_fd[1], &size_dir, sizeof(int)); 
                close(pipe_fd[1]); 
                exit(0);
            }
            else {//Parent
            // read the size calculated by the child and add it to the total.
                close(pipe_fd[1]); 
                int size_dir;
                read(pipe_fd[0], &size_dir, sizeof(int));
                close(pipe_fd[0]); 
                waitpid(pid, NULL, 0); 
                total += size_dir;
            }
            
        }
       else if (S_ISREG(st.st_mode)) total += st.st_size;//Regular file
        
    }
    closedir(directory);
    return total;
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <relative path to directory>\n", argv[0]);
        return 1;
    }

    const char *root = argv[1];
    int total = calculate_directory_size(root);
    if (total >= 0)  printf("%d\n", total);
    else {
        perror("Error");
        return -1;
    }

    return 0;
}


