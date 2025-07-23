	# -------- CONFIG --------
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Kernel module
obj-m := swpctl_module.o

# Userspace test
TEST_SRC := test.c test_util.c
TEST_BIN := test_runner
CFLAGS := -Wall -Wextra -g

# -------- TARGETS --------
.PHONY: all clean

all: $(TEST_BIN) kmod
port: $(TEST_BIN) install_module
# Build the kernel module
kmod:
	$(MAKE) -C $(KDIR) M=$(PWD) modules
	sudo rmmod swpctl_module || true
	@sudo insmod swpctl_module.ko || { echo "Failed to insert module"; exit 1; }
	
install_module:
	sudo rmmod swpctl_module || true
	@sudo insmod swpctl_module.ko || { echo "Failed to insert module"; exit 1; }

# Build the userspace test binary
$(TEST_BIN): $(TEST_SRC) test_framework.h test_util.h
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC)

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) -f $(TEST_BIN)
	sudo rmmod swpctl_module || true
