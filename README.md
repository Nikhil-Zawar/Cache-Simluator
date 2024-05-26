# Cache Simulator

## Overview 
The primary focus is the evaluation and simulation of various cache configurations using memory traces from actual benchmark programs.

## Features

### Cache Simulator 

- **Configurable Cache Parameters**: The simulator supports various cache configurations including:
  - Number of sets in the cache (power of 2).
  - Number of blocks per set (power of 2).
  - Number of bytes per block (power of 2, at least 4).
  - Write-allocate or no-write-allocate policy.
  - Write-through or write-back policy.
  - LRU (Least-Recently-Used) or FIFO (First-In-First-Out) eviction policy.

- **Memory Access Simulation**: Reads memory access traces from standard input and simulates cache behavior based on the provided parameters.
  - **Trace Format**: Each line in the trace file represents a memory access with three fields: load/store indicator, 32-bit memory address (in hexadecimal), and an ignored third field.
  - **Performance Metrics**: Outputs summary statistics such as hit rates, miss penalties, and other relevant metrics to standard output.

### Optimal Cache Configuration - Best Cache
- **Experimentation and Analysis**: Utilizes the simulator to determine the most effective cache configuration by running experiments with various parameters.
- **Criteria for Best Cache**: Considers hit rates, miss penalties, total cache size (including overhead) to identify the optimal configuration.
