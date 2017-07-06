Weird RTOS
==========
This is an educational RTOS that can be used with medium ended micro-controllers including 8-bit AVRs and Cortex-M3/4.

## Features
- Highly scalable for number of small to medium scale applications
- Priority based aperiodic scheduler
- Dynamic task creation and deletion
- *Condition* to create application specific signalling mechanisms like timers, events etc.
- *Semaphore* with optional specific interrupt protection
- *File system* based pipes to support data queues
- Support for suspending a task on multiple conditions to eliminate multiple tasks.
- A small networking stack with IPv4, ARP, UDP, TCP.
- Support for idle work to offload low priority work from main tasks.

## Supported MCUs and Platforms
| MCU/Platform | Core | RTOS Features |
| ------------ | ---- | -------- |
| atmega644 | AVR 8-bit | USART, ADC, SPI, Ethernet, MMC, File system, Alpha Numeric LCD |
| atmega1284 | AVR 8-bit | USART, ADC, SPI, Ethernet, MMC, File system, Alpha Numeric LCD |
| STM32F407Discovery | Cortex | USART, SPI, Ethernet |

## Minimum Requirements
| Controller | ROM | ROM actual\* | RAM | RAM Actual\*\* |
| ---------- | --- | ---------- | --- | ---------- |
| AVR 8-bit | 4 KB | 2962 bytes | 1 KB | 239 bytes |
| Cortex-M4 | 4 KB | 3920 bytes | 2 KB | 880 bytes |

_* with size optimizations_  
_** hello.c example with serial disabled, not including system stack_

## Building
Weird RTOS uses eclipse as build system and provides sample projects for each supported platform. Please checkout branch of the required platform and import it in eclipse. For more help please see the following pages
- [AVR Eclipse Setup](docs/build/AVR-ECLIPSE.md)

## Source Organization

```
+-- 3rdparty - 3rd party components
+-- api - Higher level components including TFTP, weird-view IoT framework
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
|           +-- eclipse_config.txt - Eclipse configurations
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
Weird RTOS is protected by three-clause MIT license, for more details see [License Agreement](LICENSE.md).