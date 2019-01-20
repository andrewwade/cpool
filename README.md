## Memory Pool Library
Memory resource management in C. For use on small embedded systems.

### Block Pool
Used to manage blocks of memory that have the same size. Knowing the 
block size ahead of time allows for faster allocation and removes 
fragmentation issues associated with typical malloc functionality.

### Byte Pool
Used to manage variable sized blocks of memory. Has same limitations 
as malloc except the memory is reserved ahead of time so fragmentation
is better controlled.

