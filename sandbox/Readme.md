# HW2 - Sandbox
Practice of library injection and API hijacking. 

## Build
```
make
```    
## Usage
```
./sandbox [-p sopath] [-d basedir] [--] cmd [cmd args ...]
```
## Functions monitoring
* For the functions below, my shared library will complete the request if the access is allowed. Otherwise, reject the request and print out error message.
    * __xstat
    * __xstat64
    * chdir
    * chmod
    * chown
    * creat
    * fopen
    * link
    * mkdir
    * open
    * openat
    * opendir
    * readlink
    * remove
    * rename
    * rmdir
    * symlink
    * symlinkat
    * unlink
* For the functions below, my shared library will reject the request directly.
    * execl
    * execle
    * execlp
    * execv
    * execve
    * execvp
    * system