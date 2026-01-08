Memory Management Simulator

A C-based, operating-system–inspired Memory Management Simulator designed to study and compare different dynamic memory allocation strategies.
The simulator provides an interactive command-line interface (CLI) that allows users to allocate, free, and inspect memory layouts while observing allocator behavior and statistics.

This project focuses on correctness, clarity, and modular design, making it suitable for academic demonstrations and learning purposes.

🎯 Objectives

Simulate common dynamic memory allocation strategies used in operating systems
Visualize memory layout changes after allocation and deallocation
Compare linear (FIT-based) allocators with hierarchical (Buddy) allocation
Maintain a clean separation between allocation logic and observability
Provide an extensible and interactive platform for experimentation

✨ Features

Multiple allocation strategies:

First Fit
Best Fit
Worst Fit
Buddy Allocator (power-of-two blocks)

Runtime switching between allocator strategies
Interactive CLI for memory operations
Unified memory dump interface
Allocation and deallocation statistics
Modular and well-organized codebase
Demonstration video included

🧠 System Design Overview

The simulator is organized into independent subsystems:

Allocator – Handles memory allocation and deallocation
Observability – Visualizes memory state using a single dump command
CLI – Parses user commands and drives execution
Statistics – Tracks allocation metrics
Cache (extension) – Implemented separately and decoupled from allocation
This separation ensures that changes in one component do not affect others.

📦 Allocation Models
FIT-Based Allocators (First / Best / Worst Fit)

Use out-of-band metadata (block_t)

Heap contains only user payload

Metadata tracks:

Block offset
Block size
Requested size
Allocation status
Support block splitting and coalescing
Linear traversal of memory

These allocators demonstrate trade-offs between speed and fragmentation.

Buddy Allocator
Memory is managed in power-of-two block sizes
Entire heap is treated as a power-of-two region

Allocation process:

Requested size is rounded up to the nearest power of two
Blocks are recursively split until the smallest suitable block is found
Remaining blocks are kept as free buddies

Each block contains a small header storing:

Allocation ID
Block order (size)
Requested size (for internal fragmentation tracking)

This allocator demonstrates predictable block sizes and fast allocation at the cost of internal fragmentation.

🔍 Memory Dump & Observability

A single dump command is exposed to the user

Internally adapts based on the active allocator:

FIT allocators use metadata arrays
Buddy allocator traverses the heap using block headers
Allocation logic never prints memory directly
Observability remains centralized and consistent

📊 Statistics Collection

The simulator records:

Allocation attempts
Successful allocations
Failed allocations
Number of frees
Internal and external fragmentation
Memory utilization percentage

Statistics are allocator-agnostic and help evaluate allocator behavior.

🎥 Demonstration Video

A demonstration video is included in the repository:
video_demo_24115135.mp4
The video demonstrates:

Initializing memory
Switching allocator strategies
Allocating and freeing memory
Visualizing memory layouts using dump
Viewing statistics
Graceful shutdown of the simulator

📁 Repository Structure
Memory-Management-Simulator/
├── allocator/               # Allocation strategies and dispatcher
│   ├── allocator.c
│   ├── allocator.h
│   ├── buddy.c
│   ├── buddy.h
│   ├── first_fit.c
│   ├── best_fit.c
│   └── worst_fit.c
├── cache/                   # Cache simulation (separate subsystem)
│   ├── cache.c
│   └── cache.h
├── observability/           # Memory dump logic
│   ├── memory_dump.c
│   └── memory_dump.h
├── simulator/               # CLI and command parsing
│   ├── cli.c
│   └── cli.h
├── stats/                   # Statistics collection
│   ├── stats.c
│   └── stats.h
├── test_artifacts/          # Logs and test outputs
├── main.c                   # Program entry point
├── Makefile
├── README.md
├── video_demo_24115135.mp4
└── report_24115135.pdf

▶️ Build & Run
Linux / GitHub Codespaces
gcc -std=c11 -Wall -Wextra -g -Icache \
main.c allocator/*.c cache/*.c observability/*.c stats/*.c simulator/*.c \
-o memsim

./memsim

Windows (MinGW)
gcc -std=c11 -Wall -Wextra -g -Icache `
main.c allocator/*.c cache/*.c observability/*.c stats/*.c simulator/*.c `
-o memsim.exe

.\memsim.exe


Compiled binaries are platform-specific and should not be committed to the repository.

💻 CLI Commands
Command	Description
init memory <bytes>	Initialize memory pool
set allocator <type>	Select allocator strategy
malloc <bytes>	Allocate memory
free <id>	Free allocated block
dump	Display memory layout
stats	Show allocation statistics
shutdown	Release memory
exit / quit	Exit simulator
🧪 Example: First Fit Allocator
init memory 2048
set allocator first
malloc 200
malloc 100
dump


This demonstrates linear allocation and block splitting.

🧪 Example: Buddy Allocator
init memory 1000
set allocator buddy
malloc 100
dump


Output:

USED (128 bytes)
FREE (128 bytes)
FREE (256 bytes)
FREE (512 bytes)


This confirms correct power-of-two rounding and block splitting.

⚠️ Assumptions & Limitations

Compiled binaries are not portable across platforms
Buddy allocator does not yet implement coalescing on free
No virtual memory or paging
Simulator is single-threaded
Intended for educational use, not production deployment

🚀 Future Enhancements

Advanced fragmentation metrics
Enhanced cache statistics
Automated test cases

Visualization tools for memory layout

👤 Author

Shivam Kumar
Memory Management Simulator
Academic Project
