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

CC = clang -target x86_64-unknown-elf
AS = nasm
LD = ld.lld
OBJCOPY = llvm-objcopy
QEMU = qemu-system-x86_64

CFLAGS ?= -Og -gdwarf
ASFLAGS ?= -g -Fdwarf
LDFLAGS ?=
QEMUFLAGS ?= -no-reboot -no-shutdown -debugcon stdio
QEMUMEMSIZE ?= 2G

CHARDFLAGS := \
	-I$(INCLUDEDIR) \
	-I$(EXTERNALDIR) \
	-I$(EXTERNALDIR)/limine-efi/inc \
	-I$(EXTERNALDIR)/limine-efi/inc/x86_64 \
	-I$(EXTERNALDIR)/openlibm/include \
	-I$(EXTERNALDIR)/gdtoa/include \
	-I$(EXTERNALDIR)/openlibm/src \
	-DNO_FENV_H \
	-nostdlib -std=gnu2x \
	-fshort-wchar -ffreestanding \
	-fno-stack-protector -fno-stack-check \
	-fno-strict-aliasing -fno-lto -fPIE \
	-mcmodel=kernel -MMD -MP \
	-mno-red-zone -msoft-float
ASHARDFLAGS := -felf64 -MD -MP
LDHARDFLAGS := -T$(EXTERNALDIR)/limine-efi/gnuefi/elf_x86_64_efi.lds -nostdlib -z max-page-size=0x1000 -static -pie --no-dynamic-linker

LUABOOT := $(BUILDDIR)/luaboot.efi
LUABOOT_ELF := $(BUILDDIR)/luaboot.elf

CFILES := $(shell find $(SRCDIR) -name *.c)
ASMFILES := $(shell find $(SRCDIR) -name *.s)
OBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(CFILES:.c=.c.o))
ASMOBJ := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(ASMFILES:.s=.s.o))
DEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(CFILES:.c=.c.d))
ASMDEPS := $(patsubst $(SRCDIR)/%, $(BUILDDIR)/%, $(ASMFILES:.s=.s.d))

LUA_OBJS = $(BUILDDIR)/lua/lapi.c.o $(BUILDDIR)/lua/lcode.c.o $(BUILDDIR)/lua/lctype.c.o $(BUILDDIR)/lua/ldebug.c.o $(BUILDDIR)/lua/ldo.c.o $(BUILDDIR)/lua/ldump.c.o $(BUILDDIR)/lua/lfunc.c.o $(BUILDDIR)/lua/lgc.c.o $(BUILDDIR)/lua/linit.c.o $(BUILDDIR)/lua/llex.c.o \
	$(BUILDDIR)/lua/lmem.c.o $(BUILDDIR)/lua/lobject.c.o $(BUILDDIR)/lua/lopcodes.c.o $(BUILDDIR)/lua/lparser.c.o $(BUILDDIR)/lua/lstate.c.o $(BUILDDIR)/lua/lstring.c.o $(BUILDDIR)/lua/ltable.c.o \
	$(BUILDDIR)/lua/ltm.c.o $(BUILDDIR)/lua/lundump.c.o $(BUILDDIR)/lua/lvm.c.o $(BUILDDIR)/lua/lzio.c.o $(BUILDDIR)/lua/ltests.c.o $(BUILDDIR)/lua/lauxlib.c.o $(BUILDDIR)/lua/lbaselib.c.o $(BUILDDIR)/lua/loadlib.c.o $(BUILDDIR)/lua/lcorolib.c.o $(BUILDDIR)/lua/ltablib.c.o \
	$(BUILDDIR)/lua/liolib.c.o $(BUILDDIR)/lua/loslib.c.o $(BUILDDIR)/lua/lstrlib.c.o $(BUILDDIR)/lua/lmathlib.c.o $(BUILDDIR)/lua/lutf8lib.c.o $(BUILDDIR)/lua/ldblib.c.o

LIBM_CFILES := $(shell find $(EXTERNALDIR)/musl-libm -name *.c)
LIBM_OBJ := $(patsubst $(EXTERNALDIR)/musl-libm/%, $(BUILDDIR)/musl-libm/%, $(LIBM_CFILES:.c=.c.o))
LIBM_DEPS := $(patsubst $(EXTERNALDIR)/musl-libm/%, $(BUILDDIR)/musl-libm/%, $(LIBM_CFILES:.c=.c.d))

GDTOA_OBJS = $(BUILDDIR)/gdtoa/dmisc.c.o $(BUILDDIR)/gdtoa/dtoa.c.o $(BUILDDIR)/gdtoa/g__fmt.c.o $(BUILDDIR)/gdtoa/g_ddfmt.c.o $(BUILDDIR)/gdtoa/g_dfmt.c.o $(BUILDDIR)/gdtoa/g_ffmt.c.o $(BUILDDIR)/gdtoa/g_Qfmt.c.o $(BUILDDIR)/gdtoa/g_xfmt.c.o $(BUILDDIR)/gdtoa/g_xLfmt.c.o $(BUILDDIR)/gdtoa/gdtoa.c.o $(BUILDDIR)/gdtoa/gethex.c.o $(BUILDDIR)/gdtoa/gmisc.c.o $(BUILDDIR)/gdtoa/hd_init.c.o $(BUILDDIR)/gdtoa/hexnan.c.o $(BUILDDIR)/gdtoa/misc.c.o $(BUILDDIR)/gdtoa/smisc.c.o $(BUILDDIR)/gdtoa/strtod.c.o $(BUILDDIR)/gdtoa/strtodg.c.o $(BUILDDIR)/gdtoa/strtodI.c.o $(BUILDDIR)/gdtoa/strtof.c.o $(BUILDDIR)/gdtoa/strtoId.c.o $(BUILDDIR)/gdtoa/strtoIdd.c.o $(BUILDDIR)/gdtoa/strtoIf.c.o $(BUILDDIR)/gdtoa/strtoIg.c.o $(BUILDDIR)/gdtoa/strtoIQ.c.o $(BUILDDIR)/gdtoa/strtoIx.c.o $(BUILDDIR)/gdtoa/strtoIxL.c.o $(BUILDDIR)/gdtoa/strtopd.c.o $(BUILDDIR)/gdtoa/strtopdd.c.o $(BUILDDIR)/gdtoa/strtopf.c.o $(BUILDDIR)/gdtoa/strtopQ.c.o $(BUILDDIR)/gdtoa/strtopx.c.o $(BUILDDIR)/gdtoa/strtopxL.c.o $(BUILDDIR)/gdtoa/strtord.c.o $(BUILDDIR)/gdtoa/strtordd.c.o $(BUILDDIR)/gdtoa/strtorf.c.o $(BUILDDIR)/gdtoa/strtorQ.c.o $(BUILDDIR)/gdtoa/strtorx.c.o $(BUILDDIR)/gdtoa/strtorxL.c.o $(BUILDDIR)/gdtoa/sum.c.o $(BUILDDIR)/gdtoa/ulp.c.o

all: luaboot
luaboot: $(LUABOOT)

$(LUABOOT): $(LUABOOT_ELF)
	@$(MKCWD)
	@echo -e "[OBJCOPY]\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(OBJCOPY) -O binary $< $@

$(LUABOOT_ELF): $(BUILDDIR)/limine-efi/crt0-efi-x86_64.S.o $(BUILDDIR)/limine-efi/reloc_x86_64.c.o $(BUILDDIR)/arith64/arith64.c.o $(LIBM_OBJ) $(GDTOA_OBJS) $(BUILDDIR)/flanterm/flanterm.c.o $(BUILDDIR)/flanterm/backends/fb.c.o $(LUA_OBJS) $(OBJ) $(ASMOBJ)
	@$(MKCWD)
	@echo -e "[LD]\t\t$(@:$(BUILDDIR)/%=%)"
	$(Q)$(LD) $(BUILDDIR)/limine-efi/crt0-efi-x86_64.S.o $(BUILDDIR)/limine-efi/reloc_x86_64.c.o $(BUILDDIR)/arith64/arith64.c.o $(LIBM_OBJ) $(GDTOA_OBJS) $(BUILDDIR)/flanterm/flanterm.c.o $(BUILDDIR)/flanterm/backends/fb.c.o $(LUA_OBJS) $(OBJ) $(ASMOBJ) $(LDFLAGS) $(LDHARDFLAGS) -o $@

-include $(DEPS) $(ASMDEPS) $(LIBM_DEPS)

$(BUILDDIR)/%.c.o: $(SRCDIR)/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/%.s.o: $(SRCDIR)/%.s
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(SRCDIR)/%=%)"
	$(Q)$(AS) $(ASHARDFLAGS) -I$(SRCDIR) $< -o $@

$(BUILDDIR)/limine-efi/%.c.o: $(EXTERNALDIR)/limine-efi/gnuefi/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/limine-efi/%.S.o: $(EXTERNALDIR)/limine-efi/gnuefi/%.S
	@$(MKCWD)
	@echo -e "[AS]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/arith64/%.c.o: $(EXTERNALDIR)/arith64/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/musl-libm/%.c.o: $(EXTERNALDIR)/musl-libm/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/gdtoa/%.c.o: $(EXTERNALDIR)/gdtoa/src/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/flanterm/%.c.o: $(EXTERNALDIR)/flanterm/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

$(BUILDDIR)/lua/%.c.o: $(EXTERNALDIR)/lua/%.c
	@$(MKCWD)
	@echo -e "[CC]\t\t$(<:$(EXTERNALDIR)/%=%)"
	$(Q)$(CC) $(CFLAGS) $(CHARDFLAGS) -c $< -o $@

run: all
	@echo -e "[QEMU]\t\t$(LUABOOT:$(BUILDDIR)/%=%)"
	$(Q)mkdir -p build/efi-root/EFI/BOOT
	$(Q)cp $(LUABOOT) build/efi-root/EFI/BOOT/BOOTX64.EFI
	$(Q)$(QEMU) -m $(QEMUMEMSIZE) $(QEMUFLAGS) -hda fat:rw:build/efi-root -bios ../kora/headstart/zig-cache/edk2-x86_64.fd

clean:
	$(Q)$(RM)r $(BUILDDIR)
