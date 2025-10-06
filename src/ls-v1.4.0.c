#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <limits.h>

extern int errno;

void mode_to_str(mode_t mode, char *str);
void display_horizontal(char *names[], int count);
int get_term_width(void);
void do_ls(const char *dir, int long_flag, int horizontal_flag);
int cmp_strptr(const void *a, const void *b);

/* ---------- main ---------- */
int main(int argc, char const *argv[])
{
    int long_flag = 0;
    int horizontal_flag = 0;
    int opt;

    /* parse options: -l and -x */
    while ((opt = getopt(argc, (char * const *)argv, "lx")) != -1)
    {
        switch (opt)
        {
        case 'l':
            long_flag = 1;
            break;
        case 'x':
            horizontal_flag = 1;
            break;
        default:
            fprintf(stderr, "Usage: %s [-l] [-x] [dir...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        do_ls(".", long_flag, horizontal_flag);
    }
    else
    {
        int multiple = (argc - optind > 1);
        for (int i = optind; i < argc; i++)
        {
            if (multiple)
                printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i], long_flag, horizontal_flag);
            if (i < argc - 1)
                puts("");
        }
    }

    return 0;
}

/* ---------- helpers ---------- */

/* Convert mode to rwx string + type */
void mode_to_str(mode_t mode, char *str)
{
    str[0] = S_ISDIR(mode) ? 'd' :
             S_ISLNK(mode) ? 'l' :
             S_ISCHR(mode) ? 'c' :
             S_ISBLK(mode) ? 'b' :
             S_ISFIFO(mode)? 'p' :
             S_ISSOCK(mode)? 's' : '-';

    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

/* Get terminal width: try ioctl, fallback to 80 */
int get_term_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
        return 80;
    if (ws.ws_col > 0)
        return ws.ws_col;
    return 80;
}

/* Horizontal (row-major) display */
void display_horizontal(char *names[], int count)
{
    if (count == 0) { printf("\n"); return; }

    int maxlen = 0;
    for (int i = 0; i < count; i++)
        if ((int)strlen(names[i]) > maxlen)
            maxlen = strlen(names[i]);

    int term_width = get_term_width();
    int col_width = maxlen + 2;
    int curr_width = 0;

    for (int i = 0; i < count; i++)
    {
        if (curr_width + col_width > term_width)
        {
            printf("\n");
            curr_width = 0;
        }
        printf("%-*s", col_width, names[i]);
        curr_width += col_width;
    }
    printf("\n");
}

/* Comparison function for qsort: compare two char* strings alphabetically */
int cmp_strptr(const void *a, const void *b)
{
    const char *const *pa = a;
    const char *const *pb = b;
    return strcmp(*pa, *pb);
}

/* ---------- main listing function ---------- */
/* Collect entries, sort alphabetically, then dispatch to display */
void do_ls(const char *dir, int long_flag, int horizontal_flag)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;

    /* dynamic array for names */
    size_t cap = 128;
    size_t count = 0;
    char **names = malloc(cap * sizeof(char *));
    if (!names) {
        perror("malloc");
        closedir(dp);
        return;
    }

    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (count + 1 >= cap) {
            cap *= 2;
            char **tmp = realloc(names, cap * sizeof(char *));
            if (!tmp) {
                perror("realloc");
                break;
            }
            names = tmp;
        }

        names[count] = strdup(entry->d_name);
        if (!names[count]) { perror("strdup"); break; }
        count++;
    }

    if (errno != 0)
    {
        perror("readdir failed");
        for (size_t i = 0; i < count; i++) free(names[i]);
        free(names);
        closedir(dp);
        return;
    }

    closedir(dp);

    if (count == 0) {
        free(names);
        return;
    }

    /* SORT alphabetically using qsort */
    qsort(names, count, sizeof(char *), cmp_strptr);

    /* If long listing requested -> print detailed info */
    if (long_flag)
    {
        for (size_t i = 0; i < count; i++)
        {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dir, names[i]);

            struct stat st;
            if (lstat(path, &st) == -1)
            {
                perror("lstat");
                continue;
            }

            char perms[11];
            mode_to_str(st.st_mode, perms);

            struct passwd *pw = getpwuid(st.st_uid);
            struct group  *gr = getgrgid(st.st_gid);

            char timebuf[64];
            struct tm *tm = localtime(&st.st_mtime);
            if (tm)
                strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);
            else
                strncpy(timebuf, "??? ?? ??:??", sizeof(timebuf));

            printf("%s %2ld %s %s %6lld %s %s\n",
                   perms,
                   (long)st.st_nlink,
                   pw ? pw->pw_name : "?",
                   gr ? gr->gr_name : "?",
                   (long long)st.st_size,
                   timebuf,
                   names[i]);
        }
    }
    else if (horizontal_flag)
    {
        display_horizontal(names, (int)count);
    }
    else
    {
        /* default vertical "down then across" */
        int maxlen = 0;
        for (size_t i = 0; i < count; i++)
            if ((int)strlen(names[i]) > maxlen)
                maxlen = strlen(names[i]);

        int term_width = get_term_width();
        int col_width = maxlen + 2;
        if (col_width <= 0) col_width = 1;
        int cols = term_width / col_width;
        if (cols < 1) cols = 1;
        int rows = (count + cols - 1) / cols;

        for (int r = 0; r < rows; r++)
        {
            for (int c = 0; c < cols; c++)
            {
                int idx = c * rows + r;
                if (idx < (int)count)
                    printf("%-*s", col_width, names[idx]);
            }
            printf("\n");
        }
    }

    for (size_t i = 0; i < count; i++)
        free(names[i]);
    free(names);
}


