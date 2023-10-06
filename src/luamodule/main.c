#include <stdint.h>
#include <luaboot/io.h>
#include <luaboot/efi.h>
#include <luaboot/string.h>
#include <luaboot/luamod.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>

static int framebuffer_put_pixel(lua_State *L) {
    uint32_t x = lua_tointeger(L, 1);
    uint32_t y = lua_tointeger(L, 2);
    uint32_t color = lua_tointeger(L, 3);
    ((uint32_t *)GOP->Mode->FrameBufferBase)[x + GOP->Mode->Info->HorizontalResolution * y] = color;
    lua_pop(L, 3);
    return 0;
}

static int framebuffer_index(lua_State* L) { 
    uint32_t **fb = luaL_checkudata(L, 1, "framebuffer");
    int index = luaL_checkinteger(L, 2);
    lua_pushinteger(L, (*fb)[index - 1]);
    return 1; 
}

static int framebuffer_newindex(lua_State* L) { 
    uint32_t **fb = luaL_checkudata(L, 1, "framebuffer");
    int index = luaL_checkinteger(L, 2);
    int value = luaL_checkinteger(L, 3);
    (*fb)[index - 1] = value;
    return 0; 
}

static int framebuffer_len(lua_State* L) { 
    uint32_t **_ = luaL_checkudata(L, 1, "framebuffer");
    (void)_;
    lua_pushinteger(L, GOP->Mode->FrameBufferSize);
    return 1; 
}

// jesee we need to cook
static const struct luaL_Reg fb_array_meth[] = {
    { "__index", framebuffer_index },
    { "__newindex", framebuffer_newindex },
    { "__len", framebuffer_len },
    { NULL, NULL }
};

static int framebuffer_as_array(lua_State* L) {
    uint32_t **fb = lua_newuserdata(L, GOP->Mode->FrameBufferSize);
    *fb = (uint32_t *)GOP->Mode->FrameBufferBase;
    luaL_getmetatable(L, "framebuffer");
    lua_setmetatable(L, -2);
    return 1;
}

static const struct luaL_Reg fb_meth[] = {
    { "putPixel", framebuffer_put_pixel },
    { "asArray", framebuffer_as_array },
    { NULL, NULL }
};

static int outb(lua_State *L) {
    uint16_t port = luaL_checkinteger(L, 1);
    uint8_t value = luaL_checkinteger(L, 2);
    io_outb(port, value);
    lua_pop(L, 2);
    return 0;
}

static int inb(lua_State *L) {
    uint16_t port = luaL_checkinteger(L, 1);
    lua_pushinteger(L, io_inb(port));
    return 1;
}

static int outw(lua_State *L) {
    uint16_t port = luaL_checkinteger(L, 1);
    uint16_t value = luaL_checkinteger(L, 2);
    io_outw(port, value);
    lua_pop(L, 2);
    return 0;
}

static int inw(lua_State *L) {
    uint16_t port = luaL_checkinteger(L, 1);
    lua_pushinteger(L, io_inw(port));
    return 1;
}

static int outl(lua_State *L) {
    uint16_t port = luaL_checkinteger(L, 1);
    uint16_t value = luaL_checkinteger(L, 2);
    io_outl(port, value);
    lua_pop(L, 2);
    return 0;
}

static int inl(lua_State *L) {
    uint16_t port = luaL_checkinteger(L, 1);
    lua_pushinteger(L, io_inl(port));
    return 1;
}

typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
} cpuid_t;

static void _cpuid(uint32_t leaf, cpuid_t *dest) {
    asm volatile("cpuid" : "=a"((*dest).eax), "=b"((*dest).ebx), "=c"((*dest).ecx), "=d"((*dest).edx) : "a"(leaf));
}

static int cpuid_tostring(lua_State* L) { 
    cpuid_t *cpuid_ud = luaL_checkudata(L, 1, "cpuid");

    char string[13];
    *(uint32_t *)&string[0] = cpuid_ud->ebx;
    *(uint32_t *)&string[4] = cpuid_ud->edx;
    *(uint32_t *)&string[8] = cpuid_ud->ecx;
    string[12] = '\0';

    lua_pushstring(L, string);
    return 1; 
}

static const struct luaL_Reg cpuid_meth[] = {
    { "__tostring", cpuid_tostring },
    { NULL, NULL }
};

static int cpuid(lua_State *L) {
    uint32_t leaf = luaL_checkinteger(L, 1);

    cpuid_t *ret = lua_newuserdata(L, sizeof(cpuid_t));
    _cpuid(leaf, ret);

    luaL_newmetatable(L, "cpuid");
    luaL_setfuncs(L, cpuid_meth, 0);

    lua_newtable(L);

    lua_pushinteger(L, ret->eax);
    lua_setfield(L, -3, "eax");

    lua_pushinteger(L, ret->ebx);
    lua_setfield(L, -3, "ebx");

    lua_pushinteger(L, ret->ecx);
    lua_setfield(L, -3, "ecx");

    lua_pushinteger(L, ret->edx);
    lua_setfield(L, -3, "edx");

    luaL_getmetatable(L, "cpuid");
    lua_setmetatable(L, -2);

    return 1;
}

static const struct luaL_Reg lib[] = {
    { "outb", outb },
    { "inb", inb },
    { "outw", outw },
    { "inw", inw },
    { "outl", outl },
    { "inl", inl },
    { "cpuid", cpuid },
    { NULL, NULL }
};

int luaopen_luaboot(lua_State *L) {
    lua_newtable(L);

    luaL_newmetatable(L, "framebuffer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, fb_array_meth, 0);

    lua_newtable(L);

    lua_pushinteger(L, GOP->Mode->FrameBufferBase);
    lua_setfield(L, -2, "address");

    lua_pushinteger(L, GOP->Mode->FrameBufferSize);
    lua_setfield(L, -2, "size");

    lua_pushinteger(L, GOP->Mode->Info->HorizontalResolution);
    lua_setfield(L, -2, "width");

    lua_pushinteger(L, GOP->Mode->Info->VerticalResolution);
    lua_setfield(L, -2, "height");

    lua_pushinteger(L, 32);
    lua_setfield(L, -2, "depth");

    lua_pushinteger(L, GOP->Mode->Info->HorizontalResolution * 4);
    lua_setfield(L, -2, "pitch");

    luaL_setfuncs(L, fb_meth, 0);
    lua_setfield(L, -2, "framebuffer");

    lua_newtable(L);
    lua_setfield(L, -2, "config");

    luaL_setfuncs(L, lib, 0);

    return 1;
}