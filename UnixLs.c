#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>



void list_directory(const char *path, int i_flag, int l_flag) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char time_string[20]; // For formatted time
    const char *directory = path ? path : "."; // If no path is given, use the current directory

    dir = opendir(directory);
    if (!dir) {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        // Skip hidden files if neither -i nor -l option is given
        if (entry->d_name[0] == '.' && !i_flag && !l_flag) continue;

        // If -l is specified, get more info with stat
        if (l_flag) {
            char full_path[1024];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);
            if (lstat(full_path, &file_stat) == -1) {
                perror("stat");
                continue;
            }



            // Format the time string
            // To simplify the printing of dates, you are to use the format
            // mmm dd yyyy hh:mm
            // regardless of the date
            strftime(time_string, sizeof(time_string), "%b %d %Y %H:%M", localtime(&file_stat.st_mtime));
        }

        // Print inode number if -i is specified
        // if (i_flag) {
        //     printf("%20lu", entry->d_ino);
        // }

        



        // Print additional info if -l is specified
        if (l_flag) {

            struct passwd *pw = getpwuid(file_stat.st_uid);
            struct group  *gr = getgrgid(file_stat.st_gid);

            char perm[11];
            snprintf(perm, sizeof(perm), "%c%c%c%c%c%c%c%c%c%c",
                    S_ISDIR(file_stat.st_mode) ? 'd' : '-',
                    file_stat.st_mode & S_IRUSR ? 'r' : '-',
                    file_stat.st_mode & S_IWUSR ? 'w' : '-',
                    file_stat.st_mode & S_IXUSR ? 'x' : '-',
                    file_stat.st_mode & S_IRGRP ? 'r' : '-',
                    file_stat.st_mode & S_IWGRP ? 'w' : '-',
                    file_stat.st_mode & S_IXGRP ? 'x' : '-',
                    file_stat.st_mode & S_IROTH ? 'r' : '-',
                    file_stat.st_mode & S_IWOTH ? 'w' : '-',
                    file_stat.st_mode & S_IXOTH ? 'x' : '-');

            printf("%8s %3ld %8s %8s", perm, (long)file_stat.st_nlink, pw->pw_name, gr->gr_name);


            printf("%8ld %20s", file_stat.st_size, time_string);
        }

        // Print file name
        printf("%30s", entry->d_name);

        printf("\n");
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    int opt;
    int i_flag = 0;
    int l_flag = 0;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "il")) != -1) {
        switch (opt) {
            case 'i':
                i_flag = 1;
                break;
            case 'l':
                l_flag = 1;
                break;
            default: /* '?' */
                fprintf(stderr, "Usage: %s [-i] [-l] [directory...]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }

    // List the current directory or specified directories

    if (optind == argc) {
        // No extra arguments were passed; list current directory
        list_directory(NULL, i_flag, l_flag);
    } else {
        // List directories provided as arguments
        for (; optind < argc; optind++) {
            list_directory(argv[optind], i_flag, l_flag);
        }
    }

    return 0;
}
