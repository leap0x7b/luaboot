#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <elf.h>
#include <errno.h>
#include <luaboot/luamod.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>

typedef struct {
    Elf64_Ehdr *header;
    Elf64_Phdr **program_headers;
    Elf64_Shdr **section_headers;
} elf_t;

static const struct luaL_Reg elf_meth[] = {
    { "load", NULL },
    { NULL, NULL }
};

#define push_ehdr_value(L, ehdr, name) \
    lua_pushinteger(L, header->e_##name); \
    lua_setfield(L, -2, "name");

#define push_elf_ident(L, ehdr, name, offset) \
    lua_pushinteger(L, header->e_ident[offset]); \
    lua_setfield(L, -2, name);

#define push_phdr_value(L, phdr, name) \
    lua_pushinteger(L, phdr->p_##name); \
    lua_setfield(L, -2, "name");

#define push_shdr_value(L, shdr, name) \
    lua_pushinteger(L, shdr->sh_##name); \
    lua_setfield(L, -2, "name");

const char *get_shdr_name(Elf64_Ehdr *ehdr, Elf64_Shdr *shdr) {
    Elf64_Shdr *sh_strtab = (Elf64_Shdr *)(ehdr + ehdr->e_shoff + ehdr->e_shstrndx);
    void *sh_strtab_p = ehdr + sh_strtab->sh_offset;

    return (const char *)(sh_strtab_p + shdr->sh_name);
}

int parse_elf(lua_State *L) {
    const char *path = luaL_checkstring(L, 1);
    FILE *file;
    size_t size;
    uint8_t *buf;

    if ((file = fopen(path, "r"))) {
        fseek(file, 0, SEEK_END);
        size = ftell(file);
        fseek(file, 0, SEEK_SET);
        buf = malloc(size + 1);
        if (!buf) {
            luaL_error(L, "Unable to allocate memory: %s", strerror(errno));
            return -1;
        }
        fread(buf, size, 1, file);
        fclose(file);
    } else {
        luaL_error(L, "Unable to open file: %s", strerror(errno));
        return -1;
    }

    elf_t **elf = lua_newuserdata(L, sizeof(elf_t));
    Elf64_Ehdr *header = (Elf64_Ehdr *)buf;
    (*elf)->header = header;
    (*elf)->program_headers = malloc(header->e_phentsize * header->e_phnum);
    (*elf)->section_headers = malloc(header->e_shentsize * header->e_shnum);

    int i = 0;
    if (!memcmp(header->e_ident, ELFMAG, SELFMAG) &&   /* magic match? */
        header->e_ident[EI_CLASS] == ELFCLASS64 &&     /* 64 bit? */
        header->e_ident[EI_DATA] == ELFDATA2LSB &&     /* LSB? */
        header->e_type == ET_EXEC &&                   /* executable object? */
        header->e_machine == EM_AMD64 &&               /* architecture match? */
        header->e_phnum > 0) {                         /* has program headers? */
        lua_newtable(L);

        lua_newtable(L);
        lua_newtable(L);
        lua_pushstring(L, strstr((const char *)header->e_ident, ELFMAG));
        lua_setfield(L, -2, "magic");
        push_elf_ident(L, header, "class", EI_CLASS);
        push_elf_ident(L, header, "data", EI_CLASS);
        push_elf_ident(L, header, "version", EI_VERSION);
        push_elf_ident(L, header, "abiversion", EI_ABIVERSION);
        lua_setfield(L, -2, "ident");
        push_ehdr_value(L, header, type);
        push_ehdr_value(L, header, machine);
        push_ehdr_value(L, header, version);
        push_ehdr_value(L, header, entry);
        push_ehdr_value(L, header, phoff);
        push_ehdr_value(L, header, shoff);
        push_ehdr_value(L, header, flags);
        push_ehdr_value(L, header, ehsize);
        push_ehdr_value(L, header, phentsize);
        push_ehdr_value(L, header, phnum);
        push_ehdr_value(L, header, shentsize);
        push_ehdr_value(L, header, shnum);
        push_ehdr_value(L, header, shstrndx);
        lua_setfield(L, -2, "header");

        lua_newtable(L);
        Elf64_Phdr *phdr;
        for (phdr = (Elf64_Phdr *)(buf + header->e_phoff), i = 0;
            i < header->e_phnum;
            i++, phdr = (Elf64_Phdr *)((uint8_t *)phdr + header->e_phentsize)) {
            lua_newtable(L);
            push_phdr_value(L, phdr, type);
            push_phdr_value(L, phdr, flags);
            push_phdr_value(L, phdr, offset);
            push_phdr_value(L, phdr, vaddr);
            push_phdr_value(L, phdr, paddr);
            push_phdr_value(L, phdr, filesz);
            push_phdr_value(L, phdr, memsz);
            push_phdr_value(L, phdr, align);
            lua_settable(L, i);
            (*elf)->program_headers[i] = phdr;
        }
        lua_setfield(L, -2, "program_headers");

        lua_newtable(L);
        Elf64_Shdr *shdr;
        for (shdr = (Elf64_Shdr *)(buf + header->e_shoff), i = 0;
            i < header->e_shnum;
            i++, shdr = (Elf64_Shdr *)((uint8_t *)shdr + header->e_shentsize)) {
            lua_newtable(L);
            lua_pushstring(L, get_shdr_name(header, shdr));
            lua_setfield(L, -2, "name");
            push_shdr_value(L, shdr, type);
            push_shdr_value(L, shdr, flags);
            push_shdr_value(L, shdr, addr);
            push_shdr_value(L, shdr, offset);
            push_shdr_value(L, shdr, size);
            push_shdr_value(L, shdr, link);
            push_shdr_value(L, shdr, info);
            push_shdr_value(L, shdr, addralign);
            push_shdr_value(L, shdr, entsize);
            lua_settable(L, i);
            (*elf)->section_headers[i] = shdr;
        }
        lua_setfield(L, -2, "section_headers");
    } else {
        luaL_error(L, "Not a valid ELF executable for this architecture\n");
        return -1;
    }

    free(buf);
    return 1;
}