#include <string.h>
#include <os.h>
#include <tasks.h>

static void task_stack_init(TASK *tcb, TASK_ENTRY *entry, void *argv)
{
    /* The start of the task code will be popped off the stack last, so place
     * it on first. */
    *(tcb->tos) = ((uint16_t)entry & 0x00FF);
    (tcb->tos)--;

    *(tcb->tos) = ((uint16_t)entry & 0xFF00) >> 8;
    (tcb->tos)--;

    (tcb->tos)--;                                   /* Push R0 on the stack. */

    *(tcb->tos) = 0x80;
    (tcb->tos)--;

    *(tcb->tos) = 0x00;                             /* The compiler expects R1 to be 0. */
    (tcb->tos) -= 0x17;                             /* Push R1-R23 on the stack. */

    *(tcb->tos) = ((uint16_t)argv & 0x00FF);
    (tcb->tos)--;                                   /* Push R24 on the stack. */

    *(tcb->tos) = ((uint16_t)argv & 0xFF00) >> 8;
    (tcb->tos)--;                                   /* Push R25 on the stack. */

    (tcb->tos) -= 0x06;                             /* Push R26-R31 on the stack. */
}

void task_create(TASK *tcb, char *name, char *stack, uint32_t stack_size, TASK_ENTRY *entry, void *argv)
{
    /* Clear the task structure. */
    memset(tcb, 0, sizeof(TASK));

#ifdef CONFIG_INCLUDE_TASK_STATS
    /* Copy the task name. */
    strncpy(tcb->name, name, 7);

    /* Store the stack information for this task. */
    tcb->stack_size = stack_size;
    tcb->stack_start = stack;

    /* Fill the task stack with a pre-defined constant. */
    memset(stack, CONFIG_STACK_FILL, stack_size);
#else
    /* Remove some warnings. */
    UNUSED_PARAM(name);
#endif /* CONFIG_INCLUDE_TASK_STATS */

    /* Set task's stack pointer. */
    tcb->tos = stack + (stack_size - 1);

    /* Initialize task's stack. */
    task_stack_init(tcb, entry, argv);

}
