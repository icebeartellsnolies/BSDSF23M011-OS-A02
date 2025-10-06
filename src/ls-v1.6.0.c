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

/* ---------- ANSI color codes ---------- */
#define ANSI_RESET    "\033[0m"
#define ANSI_BLUE     "\033[0;34m"
#define ANSI_GREEN    "\033[0;32m"
#define ANSI_RED      "\033[0;31m"
#define ANSI_MAGENTA  "\033[0;35m"
#define ANSI_REVERSE  "\033[7m"

typedef struct {
    char *name;
    mode_t mode;
    off_t size;
    int is_symlink;
} FileEntry;

/* ---------- Function Prototypes ---------- */
void mode_to_str(mode_t mode, char *str);
int get_term_width(void);
void display_horizontal(FileEntry entries[], int count);
void display_vertical(FileEntry entries[], int count);
void print_colored_padded(FileEntry *e, int col_width);
int is_tarball(const char *name);
int cmp_entry(const void *a, const void *b);
void do_ls(const char *dir, int long_flag, int horizontal_flag, int recursive_flag);

/* ---------- Main ---------- */
int main(int argc, char const *argv[])
{
    int long_flag = 0;
    int horizontal_flag = 0;
    int recursive_flag = 0;
    int opt;

    /* Parse -l, -x, and -R */
    while ((opt = getopt(argc, (char * const *)argv, "lxR")) != -1)
    {
        switch (opt)
        {
        case 'l': long_flag = 1; break;
        case 'x': horizontal_flag = 1; break;
        case 'R': recursive_flag = 1; break;
        default:
            fprintf(stderr, "Usage: %s [-l] [-x] [-R] [dir...]\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        do_ls(".", long_flag, horizontal_flag, recursive_flag);
    }
    else
    {
        int multiple = (argc - optind > 1);
        for (int i = optind; i < argc; i++)
        {
            if (multiple)
                printf("Directory listing of %s:\n", argv[i]);
            do_ls(argv[i], long_flag, horizontal_flag, recursive_flag);
            if (i < argc - 1)
                puts("");
        }
    }

    return 0;
}

/* ---------- Helper Functions ---------- */
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

int get_term_width(void)
{
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) return 80;
    return ws.ws_col > 0 ? ws.ws_col : 80;
}

int is_tarball(const char *name)
{
    if (!name) return 0;
    const char *ext = strrchr(name, '.');
    if (!ext) return 0;
    return (
        strcasecmp(ext, ".tar") == 0 ||
        strcasecmp(ext, ".gz") == 0 ||
        strcasecmp(ext, ".zip") == 0 ||
        strcasecmp(ext, ".tgz") == 0 ||
        strstr(name, ".tar.gz") != NULL
    );
}

void print_colored_padded(FileEntry *e, int col_width)
{
    const char *color = NULL;
    mode_t m = e->mode;

    if (e->is_symlink) color = ANSI_MAGENTA;
    else if (S_ISDIR(m)) color = ANSI_BLUE;
    else if (S_ISCHR(m) || S_ISBLK(m) || S_ISSOCK(m) || S_ISFIFO(m)) color = ANSI_REVERSE;
    else if (is_tarball(e->name)) color = ANSI_RED;
    else if (m & (S_IXUSR | S_IXGRP | S_IXOTH)) color = ANSI_GREEN;

    int len = (int)strlen(e->name);
    if (color) printf("%s%s%s", color, e->name, ANSI_RESET);
    else printf("%s", e->name);

    int pad = col_width - len;
    for (int i = 0; i < pad; ++i) putchar(' ');
}

void display_horizontal(FileEntry entries[], int count)
{
    int maxlen = 0;
    for (int i = 0; i < count; i++)
        if ((int)strlen(entries[i].name) > maxlen)
            maxlen = strlen(entries[i].name);

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
        print_colored_padded(&entries[i], col_width);
        curr_width += col_width;
    }
    printf("\n");
}

void display_vertical(FileEntry entries[], int count)
{
    int maxlen = 0;
    for (int i = 0; i < count; i++)
        if ((int)strlen(entries[i].name) > maxlen)
            maxlen = strlen(entries[i].name);

    int term_width = get_term_width();
    int col_width = maxlen + 2;
    int cols = term_width / col_width;
    if (cols < 1) cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++)
    {
        for (int c = 0; c < cols; c++)
        {
            int idx = c * rows + r;
            if (idx < count)
                print_colored_padded(&entries[idx], col_width);
        }
        printf("\n");
    }
}

int cmp_entry(const void *a, const void *b)
{
    const FileEntry *ea = a;
    const FileEntry *eb = b;
    return strcmp(ea->name, eb->name);
}

/* ---------- Recursive ls ---------- */
void do_ls(const char *dir, int long_flag, int horizontal_flag, int recursive_flag)
{
    DIR *dp = opendir(dir);
    if (!dp)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    printf("%s:\n", dir);  // header for recursive display

    errno = 0;
    size_t cap = 128, count = 0;
    FileEntry *entries = malloc(cap * sizeof(FileEntry));
    if (!entries)
    {
        perror("malloc");
        closedir(dp);
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.') continue;

        if (count + 1 >= cap)
        {
            cap *= 2;
            entries = realloc(entries, cap * sizeof(FileEntry));
        }

        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        struct stat st;
        if (lstat(path, &st) == -1) continue;

        entries[count].name = strdup(entry->d_name);
        entries[count].mode = st.st_mode;
        entries[count].size = st.st_size;
        entries[count].is_symlink = S_ISLNK(st.st_mode);
        count++;
    }
    closedir(dp);
    if (errno != 0) perror("readdir failed");

    if (count == 0)
    {
        printf("\n");
        free(entries);
        return;
    }

    qsort(entries, count, sizeof(FileEntry), cmp_entry);

    if (long_flag)
    {
        for (size_t i = 0; i < count; i++)
        {
            char path[PATH_MAX];
            snprintf(path, sizeof(path), "%s/%s", dir, entries[i].name);

            struct stat st;
            if (lstat(path, &st) == -1) continue;

            char perms[11];
            mode_to_str(st.st_mode, perms);

            struct passwd *pw = getpwuid(st.st_uid);
            struct group *gr = getgrgid(st.st_gid);

            char timebuf[64];
            struct tm *tm = localtime(&st.st_mtime);
            strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm);

            printf("%s %2ld %s %s %6lld %s ",
                   perms, (long)st.st_nlink,
                   pw ? pw->pw_name : "?",
                   gr ? gr->gr_name : "?",
                   (long long)st.st_size,
                   timebuf);
            print_colored_padded(&entries[i], 0);
            printf("\n");
        }
    }
    else if (horizontal_flag)
    {
        display_horizontal(entries, (int)count);
    }
    else
    {
        display_vertical(entries, (int)count);
    }

    /* ---------- Recursion ---------- */
    if (recursive_flag)
    {
        for (size_t i = 0; i < count; i++)
        {
            if (S_ISDIR(entries[i].mode))
            {
                if (strcmp(entries[i].name, ".") == 0 || strcmp(entries[i].name, "..") == 0)
                    continue;

                char subpath[PATH_MAX];
                snprintf(subpath, sizeof(subpath), "%s/%s", dir, entries[i].name);
                printf("\n");
                do_ls(subpath, long_flag, horizontal_flag, recursive_flag);
            }
        }
    }

    for (size_t i = 0; i < count; i++)
        free(entries[i].name);
    free(entries);
}

