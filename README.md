Weird RTOS
==========
This is an educational RTOS that can be used with medium ended micro-controllers including 8-bit AVRs and Cortex-M3/4.

## Features
- Highly scalable for number of small to medium scale applications
- Priority based aperiodic scheduler
- Dynamic task creation and deletion
- *Condition* to create application specific signaling mechanisms like timers, events etc.
- *Semaphore* with optional specific interrupt protection
- *File system* based pipes to support data queues
- Support for suspending a single task on multiple conditions to eliminate multiple tasks
- A small networking stack with IPv4, ARP, UDP, TCP
- Support for idle work to offload low priority work from main tasks

## Supported MCUs and Platforms
| MCU/Platform | Core | RTOS Features |
| ------------ | ---- | -------- |
| atmega644 | AVR 8-bit | USART, ADC, SPI, Ethernet, MMC, File system, Alphanumeric LCD |
| atmega1284 | AVR 8-bit | USART, ADC, SPI, Ethernet, MMC, File system, Alphanumeric LCD |
| STM32F407vgt6 | Cortex-M4 | USART, SPI, Ethernet |
| STM32F103-stamp | Cortex-M3 | USART, ADC, SPI, Ethernet, MMC, File system, Alpha Numeric LCD |

## Minimum Requirements
| Controller | ROM | ROM actual\* | RAM | RAM Actual\*\* |
| ---------- | --- | ---------- | --- | ---------- |
| AVR 8-bit | 4 KB | 3086 bytes | 1 KB | 345 bytes |
| Cortex-M3 | 4 KB | 2075 bytes | 2 KB | 784 bytes |
| Cortex-M4 | 4 KB | 2139 bytes | 2 KB | 960 bytes |

_* with size optimizations_
_** hello.c example with serial disabled, not including system stack_

## Building
### Building as Library
To build the RTOS as library follow these steps:
1. Create a build folder
    `mkdir lib-build`
    `cd lib-build`
2. Configure the library project for a platform
    `cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=<path-to-root>/cmake/toolchains/<cross-tool>.cmake <path-to-root>`
3. Build the library
    `make`

The created library can be used to build sample applications using other tools.

### Building a Sample
To build an example project please follow these steps:
1. Create a build folder
    `mkdir build`
    `cd build`
2. Configure the example project for a platform
    `cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=<path-to-root>/cmake/toolchains/<cross-tool>.cmake <path-to-root>/examples/<example>`
3. Build the sample
    `make`

## Configurations
Weird-RTOS provides various configuration options that can be best viewed if project is configured through cmake-gui. These allow user to adjust various RTOS features according to his requirements.

## Source Organization

```
+-- 3rdparty - 3rd party components
+-- api - Higher level components including TFTP, weird-view IoT framework
+-- cmake - Cmake project files
|   +-- modules - Implements various helper APIs for cmake
|   +-- toolchains - Provides toolchain files
+-- docs - Related documentation
+-- examples - Some usage examples
+-- port - Target specific bits
|   +-- kernel - Target specific kernel components
|   +-- target - Platform specific device drivers
|       +-- <platform> - A platform entry
|           +-- board - Board specific configurations
|           +-- io - Device drivers
|           +-- schematics - Platform schematics if applicable
|           +-- utils - Broad utilities
|   +-- toolset - Toolset specific definitions
+-- rtos - Operating system generic components
|   +-- fs - File system
|   +-- io - IO device drivers and their middleware
|   +-- mem - Memory management
|   +-- net - Networking stack
|   +-- kernel - Kernel APIs
|   +-- utils - System utilities
```

## Documentation
### Kernel
- [Scheduler](docs/kernel/SCHEDULER.md)
- [Semaphore](docs/kernel/SEMAPHORE.md)
- [Sleep](docs/kernel/SLEEP.md)
- [Idle](docs/kernel/IDLE.md)
- [System Logging](docs/kernel/LOGGING.md)

## Licensing
This is a _non-commercial_ RTOS, for more details see [License Agreement](LICENSE.md).