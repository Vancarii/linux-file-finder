#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include <getopt.h>
#include <pwd.h>
#include <grp.h>
#include <stdbool.h>
#include <string.h>

// Some difference between this program and the regular ls:
// 
// 1. The time format is different. The regular ls uses the format mmm dd hh:mm
// while this program uses mmm dd yyyy hh:mm regardless of the date.

// 2. If input is:
//   ./UnixLs -iW 
// This program will consider the W as a directory or file input
// So this program will look for the directory W, and return error if not found
// while the regular ls command will consider W as a flag and provide an error

// This program provides more spacing for each column for the unknown sized variables like
// user name, group name, and file size. This is to make the output more readable.

// To mimic the ls command, This program does not print theinformation for . and .. and any
// directory/files that start with .


// function to print the additional information when -l is specified
// since we require this process for both directories and files
void print_l_flag(struct stat file_stat, const char *file, char time_string[20]) {

    // Print additional info if -l is specified
    struct passwd *pw = getpwuid(file_stat.st_uid);
    struct group  *gr = getgrgid(file_stat.st_gid);

    if (!pw || !gr) {
        perror("UnixLs: cannot get user or group name");
    }

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

    printf("%11s %-2ld %7s %7s", perm, (long)file_stat.st_nlink, pw->pw_name, gr->gr_name);

    printf("%8ld %-17s", file_stat.st_size, time_string);

    // Print file name
    printf(" %-30s", file);

}

// function to use to print file info
// when the user inputs a file and not a directory, we only print that 
// one file information without looking for the directory
void print_file_info(const char *file, int i_flag, int l_flag) {
    struct stat file_stat;
    char time_string[20]; // For formatted time

    if (lstat(file, &file_stat) == -1) {
        perror("stat");
        return;
    }

    // Format the time string
    // To simplify the printing of dates, you are to use the format
    // mmm dd yyyy hh:mm
    // regardless of the date
    strftime(time_string, sizeof(time_string), "%b %d %Y %H:%M", localtime(&file_stat.st_mtime));

    // Print inode number if -i is specified
    if (i_flag) {
        printf("%17lu", file_stat.st_ino);

        if (!l_flag) {
            printf(" %-30s\n", file);
        }
    }

    if (l_flag) {
        print_l_flag(file_stat, file, time_string);
    }


}



void list_directory(const char *path, int i_flag, int l_flag) {
    DIR *dir;
    struct dirent *entry;
    struct stat file_stat;
    char time_string[20]; // For formatted time
    const char *directory = path ? path : "."; // If no path is given, use the current directory


    // Check if the input is valid
    if (stat(directory, &file_stat) == -1) {
        perror("error");
        return;
    }

    // check if the path is a file 
    if (S_ISREG(file_stat.st_mode)) {
        print_file_info(directory, i_flag, l_flag);
        return;
    }

    dir = opendir(directory);
    if (!dir) {
        perror("UnixLs: cannot access directory");
        return;
    }

    // this is to count up to 3 prints before printing a new line
    int i_flag_count = 0;

    while ((entry = readdir(dir)) != NULL) {

        // skip the . and .. directory
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0 || entry->d_name[0] == '.') {
            continue;
        }

        // Print inode number if -i is specified
        // print 3 files/columns per line
        if (i_flag) {
            printf("%17lu", entry->d_ino);

            if (!l_flag) {

                i_flag_count++;
                // Print file name
                printf(" %-30s", entry->d_name);

                if (i_flag_count == 3) {
                    i_flag_count = 0;
                    printf("\n");
                }
            }

        }


        // If -l is specified, get more info with stat
        if (l_flag) {
            char full_path[1024];
            int ret = snprintf(full_path, sizeof(full_path), "%s/%s", directory, entry->d_name);

            // length check
            if (ret < 0 || ret >= sizeof(full_path)) {
                perror("UnixLs: path is too long");
                continue;
            }

            // Get file info
            if (lstat(full_path, &file_stat) == -1) {
                perror("error");
                closedir(dir);
                continue;
            }

            // Format the time string
            strftime(time_string, sizeof(time_string), "%b %d %Y %H:%M", localtime(&file_stat.st_mtime));
        
            print_l_flag(file_stat, entry->d_name, time_string);
            printf("\n");
        
        }



        // formatting

        if (!i_flag && !l_flag) {
            printf("%s   ", entry->d_name);
        }
    }

    if (!i_flag && !l_flag) {
        printf("\n");
    }

    closedir(dir);
}

int main(int argc, char *argv[]) {
    int opt;
    int i_flag = 0;
    int l_flag = 0;

    // Disables getopt error messages
    // since we want to interpret the wrong flags as directories or files
    opterr = 0;

    bool no_flag = false;

    // Parse command-line arguments
    while ((opt = getopt(argc, argv, "il")) != -1) {
        switch (opt) {
            case 'i':
                i_flag = 1;
                break;
            case 'l':
                l_flag = 1;
                break;
            case '?':
                no_flag = true;
                // If an unknown option is given, treat it as a directory input
                list_directory(argv[optind - 1], i_flag, l_flag);
                break; 
            default:
        }
    }

    // List the current directory or specified directories

    if (no_flag == false) {
        if (optind == argc) {
            // No extra arguments were passed
            // use the current directory
            list_directory(NULL, i_flag, l_flag);
        } else {
            // List directories provided as arguments
            for (; optind < argc; optind++) {
                list_directory(argv[optind], i_flag, l_flag);
            }
        }
    }

    return 0;
}
