/********************************************************************************************************************/
/*
 * Runtime Prolog
 */
/********************************************************************************************************************/

.section .init
.global _init
.type _init, @function
_init:
    push %ebp
    movl %esp, %ebp
    /* Call our generic __runtime_init function */
    call __runtime_init

.section .fini
.global _fini
.type _fini, @function
_fini:
    push %ebp
    movl %esp, %ebp
    /* Call our generic __runtime_fini function */
    call __runtime_fini
