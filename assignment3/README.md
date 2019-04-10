## Included files
#### multi-lookup.c
Main file that runs the program

#### ArrayBuffer.c </br>
Object that is shared between threads for pushing and popping data between requesters and resolvers

#### FileBuffer.c
Object that keeps track of which files have been read and assists in the Round Robin order in which threads that finish start helping the next file

#### compile.sh
Used to compile the program

## Compiling Program
Run make

## Usage
```
multi-lookup <# requester> <# resolver> <requester log> <resolver log> [ <data file> ...]
```
