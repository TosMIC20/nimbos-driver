#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#include <nimbos.h>
#include <remap.h>

int main()
{
    printf("Hello NimbOS!\n");

    // Check For reserved low address and high address
    if (!mmap((void*)0x0, NIMBOS_SIZE, PROT_NONE, MAP_SHARED, 0, 0)) {
        printf("Lower address [0x0, %lx] not reserved.\n", (size_t)NIMBOS_SIZE);
        return -1;
    }

    // Nimbos kernel virtual address
    void* nimbos_kernel_virt_addr = (void *)NIMBOS_KERNEL_PADDR_TO_VADDR(NIMBOS_KERNEL_BASE_PADDR);
    if (!mmap((void*)nimbos_kernel_virt_addr, NIMBOS_KERNEL_MAXSIZE, PROT_NONE, MAP_SHARED, 0, 0)) {
        printf("Higher address [%lx, %lx] not reserved.\n", (size_t)nimbos_kernel_virt_addr, (size_t)nimbos_kernel_virt_addr + NIMBOS_KERNEL_MAXSIZE);
        return -1;
    }

    // Nimbos user stack 
    if (!mmap((void*)NIMBOS_USER_STACK_BASE_VADDR, NIMBOS_USER_STACK_SIZE, PROT_NONE, MAP_SHARED, 0, 0)) {
        printf("User stack address [%lx, %lx] not reserved.\n", (size_t)NIMBOS_USER_STACK_BASE_VADDR, (size_t)NIMBOS_USER_STACK_BASE_VADDR + NIMBOS_USER_STACK_SIZE);
        return -1;
    }


    int fd = nimbos_setup_syscall();
    if (fd <= 0) {
        printf("Failed to open NimbOS device `%s`\n", NIMBOS_DEV);
        return fd;
    }

    for (;;) {
        // printf("Sleep %d...\n", i);
        usleep(1000);
    }

    close(fd);
    return 0;
}
