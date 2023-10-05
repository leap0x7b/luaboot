#include <stdint.h>
#include <luaboot/efi.h>
#include <luaboot/luamod.h>
#include <lua/lua.h>
#include <lua/lauxlib.h>

int framebuffer_put_pixel(lua_State *L) {
    uint32_t x = lua_tonumber(L, 1);
    uint32_t y = lua_tonumber(L, 2);
    uint32_t color = lua_tonumber(L, 3);
    lua_pop(L, 3);
    ((uint32_t *)GOP->Mode->FrameBufferBase)[x + GOP->Mode->Info->HorizontalResolution * y] = color;
    return 0;
}

static int framebuffer_index(lua_State* L) { 
    uint32_t **fb = luaL_checkudata(L, 1, "framebuffer");
    int index = luaL_checkinteger(L, 2);
    lua_pushnumber(L, (*fb)[index - 1]);
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
    lua_pushnumber(L, GOP->Mode->FrameBufferSize);
    return 1; 
}

// jesee we need to cook
static const struct luaL_Reg fb_array_meth[] = {
    { "__index", framebuffer_index },
    { "__newindex", framebuffer_newindex },
    { "__len", framebuffer_len },
    { "putPixel", put_pixel },
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

static const struct luaL_Reg lib[] = {
    { "test", NULL },
    { NULL, NULL }
};

int luaopen_luaboot(lua_State *L) {
    lua_newtable(L);
    luaL_newmetatable(L, "framebuffer");
    lua_pushvalue(L, -1);
    lua_setfield(L, -2, "__index");
    luaL_setfuncs(L, fb_array_meth, 0);

    lua_newtable(L);

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

    luaL_setfuncs(L, lib, 0);

    //luaL_newlib(L, lib);
    return 1;
}