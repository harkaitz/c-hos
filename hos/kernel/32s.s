// CONSTANTS.
.set MAGIC, 0x1badb002
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)
.extern hos_32c_multibootEntry
.extern hos_32c_interruptionEntry
.global hos_32s_interruptionIgnorer

.macro hos_DefineExceptionHandler num
        .global hos_32s_exceptionHandler\num
        movb $\num, (interruptNumber)
        jmp int_bottom
.endm
.macro hos_DefineInterruptionHandler num
        .global hos_32s_interruptionHandler\num
        hos_32s_interruptionHandler\num:
        movb $\num, (interruptNumber)
        jmp int_bottom
.endm
.macro hos_DefineInterruptionIgnorer num
        .global hos_32s_interruptionHandler\num
        hos_32s_interruptionHandler\num:
        iret
.endm
        
    
// ------------------------------------------------------------------
// MULTIBOOT HEADER
// ------------------------------------------------------------------
        .section .multiboot
        .long MAGIC
        .long FLAGS
        .long CHECKSUM

// ------------------------------------------------------------------
// CODE SECTION
// ------------------------------------------------------------------
        .section .text
loader:
        .global loader
        mov $kernel_stack, %esp
        push %eax
        push %ebx
        call hos_32c_multibootEntry
_stop:
        hlt
        jmp _stop
int_bottom:
        // Push all registers.
        
        pusha
        pushl %ebp
        pushl %edi  
        pushl %esi 
        pushl %edx  
        pushl %ecx  
        pushl %ebx  
        pushl %eax
        
        // Execute interrupt handler.
        push %esp
        push (interruptNumber)
        cli
        call hos_32c_interruptionEntry
        sti
        mov %eax, %esp
        
        // Restore registers.
        popl %eax
        popl %ebx
        popl %ecx
        popl %edx
        popl %esi
        popl %edi
        popl %ebp
        popa
hos_32s_interruptionIgnorer:
        iret
hos_DefineInterruptionHandler 0x20
hos_DefineInterruptionHandler 0x21        

// -------------------------------------------------------------------
// DATA SECTION
// -------------------------------------------------------------------
        .section .data
interruptNumber:
        .byte 8
        .section .bss
        .space 2*1024*1024
kernel_stack:
        
