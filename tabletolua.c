#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>

#define STR_PAGE_SIZE 4096
#define STR_BUFF_SIZE (STR_PAGE_SIZE-sizeof(struct str_page))

struct str_page
{
    uint16_t size;
    struct str_page* next;
    char buff[0];
};

struct str_base_page
{
    uint16_t size;
    struct str_page* next;
    char buff[STR_BUFF_SIZE];
};

struct convert_ctx
{
    struct str_base_page first_page;
    struct str_page* now_page;
    size_t total_len;
    char* build_str;
};

static struct str_page*
alloc_str_page() {
    struct str_page* page = (struct str_page*)malloc(STR_PAGE_SIZE);
    memset(page, 0, STR_PAGE_SIZE);
    return page;
}

static void
add_string(struct convert_ctx* ctx, const char* str) {
    size_t len = strlen(str);
    while (len) {
        if (ctx->now_page->size + len <= STR_BUFF_SIZE) {
            memcpy(ctx->now_page->buff + ctx->now_page->size, str, len);
            ctx->now_page->size += len;
            ctx->total_len += len;
            return;
        }
        size_t remaining_size = STR_BUFF_SIZE - ctx->now_page->size;
        memcpy(ctx->now_page->buff + ctx->now_page->size, str, remaining_size);
        ctx->now_page->size = STR_BUFF_SIZE;
        ctx->now_page = ctx->now_page->next = alloc_str_page();
        str += remaining_size;
        len -= remaining_size;
        ctx->total_len += remaining_size;
    }
}

static void
build_string(struct convert_ctx* ctx) {
    if (ctx->total_len < STR_BUFF_SIZE) {
        ctx->build_str = ctx->first_page.buff;
        return;
    }
    char* str = (char*)malloc(ctx->total_len+1);
    struct str_page* p = (struct str_page*)&ctx->first_page;
    size_t pos = 0;
    do {
        memcpy(str + pos, p->buff, p->size);
        pos += p->size;
        p = p->next;
    } while (p);
    str[pos] = 0;
    ctx->build_str = str;
}

static void
clean_ctx(struct convert_ctx* ctx) {
    if (ctx->build_str && ctx->total_len > STR_BUFF_SIZE) free(ctx->build_str);
    struct str_page* p = ctx->first_page.next;
    while (p) {
        struct str_page* next = p->next;
        free(p);
        p = next;
    }
    ctx->build_str = NULL;
    ctx->first_page.size = 0;
    ctx->first_page.next = NULL;
}

static void
travrse_table(lua_State *L, struct convert_ctx* ctx, int index) {
    lua_pushnil(L);
    int i = 1;
    while (lua_next(L, index)) {
        //-1value -2key
        int value_type = lua_type(L, -2);
        if (value_type == LUA_TNUMBER) {
            if (lua_tonumber(L, -2) == i) {
                add_string(ctx, ",");
                i++;
            }
        } else {

        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
}

static int
lt2ls(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    struct convert_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.now_page = (struct str_page*)&ctx.first_page;


    /*
    struct convert_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.now_page = (struct str_page*)&ctx.first_page;

    add_string(&ctx, "[", 1);
    const char* str = "--123456789abcdefghijklmnopqrstuvwxyz--";
    add_string(&ctx, str, strlen(str));
    add_string(&ctx, str, strlen(str));
    add_string(&ctx, str, strlen(str));
    add_string(&ctx, str, strlen(str));
    add_string(&ctx, "]", 1);

    build_string(&ctx);

    printf("build_string len:%lu\n", ctx.total_len);
    printf(ctx.build_str);

    clean_ctx(&ctx);*/


    return 0;
};



int
luaopen_tabletolua(lua_State *L) {
	luaL_checkversion(L);
	luaL_Reg l[] = {
		{"toluastring",  lt2ls},
	  	{NULL, NULL}
	};
	lua_createtable(L, 0, (sizeof(l)) / sizeof(luaL_Reg) - 1);
	luaL_setfuncs(L, l, 0);
	return 1;
}