# Custom Memory Allocator in C

A simple educational implementation of a dynamic memory allocator written entirely in C.

The project recreates the basic functionality of the standard C allocation routines without relying on the system heap.

Implemented functions:

- `heap_malloc()`
- `heap_free()`
- `heap_calloc()`
- `heap_realloc()`

The allocator manages its own fixed-size heap using a linked list of memory blocks.

---

## Features

- Fixed-size heap (4 KB)
- 8-byte memory alignment
- Block splitting during allocation
- Block coalescing during deallocation
- Pointer validation
- Double-free detection
- `calloc()` overflow protection
- Heap visualization for debugging

---

## Heap Layout

```
+---------+----------------------+
| Header  | User payload         |
+---------+----------------------+

Header:
- block size
- free/used flag
- pointer to next block
```

Each allocated block contains a metadata header followed by the user-accessible memory.

---

## Allocation Strategy

Current implementation uses the **First Fit** algorithm.

Allocation process:

1. Traverse the linked list.
2. Find the first free block large enough.
3. Split the block if the remaining space is sufficient.
4. Return a pointer to the payload.

---

## Deallocation

`heap_free()` performs:

- pointer validation
- double-free detection
- forward block coalescing
- backward block coalescing

to reduce fragmentation.

---

## Current Limitations

- Fixed heap size
- Single-threaded
- No memory protection
- First Fit only
- Shrinking `realloc()` is not yet implemented

---

## Future Improvements

- Best Fit
- Next Fit
- Heap statistics
- Fragmentation metrics
- Unit tests
- Thread safety
- Configurable heap size

---

## Example

```c
heap_init();

void *a = heap_malloc(100);
void *b = heap_malloc(200);

heap_free(a);

b = heap_realloc(b, 500);

dump_heap();
```

---

## Example Output

```
========================= HEAP STATUS =========================

Total memory : 4096 B 
User payload : 504 B 
Metadata     : 72 B 
Free memory  : 3496 B 
Blocks       : 3 

===============================================================

+--------------+--------+--------+--------------+--------------+
| Address      | Size   | Free   | Payload      | Next         |
+--------------+--------+--------+--------------+--------------+
| 0x1021fc000  | 104    | YES    | 0x1021fc018  | 0x1021fc080  |
| 0x1021fc080  | 504    | NO     | 0x1021fc098  | 0x1021fc290  |
| 0x1021fc290  | 3392   | YES    | 0x1021fc2a8  | 0x0          |
+--------------+--------+--------+--------------+--------------+

===============================================================
```

---

## Build

```bash
gcc src/main.c -o allocator
```

Run:

```bash
./allocator
```

---

## Purpose

This project was created for educational purposes to better understand:

- dynamic memory allocation
- memory layout
- pointer arithmetic
- linked lists
- low-level systems programming

It is intended as preparation for embedded software and operating system development.
