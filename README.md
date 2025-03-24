# NimbOS driver

A Linux kernel module to control [NimbOS](https://github.com/rvm-rtos/nimbos) from the guest Linux hosted by [RVM1.5](https://github.com/rvm-rtos/RVM1.5).

## How-to

* build [nimbos](https://github.com/rvm-rtos/nimbos)

```bash
make env # First build
make build RVM=on
```

* scp nimbos binary into guest and create link file in `/lib/firmware`
```bash
scp -P 2333 target/x86_64/release/nimbos.bin ubuntu@localhost:~/
sudo ln -sf ~/nimbos.bin /lib/firmware
```

* copy `nimbos-driver` directory to guest and compile

```bash
# On host
scp -P 2333 -r ../nimbos-driver ubuntu@localhost:~/
```

* compile and run
    * Note: 
        * nimbos-driver kernel module has to be install **after jailhouse kernel module**
        * nimbos has to be run **after RVM is enabled**

```bash
# On guest
make build
make run
```

* output

```bash
ubuntu@ubuntu:~$ cd nimbos-driver/
ubuntu@ubuntu:~/nimbos-driver$ make run
sudo insmod driver/nimbos_driver.ko
[ 321.125619 INFO  1] Starting RTOS: entry=0x42000000
[ 321.125913 INFO  1] Starting RT cpu 3...

NN   NN  iii               bb        OOOOO    SSSSS
NNN  NN       mm mm mmmm   bb       OO   OO  SS
NN N NN  iii  mmm  mm  mm  bbbbbb   OO   OO   SSSSS
NN  NNN  iii  mmm  mm  mm  bb   bb  OO   OO       SS
NN   NN  iii  mmm  mm  mm  bbbbbb    OOOO0    SSSSS
              ___    ____    ___    ___
             |__ \  / __ \  |__ \  |__ \
             __/ / / / / /  __/ /  __/ /
            / __/ / /_/ /  / __/  / __/
           /____/ \____/  /____/ /____/

arch = x86_64
platform = pc-rvm
build_mode = release
log_level = warn

Initializing kernel heap at: [0xffffff8042091978, 0xffffff8042491978)
Initializing IDT...
Loading IDT and GDT for CPU 0...
Initializing frame allocator at: [PA:0x42492000, PA:0x49eff000)
Mapping .text: [0xffffff8042000000, 0xffffff8042019000)
Mapping .rodata: [0xffffff8042019000, 0xffffff804201e000)
Mapping .data: [0xffffff804201e000, 0xffffff804208d000)
Mapping .bss: [0xffffff8042091000, 0xffffff8042492000)
Mapping boot stack: [0xffffff804208d000, 0xffffff8042091000)
Mapping physical memory: [0xffffff8042492000, 0xffffff8049eff000)
Mapping syscall data buffer: [0xffffff8049eff000, 0xffffff8049fff000)
Mapping syscall queue buffer: [0xffffff8049fff000, 0xffffff804a000000)
Mapping MMIO: [0xffffff80fec00000, 0xffffff80fec01000)
Mapping MMIO: [0xffffff80fee00000, 0xffffff80fee01000)
Initializing drivers...
Initializing task manager...
/**** APPS ****
cyclictest
exit
fantastic_text
forktest
forktest2
forktest_simple
forktest_simple_c
forktree
hello_c
hello_world
matrix
sleep
sleep_simple
stack_overflow
thread_simple
user_shell
usertests
yield
**************/
Running tasks...
test kernel task: pid = TaskId(2), arg = 0xdead
test kernel task: pid = TaskId(3), arg = 0xbeef
sudo chmod 666 /dev/nimbos
./apps/hello
Hello NimbOS!
Rust user shell
>>
```