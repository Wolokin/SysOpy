#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int search_line(char* name, char* pattern) {
    int p1 = 0, p2 = 0;
    while (name[p1] != '\0' && name[p1] != '\n') {
        if (name[p1] == pattern[p2]) {
            p2++;
        } else {
            p2 = 0;
        }
        if (pattern[p2] == '\0' || pattern[p2] == '\n') {
            return 1;
        }
        p1++;
    }
    return 0;
}

int search_file(FILE* f, char* pattern) {
    char line[BUFSIZ];
    while (fgets(line, BUFSIZ, f)) {
        if (search_line(line, pattern)) {
            return 1;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 4) {
        printf("nie podano wszystkich argumentow\n");
        exit(1);
    }
    DIR* dir;
    if ((dir = opendir(argv[1])) == NULL) {
        printf("nie udalo sie otworzyc katalogu...\n");
    }
    int max_depth = strtol(argv[3], NULL, 10);
    int current_depth = 0;
    struct dirent* current;
    char txt[] = ".txt";
    char* pattern = argv[2];
    char relative_dir_path[BUFSIZ - 256] = ".";
    char relative_file_path[BUFSIZ];
    while ((current = readdir(dir)) != NULL) {
        char* name = current->d_name;
        sprintf(relative_file_path, "%s/%s", relative_dir_path, name);
        unsigned char t = current->d_type;

        if (t == DT_REG) {
            if (search_line(name, txt)) {
                FILE* f = fopen(relative_file_path, "r");
                if (search_file(f, pattern)) {
                    printf("PID: %d \tNazwa: %s\tKatalog: %s \n", getpid(),
                           name, relative_dir_path);
                }
                fclose(f);
            }
        } else if (t == DT_DIR && current_depth < max_depth &&
                   strcmp(name, ".") && strcmp(name, "..")) {
            pid_t pid = fork();
            if (pid == 0) {
                current_depth++;
                sprintf(relative_dir_path + strlen(relative_dir_path), "/%s",
                        name);
                closedir(dir);
                dir = opendir(relative_dir_path);
            }
        }
    }
    closedir(dir);
    while (wait(NULL) > 0)
        ;

    return 0;
}
