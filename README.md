## Memory Pool Library
[![Build Status](https://travis-ci.com/andrewwade/cpool.svg?branch=master)](https://travis-ci.com/andrewwade/cpool)
Memory resource management in C. For use on small embedded systems.

### Block Pool
Used to manage blocks of memory that have the same size. Knowing the 
block size ahead of time allows for faster allocation and removes 
fragmentation issues associated with typical malloc functionality.

Initializing a Memory Block Pool:
```c
uint8_t buffer[512];
block_pool_t block_pool;
block_pool_init(&block_pool, sizeof(some_struct), buffer, buffer+512);
```

Allocating and Releasing Memory Block:
```c
struct some_struct *obj = block_allocate(&block_pool);
do_stuff(obj);
block_release(obj);
```

### Byte Pool
Used to manage fixed size blocks of memory. Has same limitations 
as malloc except the memory is reserved ahead of time so fragmentation
is better controlled. Size and ownership of every byte block is 
preserved in an inline memory header which adds memory overhead.

Initializing a Memory Byte Pool:
```c
uint8_t buffer[512];
byte_pool_t byte_pool;
byte_pool_init(&byte_pool, sizeof(some_struct), buffer, buffer+512);
```

Allocating and Releasing a Memory Byte Block:
```c
struct some_struct *obj = byte_allocate(&byte_pool, sizeof(some_struct));
do_stuff(obj);
byte_release(obj);
```

### Segment Pool
Used to manage fixed or custom length segments of memory. This pool
doesn't use any inline memory so users must keep track of each 
segment's size in order to release them properly.

Initializing Memory Segment Pool:
```c
segment_pool_t segment_pool;
struct some_struct buffer[16];
segment_pool_init(&segment_pool, sizeof(some_struct), buffer, buffer + 16);
```
Allocating and Releasing a Fixed Size Memory Segment:
```c
struct some_struct *obj = segment_allocate(&segment_pool);
do_stuff(obj);
segment_release(&segment_pool, obj);
```
Allocating and Releasing a Custom Size Memory Segment:
```c
int buffer[256];
segment_pool_t segment_pool;
segment_pool_init(&segment_pool, sizeof(int), buffer, buffer + 256);
// segment pool aligned to sizeof(int) but sizeof(some_struct) > sizeof(int)
struct some_struct *obj = segment_allocate_size(&segment_pool, sizeof(some_struct));
do_stuff(obj);
segment_release(&segment_pool, obj, sizeof(some_struct));
```
