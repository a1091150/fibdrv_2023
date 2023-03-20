CONFIG_MODULE_SIG = n
TARGET_MODULE := fibdrv_new

obj-m := $(TARGET_MODULE).o
$(TARGET_MODULE)-objs := fibdrv.o bn.o
ccflags-y := -std=gnu99 -Wno-declaration-after-statement

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

GIT_HOOKS := .git/hooks/applied

all: $(GIT_HOOKS) client
	$(MAKE) -C $(KDIR) M=$(PWD) modules

$(GIT_HOOKS):
	@scripts/install-git-hooks
	@echo

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	$(RM) client out
	$(RM) *.out
load:
	sudo insmod $(TARGET_MODULE).ko
unload:
	sudo rmmod $(TARGET_MODULE) || true >/dev/null

client: client.c
	$(CC) -o $@ $^

PRINTF = env printf
PASS_COLOR = \e[32;01m
NO_COLOR = \e[0m
pass = $(PRINTF) "$(PASS_COLOR)$1 Passed [-]$(NO_COLOR)\n"

check: all
	$(MAKE) unload
	$(MAKE) load
	sudo ./client > out
	$(MAKE) unload
	@diff -u out scripts/expected.txt && $(call pass)
	@scripts/verify.py

ufib:
	$(CC) -g ufib.c bn.c -o ufib.out -D _USERSPACEFIB

uperf: ufib
	sudo perf record -g --call-graph dwarf ./ufib.out
	sudo perf report --stdio

ustat: ufib
	sudo perf stat -r 10 -e cycles,instructions,cache-misses,cache-references,branch-instructions,branch-misses ./ufib.out

uplot: ufib
	./ufib.out > ./ufib_time.ut
	gnuplot scripts/ufib_plot.gp

tclient: 
	$(CC) tclient.c -o tclient.out
	sudo ./tclient.out > ./kfib_time.ut
	gnuplot scripts/kfib_plot.gp