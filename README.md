# Aging Heap Allocator

## Overview
The Aging Heap Allocator is a simple C-based simulation that demonstrates how memory blocks can age over time and be automatically deallocated once they exceed a predefined threshold.

This project illustrates a conceptual memory management strategy where memory blocks track their age during execution cycles. When a block becomes too old, it is considered expired and is removed from memory.

## Features
- Simulate heap memory allocation
- Track memory block aging across cycles
- Automatic deallocation when threshold is exceeded
- Demonstrates basic memory management logic

## Technologies Used
- C Programming Language
- Standard C Libraries

## How It Works

1. A memory block is allocated with an ID and size.
2. Each execution cycle increases the block's age.
3. The system checks if the block age exceeds a predefined threshold.
4. If the threshold is exceeded, the block is automatically deallocated.

## Example Output
Block Allocated | ID: 1 | Size: 256
Cycle 1 | Block Age: 1
Cycle 2 | Block Age: 2
Cycle 3 | Block Age: 3
Cycle 4 | Block Age: 4
Cycle 5 | Block Age: 5
Cycle 6 | Block Age: 6
Block expired and deallocated.
## Learning Purpose
This project demonstrates:
- Memory allocation concepts
- Aging-based memory management
- Struct usage in C
- Basic system simulation

## Author
Jisan
