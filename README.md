# Buffer Cache Simulation
A simple buffer cache implementation for simulation of getblk and brelse algorithms.
## Overview
In this implementation, the main program act as kernel and manipulates the buffer cache, while processes are being created by using the fork () command. It controls which block to get and release. User can manipulate buffer status. This gives us complete control over the simulation. Also, we can view the buffer cache, free list, buffer list, and sleeping processes at all times. This is a multi-process synchronous program using fork (), so there is no added complexity of managing threads. We can focus on getblk and buffer cache.
## Problem Statement
The idea was to simulate getblk and brelse algorithms that handle the allocation and release of buffers to processes. The key requirement was to be able to visualize all 5 scenarios of getblk and clearly identify the working of getblk in all of those scenarios.
## Suggested Solution
We want a simulation slow enough to analyse the working details of getblk.
## Programming Language Used
C++ is the chosen language. Because we are implementing buffer cache, which is a lower level algorithm, it needs to be fast though C provides the fastest system calls but C++ gives us object-oriented approach.
## AOS Concepts Implemented
1. Multiprogramming concept using forks
2. Locking unlocking mechanism
