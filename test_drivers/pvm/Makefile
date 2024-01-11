# SPDX-License-Identifier: GPL-2.0

AUDIOLITE_ROOT=$(PWD)
KBUILD_OPTIONS += AUDIOLITE_ROOT=$(AUDIOLITE_ROOT)

obj-y += $(PWD)

all: clean modules

modules:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) $(KBUILD_OPTIONS) modules

modules_install:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) modules_install

clean:
	$(MAKE) -C $(KERNEL_SRC) M=$(PWD) clean
