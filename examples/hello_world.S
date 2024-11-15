.data
    message: .string "Hello World"

.text
    .global main
    .extern puts

main:
    # Set up stack frame
    pushq %rbp
    movq %rsp, %rbp

    # Call puts with message
    leaq message(%rip), %rdi    # Load effective address relative to RIP
    call puts

    # Return 0
    movl $0, %eax
    
    # Restore stack frame and return
    popq %rbp
    ret
