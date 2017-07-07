# Shared Memory Management (SysV IPC)

## Synopsis

```
$ ./rpio shm help
arguments: MODE

MODE : create  KEY SIZE UGO
     | destroy ID
     | id      KEY SIZE
     | layout  ID
     | size    ID

ID   : a unique integer (returned by create and id)
KEY  : a unique IPC key as integer value
SIZE : size in bytes
UGO  : access permission as integer (0777)
```
