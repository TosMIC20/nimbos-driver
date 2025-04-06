#include <assert.h>
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "nimbos.h"
#include "scf.h"

struct sync_map_args {
    uint64_t vaddr;
    uint64_t len;
    uint64_t paddr;
    int flags;
};

struct read_write_args {
    int fd;
    uint64_t buf_offset;
    uint64_t len;
};

struct read_write_args_new {
    int fd;
    uint64_t buf;
    uint64_t len;
};

static void *read_thread_fn(void *arg)
{
    struct read_write_args_new *args;
    struct syscall_queue_buffer *scf_buf = get_syscall_queue_buffer();
    uint16_t desc_index = (uint16_t)(long)arg;
    struct scf_descriptor *desc = get_syscall_request_from_index(scf_buf, desc_index);

    if (!desc) {
        return NULL;
    }

    args = offset_to_ptr(desc->args);
    char *buf = (char *)args->buf;
    int ret = read(args->fd, buf, args->len);
    // assert(ret == args->len);
    push_syscall_response(scf_buf, desc_index, ret);
    return NULL;
}

void poll_requests(void)
{
    uint16_t desc_index;
    struct scf_descriptor desc;
    struct syscall_queue_buffer *scf_buf = get_syscall_queue_buffer();
    int nimbos_fd = *get_nimbos_fd();
    pthread_t thread; // FIXME: use global threads pool

    while (!pop_syscall_request(scf_buf, &desc_index, &desc)) {
        // printf("syscall: desc_index=%d, opcode=%d, args=0x%lx\n", desc_index,
        // desc.opcode, desc.args);
        switch (desc.opcode) {
        case IPC_OP_READ: {
            pthread_create(&thread, NULL, read_thread_fn, (void *)(long)desc_index);
            break;
        }
        case IPC_OP_WRITE: {
            struct read_write_args_new *args = offset_to_ptr(desc.args);
            char *buf = (char *)args->buf;
            int ret = write(args->fd, buf, args->len);
            assert(ret == args->len);
            push_syscall_response(scf_buf, desc_index, ret);
            break;
        }
        case IPC_OP_SYNCMAP: {
            struct sync_map_args *args = offset_to_ptr(desc.args);
            void *vaddr = (void *)args->vaddr;
            // printf("Shadow: mmap vaddr=%p, len=%lu, paddr=%lx, flags=%x, fd=%d\n", vaddr, args->len, args->paddr, args->flags, nimbos_fd);
            void* mapped_ptr = mmap(vaddr, args->len, args->flags, MAP_SHARED | MAP_FIXED, nimbos_fd, args->paddr - NIMBOS_BASE_PADDR);
            int ret = 0;
            if (mapped_ptr == MAP_FAILED) {
                ret = -1;
            }
            push_syscall_response(scf_buf, desc_index, ret);
            // printf("Shadow: mmap ret=%d, mapped_ptr=%p\n", ret, mapped_ptr);
            break;
        }
        default:
            break;
        }
    }
}

static void nimbos_syscall_handler(int signum)
{
    if (signum == NIMBOS_SYSCALL_SIG_NUM) {
        poll_requests();
    }
}

int nimbos_setup_syscall()
{
    int fd = open(NIMBOS_DEV, O_RDWR);
    if (fd <= 0) {
        return fd;
    }
    int err = nimbos_setup_syscall_buffers(fd);
    if (err) {
        fprintf(stderr, "Failed to setup syscall buffers: %d\n", err);
        return err;
    }

    ioctl(fd, NIMBOS_SETUP_SYSCALL);
    signal(NIMBOS_SYSCALL_SIG_NUM, nimbos_syscall_handler);

    // handle requests before app starting
    poll_requests();

    return fd;
}
