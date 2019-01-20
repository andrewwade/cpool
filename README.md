## Memory Pool Library
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
Used to manage variable sized blocks of memory. Has same limitations 
as malloc except the memory is reserved ahead of time so fragmentation
is better controlled.

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
```$xslt

```