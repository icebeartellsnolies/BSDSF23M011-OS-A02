FEATURE 2
Question 1 ans.
The crucial difference between stat() and lstat() lies in how they handle symbolic links (symlinks).
stat(): It follows symbolic links. If the path provided is a symlink, stat() returns information about the target file the link points to.
lstat(): It does not follow symbolic links. If the path is a symlink, lstat() returns information about the link itself (the link file's properties), not the file it references.
In the context of the ls command, it is generally more appropriate to use lstat() when listing files. This allows the program to detect and report on symbolic links specifically,
which is essential for the -l (long listing) format where symlinks are denoted with an l in the file type and their target is often displayed. Using stat() would hide the fact 
that the entry is a symbolic link, instead treating it as its target file type (e.g., a regular file or directory).

Question 2 ans.
Extracting File Type:
To determine the file type, you use the bitwise AND operator (&) with the S_IFMT macro, which is a mask for the file type bits.
The result of st_mode & S_IFMT isolates the file type bits.
You then compare this result against specific file type macros, such as S_IFREG (regular file), S_IFDIR (directory), S_IFLNK (symbolic link), etc.
Example: if ((statbuf.st_mode & S_IFMT) == S_IFDIR) checks if the file is a directory.
Extracting Permission Bits:

The permission bits are located in the lower 9 bits of st_mode.
You use the bitwise AND operator (&) with predefined permission macros to check if a specific permission bit is set.
For instance, S_IRUSR is a macro for the Owner Read permission bit.
Example: if (statbuf.st_mode & S_IRUSR) checks if the owner has read permission. Similarly, you can check S_IWGRP (Group Write), S_IXOTH (Others Execute), and so on.

FEATURE 3
Question 1 ans.
The logic for "down then across" (vertical) columnar printing involves calculating the number of rows needed based on the total number of items and the number of columns 
that can fit on the screen.

1. **Determine Parameters**: Identify the maximum filename length, terminal width, and total number of files.
2. **Calculate Columns**: Compute the maximum number of columns by dividing the terminal width by the maximum filename length plus necessary spacing.
3. **Calculate Rows**: Calculate the number of rows using the formula:  
   Rows = ceiling(Total Files / Columns).
4. **Nested Loop Iteration**: Use a nested loop structure where the outer loop iterates through rows and the inner loop iterates downward through items in each column.
   The index for the item in the i-th row and j-th column is:  Index = Row + (j × Rows).
6. **Why a Single Loop Fails**: A single loop iterates linearly through the list, printing items "across then down" (horizontal format). For "down then across" (vertical format),
   you need to print the 1st, 2nd, 3rd, etc., items of the first column, then move to the 1st, 2nd, 3rd, etc., items of the second column, and so on.
   This requires non-linear index jumps, which a single loop cannot achieve.

Question 2 ans.
The purpose of the ioctl (Input/Output Control) system call in this context is to retrieve the current size of the terminal window (console). Specifically, it is used
with the TIOCGWINSZ request, which fills a structure (struct winsize) with the terminal's dimensions in rows and columns. This information is crucial for calculating the 
optimal number of columns to use in the columnar display format.
If you only used a fixed-width fallback (e.g., 80 columns) instead of detecting the terminal size, the limitations of your program would be:
Inefficient Use of Space: If the user has a wide terminal (e.g., 200 columns), the program would only use 80, wasting screen real estate and potentially requiring unnecessary 
vertical scrolling.
Poor Display on Narrow Terminals: If the user has a very narrow terminal (e.g., 40 columns), the program would attempt to fit the display into 80 columns, leading to mangled 
output as lines would wrap or be truncated awkwardly, rendering the columnar display useless.
Lack of Adaptability: The display would not adapt if the user resized the terminal window while the program was running, leading to an immediate mismatch between
the expected layout and the actual screen size.

FEATURE 4
Question 1 ans.
The "down then across" (vertical) printing logic is notably more complex and demands more upfront calculations compared to the "across" (horizontal) logic. 
Horizontal printing is simpler, requiring only tracking of the current column position or character count and inserting a newline when the terminal width is exceeded.
In contrast, vertical printing involves pre-calculating the maximum filename length, the number of columns that fit on the screen, and the number of rows. 
These calculations are essential to determine the non-linear index formula, Index = Row + (j × Rows), which governs the column-by-column jumps within the loop structure. 
While horizontal printing can process items linearly, making decisions per item, vertical printing requires all item counts, maximum lengths, and screen dimensions to be 
computed before printing can start.

Question 2 ans.
The strategy to manage different display modes typically involves:
Command-Line Option Parsing: Using a function (like getopt()) to parse command-line arguments and set corresponding global flags or an enumeration variable (e.g., DISPLAY_MODE).
If -l is present, set the mode to LONG_FORMAT.
If -x is present, set the mode to ACROSS_FORMAT.
If neither is present, default to VERTICAL_FORMAT (down then across).
Central Dispatch: Using a switch statement or a series of if/else if checks on the DISPLAY_MODE variable after all input files/directories have been processed.
Specialized Functions: Defining a specific printing function for each mode:
print_long_format() (for -l)
print_across_format() (for -x)
print_vertical_format() (for default)
The program decides which function to call by using the central dispatch mechanism (the switch statement). Based on the value of the DISPLAY_MODE variable set during the 
initial argument parsing, execution branches to the appropriate, specialized printing function responsible for formatting and displaying the data for all files and directories.

FEATURE 5
Question 1 ans.
To sort directory entries, all entries must be read into memory first because directories are typically stored as unsorted data structures, like linked lists or 
unindexed arrays, on the filesystem. The `readdir()` system call retrieves entries in their stored order—often based on creation or modification time—rather than in a 
sorted (e.g., alphabetical) order.
For a sorted output, as is standard with the `ls` command, the program needs the full set of entries to:
- Compare each entry with all others.
- Reorganize the entire collection into the desired order (e.g., by name, size, or time).
**Challenges with Directories Containing Millions of Files**:
The main issue is memory exhaustion.
- **High Memory Usage**: Loading millions of file entries, including filenames and possibly `struct stat` data, into an array or similar structure demands significant RAM.
- **Performance Bottleneck**: Sorting large datasets (with a time complexity of O(N log N) for efficient algorithms like quicksort) becomes time-intensive as the number of files (N)
  reaches millions, delaying output.
- **Thrashing**: If memory needs exceed available RAM, the system resorts to swap space, causing severe performance degradation due to thrashing.

Question 2 ans.
Purpose and Signature
The purpose of the comparison function required by qsort() is to define the sorting criteria (e.g., alphabetical ascending, numerical descending). It determines the 
relative order of any two elements in the array being sorted. Its signature must strictly adhere to:
int comparison_function(const void *a, const void *b);

FEATURE 6
Question 1 ans.
The specific SGR code for setting the **standard foreground green color** is $\mathbf{32}$.
The full code sequence to print text in green is:
* `\033[`: Control Sequence Introducer (CSI)
* `32`: Parameter for **Foreground Color Green**
* `m`: SGR (Select Graphic Rendition) command
To print the word "Hello" in green and then **reset** the color (crucial to prevent subsequent text from also being green), you would use:
printf("\033[32mHello\033[0m\n");

Question 2 ans.
To determine if a file is executable by the owner, group, or others, you need to check the following specific bits in the **`st_mode`** field of the `struct stat`, 
using the **bitwise AND operator (`&`)** with the corresponding predefined macros:
| Executable By | `st_mode` Bit (Octal) | Predefined Macro | Condition Check |
| :--- | :--- | :--- | :--- |
| **Owner (User)** | $9^{th}$ bit (0100) | **`S_IXUSR`** | `(statbuf.st_mode & S_IXUSR)` |
| **Group** | $6^{th}$ bit (0010) | **`S_IXGRP`** | `(statbuf.st_mode & S_IXGRP)` |
| **Others** | $3^{rd}$ bit (0001) | **`S_IXOTH`** | `(statbuf.st_mode & S_IXOTH)` |

A file is considered executable for coloring purposes if **any** of these three bits is set. Therefore, the overall check often looks like:
if (statbuf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) {
    // The file is executable by at least one entity
}

FEATURE 7
Question 1 ans.
In a recursive function, the **base case** is the **non-recursive condition** that **stops the function from calling itself further**. It is the point at which the recursion 
unwinds and returns a result. Without a base case, or with an improperly defined one, a recursive function will lead to an infinite loop, eventually causing a **stack overflow**.
In the context of a recursive `ls` (implementing the `-R` option), the primary base case that stops the recursion from continuing forever is the condition where the function
encounters an entry that **is not a directory**.

The recursive `do_ls()` function will call itself only if the current entry it processes is a directory (and meets other criteria like not being "." or "..").
Therefore, the base cases (the points where recursion stops) are when `do_ls()` processes an entry that is:
* A **regular file**, symbolic link, block device, etc. (i.e., **not a directory**). The function simply prints its details and returns.
* The **current directory** (`.`) or **parent directory** (`..`). These are explicitly skipped to prevent infinite or redundant loops.
* A **directory for which the user lacks read permission** (an error condition).

The most fundamental base case is encountering a **non-directory file type**.

Question 2 ans.
It is essential to construct a **full, absolute, or relative path** (e.g., `"parent_dir/subdir"`) before making a recursive call because system calls like `stat()`,`lstat()`
, and `opendir()` typically resolve paths **relative to the process's current working directory (CWD)**, *not* relative to the directory currently being processed by the function.
When `do_ls()` is called, it might open a directory like `"parent_dir"`. Inside this call, it finds an entry named `"subdir"`. The process's CWD often remains unchanged
(e.g., the directory where the user initially ran the `ls -R` command).
If you construct the full path (`"parent_dir/subdir"`), the next recursive call, `do_ls("parent_dir/subdir")`, correctly points to the intended directory regardless of the CWD.
If you simply called `do_ls("subdir")` from within `do_ls("parent_dir")`, the program would attempt to open a directory named `"subdir"` **relative to the process's CWD**.

This could lead to two critical errors:

1.  **Failure:** If a directory named `"subdir"` does not exist in the CWD, the call to `opendir("subdir")` will fail, and the recursion chain will be broken for that branch.
2.  **Incorrect Recursion/Infinite Loop:** If a directory named `"subdir"` *does* happen to exist in the CWD, the program will process that **wrong directory**. In a worst-case scenario, if the CWD contains a directory with the same name as one of the recursive subdirectories, it could lead to the program jumping to a completely different part of the filesystem or even triggering an **infinite loop** if the path structure is circular.

By constructing the full path, you ensure that the recursive call is operating on the correct, fully qualified location in the file system, independent of the process's CWD.

