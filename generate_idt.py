with open("interrupt_handlers.h", "w") as f:
    f.write("#ifndef INCLUDE_INTERRUPT_HANDLERS_H\n")
    f.write("#define INCLUDE_INTERRUPT_HANDLERS_H\n\n")
    f.write("extern void (*interrupt_handlers[256])();\n")
    for i in range(256): 
        f.write(f"extern void interrupt_handler_{i}();\n")
    f.write("\n#endif\n")
    
with open("interrupt_handlers.c", "w") as f:
    f.write("#include \"interrupt_handlers.h\"\n\n")
    f.write("void (*interrupt_handlers[256])() = {\n")
    for i in range(256):
        f.write(f"    interrupt_handler_{i},\n")
    f.write("};\n")
    
with open("interrupt_handlers.s", "w") as f:
    f.write("section .text\n")
    error_code_interrupts = [8, 10, 11, 12, 13, 14, 17]
    for i in range(256):
        if i in error_code_interrupts:
            f.write(f"error_code_interrupt_handler {i}\n")
        else:
            f.write(f"no_error_code_interrupt_handler {i}\n")