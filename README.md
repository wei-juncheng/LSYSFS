Less Simple, Yet Stupid Filesystem (Using FUSE)
=======================================

This is an example of using FUSE to build a simple in-memory filesystem that supports creating new files and directories. It is a part of a tutorial in MQH Blog with the title "Writing Less Simple, Yet Stupid Filesystem Using FUSE in C": <http://maastaar.net/fuse/linux/filesystem/c/2019/09/28/writing-less-simple-yet-stupid-filesystem-using-FUSE-in-C/>

## Prerequire
- Linux
- gcc
- FUSE install
    - Ubuntu:
        ```
        $ sudo apt-get update && sudo apt-get install libfuse-dev -y
        ```
## Environment Setup
1. Download code
```
$ git clone https://github.com/wei-juncheng/LSYSFS.git
```
```
$ cd LSYSFS
```
2. compile
```
$ make
```

3. create a directory and mount
```
$ mkdir mount_dir
```
```
$ ./lsysfs -f mount_dir
```
- keep this terminal, don't close it!
- Now it is running LSYSFS, but you can't interact with him in this terminal. Please open another terminal window.

4. open another terminal and using LSYSFS
```
$ cd mount_dir
```
- Now you can use some command like `ls`, `mkdir`, `touch`...
 
## Notice
- This project only store in memory, so after you close the program, the file that you created will disappear!



License: GNU GPL.
