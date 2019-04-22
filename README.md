### Completely implemented in C ### 

Flag | Description 
-----|------------
`-b [LONG]` | Specifies the size of a block in the system memory.
`-s [LONG]` | Specifies the capacity of the system memory.

Note that files provided with `-d` must be built by running `find [DIR] -type d > [FILENAME]` with a directory, or must be shaped to have the same format. Similarly, files provided with `-f` must be built with `find [DIR] -type f -ls > [FILENAME]` with a directory of choice. If a path cannot be resolved for some of the files, they will not be added.

The default sizes for the simulated filesystem are to use 512B blocks with a 64kB capacity. If a block size or a disk size are not given, a warning will be thrown to notify the user of the default values. If files are too big to fit in remaining space, an error will be thrown and the file will be skipped.

### How do I use the filesystem? ###

The file system simulator comes with the ability to run several bash-like commands. These include:

Command | Description
--------|------------
`cd [DIR (optional)]` | Changes directory. Acts nearly identically to bash.
`ls [DIR (optional)]` | Lists the contents of the directory. Acts similarly to `ls -al`.
`mkdir [DIR1] [DIR2] ...` | Creates a set of directories. Acts nearly identically to bash.
`create [FILE1] [FILE2] ...` | Creates a set of files. Will not create file if owner directory doesn't exist.
`append [FILE] [LONG]` | Appends a number of bytes to a file.
`remove [FILE] [LONG]` | Remove a number of bytes from a file.
`delete [FILE1] [FILE2] ...` | Removes a set of files or directories.
`exit [NUM (optional)]` | Exits the system. Optionally return an error code.
`dir [PATH (optional)]` | Does a BFS print of the file system.
`prfiles [PATH (optional)]` | Does a BFS print of all files with space analysis.


