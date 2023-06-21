.SUFFIXES:
.DELETE_ON_ERROR:
.DEFAULT_GOAL := all

ifneq ($(VERBOSE), 1)
	Q := @
endif

SHELL = bash
MKDIR = mkdir -p
MKCWD = $(MKDIR) $(@D)

SRCDIR = $(CURDIR)/src
INCLUDEDIR = $(CURDIR)/include
BUILDDIR = $(CURDIR)/build
EXTERNALDIR = $(CURDIR)/external

CC = clang -target x86_64-efi-windows-msvc
AS = nasm
LD = lld-link
OBJCOPY = llvm-objcopy
QEMU = qemu-system-x86_64

CFLAGS ?= -Og -gcodeview
ASFLAGS ?= -g -Fcv8
LDFLAGS ?= /debug
QEMUFLAGS ?= -no-reboot -no-shutdown
QEMUMEMSIZE ?= 2G

CHARDFLAGS := \
	-I$(INCLUDEDIR) \
	-I$(EXTERNALDIR)/tinyefi/include \
	-nostdlib -std=gnu2x \
	-fshort-wchar -ffreestanding \
	-mcmodel=kernel -MMD -MP \
	-mno-80387 -mno-mmx -mno-3dnow \
	-mno-sse -mno-sse2 -msoft-float
ASHARDFLAGS := -fwin64 -MD -MP
LDHARDFLAGS := /subsystem:efi_application /nodefaultlib /entry:main

LUABOOT := $(BUILDDIR)/luaboot.efi

CFILES := $(shell find $(SRCDIR) -name *.c)
ASMFILES := $(shell find $(SRCDIR) -name *.s)
OBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(CFILES:.c=.c.obj))
ASMOBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(ASMFILES:.s=.s.obj))
DEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(CFILES:.c=.c.d))
ASMDEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(ASMFILES:.s=.s.d))

all: luaboot
luaboot: $(LUABOOT)

$(LUABOOT): $(OBJ) $(ASMOBJ) $(BUILDDIR)/tinyefi/tinyefi.c.obj $(BUILDDIR)/tinyefi/console.c.obj
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(OBJ) $(ASMOBJ) $(BUILDDIR)/tinyefi/tinyefi.c.obj $(BUILDDIR)/tinyefi/console.c.obj $(LDFLAGS) $(LDHARDFLAGS) /out:$@

-include $(DEPS) $(ASMDEPS)

$(BUILDDIR)/%.c.obj: $(SRCDIR)/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/%.s.obj: $(SRCDIR)/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR) $< -o $@

$(BUILDDIR)/tinyefi/%.c.obj: $(EXTERNALDIR)/tinyefi/src/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

run: all
	@echo -e "[QEMU]\t\t$(LUABOOT:$(BUILDDIR)/%=%)"
	$(Q)$(QEMU) -m $(QEMUMEMSIZE) $(QEMUFLAGS) -hda $(LUABOOT) -debugcon stdio

clean:
	$(Q)$(RM)r $(BUILDDIR)
