Getting Started Guide
=====================

## Source Organization
```
+-- 3rdparty - 3rd party components
+-- api - Higher level components including TFTP, weird-view IoT framework
+-- docs - Documentation
+-- examples - Some usage examples
+-- port - Target specific bits
|   +-- os - Target specific operating system components
|   +-- target - Platform specific device drivers
|       +-- <platform> - A platform entry
|           +-- board - Board specific configurations
|           +-- io - Device drivers
|           +-- schematics - Platform schematics if applicable
|           +-- utils - Broad utilities
|           +-- eclipse_config.txt - Eclipse configurations
|   +-- toolset - Toolset specific definitions
+-- rtos - Operating system generic components
|   +-- fs - File system
|   +-- io - IO device drivers and their middleware
|   +-- mem - Memory management
|   +-- net - Networking stack
|   +-- os - Operating system
|   +-- utils - System utilities
```

## Features
- Basic operating system features including
  - *Semaphores* with optional specific interrupt protection
  - *Conditionion* to create timers, events etc.
  - Dynamic task creation and deletion.
  - Priority based aperiodic schedular
- Mutiple condition suspend support to eliminate threads from system.
- A small networking stack with IPv4, ARP, UDP, TCP.
- Idle task work scheduling for polling.
- Centralized component based logging mechanisum.
- Minimilistc global interrupt locking.

## Usage Guides
Weird RTOS lacks a good build system and is not yet planed, however for time 
being smaple eclipse projects for each supported platforms are available.
Please checkout branch for any of the required platform and import the eclipse 
project. For more help please see following pages:
- [AVR Eclipse Setup](docs/AVR-ECLIPSE.md)

## API Documentation
TODO