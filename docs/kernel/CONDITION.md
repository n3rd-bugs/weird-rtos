Condition
=========
## Introduction
Weird RTOS provides a generalized suspension mechanisum through *condition*. This not only generalize the suspensions in the system but also allows a task to suspend on multiple conditions to remove requirement of spawning separate handler tasks.

### Sources
- [condition.c](../../rtos/kernel/condition.c)

### Headers
- [condition.h](../../rtos/kernel/condition.h)

## Basic Concepts
A major feature that an operating system provides is to allow task to wait for a certain condition and resume it when that condition is achived, this can be a semepahore, an event or even data comming from a socket. Weird RTOS provide the suspension services through *condition*.

### Basic features
- Common suspension flow for all the components.
- Allows suspension for more than one conditions.
- Internally handles the protection of condition data through condition specific semaphore.

### How it works
Weird RTOS breaks the suspension criteria in two parts

- *condition*
As it's name suggest it defines the condition for the suspension, e.g. condition for sempahore availability, condition for data availability on a socket etc.

- *suspend*
This defines the suspension criteria. This mainly holds the task suspended on a condition and if we need to wait indefinatly or not.