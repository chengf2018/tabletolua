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
add_char(struct convert_ctx* ctx, char c) {
    if (!c) return;
    if (ctx->now_page->size + 1 > STR_BUFF_SIZE) {
        ctx->now_page = ctx->now_page->next = alloc_str_page();
    }
    ctx->now_page->buff[ctx->now_page->size] = c;
    ctx->now_page->size += 1;
    ctx->total_len += 1;
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
    str[pos] = '\0';
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
/*
\"
\t'
\n
\a
\b
\f
\r
\v
\\
\'
*/
static void travrse_table(lua_State *L, struct convert_ctx* ctx, int index);

static void trans2escapechar(struct convert_ctx* ctx, const char* str, size_t len) {
    size_t i;
    for (i=0; i<len; i++) {
        char c = str[i];
        switch (c) {
            case '\"':
            add_string(ctx, "\\\"");
            break;
            case '\t':
            add_string(ctx, "\\t");
            break;
            case '\n':
            add_string(ctx, "\\n");
            break;
            case '\a':
            add_string(ctx, "\\a");
            break;
            case '\b':
            add_string(ctx, "\\b");
            break;
            case '\f':
            add_string(ctx, "\\f");
            break;
            case '\r':
            add_string(ctx, "\\r");
            break;
            case '\v':
            add_string(ctx, "\\v");
            break;
            case '\\':
            add_string(ctx, "\\\\");
            break;
            case '\'':
            add_string(ctx, "\\\'");
            break;
            default:
            add_char(ctx, c);
            break;
        }
    }
}

static void
tostringvalue(lua_State *L, struct convert_ctx* ctx, int index) {
    int type = lua_type(L, index);
    if (type == LUA_TTABLE) {
        travrse_table(L, ctx, index);
    } else if (type == LUA_TSTRING) {
        add_char(ctx, '\"');
        size_t len = 0;
        const char* str = lua_tolstring(L, index, &len);
        trans2escapechar(ctx, str, len);
        add_char(ctx, '\"');
    } else if (type == LUA_TNUMBER) {
        add_string(ctx, lua_tostring(L, index));
    } else if (type == LUA_TBOOLEAN) {
        if (lua_toboolean(L, index))
            add_string(ctx, "true");
        else
            add_string(ctx, "false");
    }
}

static void
travrse_table(lua_State *L, struct convert_ctx* ctx, int index) {
    int64_t i = 1, first = 0;
    add_char(ctx, '{');
    lua_pushvalue(L, index);
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        //-1value -2key
        char comma = ',';
        if (!first) {
            first = 1;
            comma = 0;
        }
        if (lua_isinteger(L, -2) && lua_tointeger(L, -2) == i) {
            add_char(ctx, comma);
            tostringvalue(L, ctx, -1);
            i++;
        } else if (lua_type(L, -2) == LUA_TNUMBER){
            add_char(ctx, comma);
            add_char(ctx, '[');
            int64_t key = lua_tointeger(L, -2);
            char str[32] = {0};
            sprintf(str, "%ld", key);
            add_string(ctx, str);
            add_string(ctx, "]=");
            tostringvalue(L, ctx, -1);
        } else if (lua_type(L, -2) == LUA_TSTRING) {
            add_char(ctx, comma);
            add_string(ctx, lua_tostring(L, -2));
            add_char(ctx, '=');
            tostringvalue(L, ctx, -1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);
    add_char(ctx, '}');
}

static int
lt2ls(lua_State *L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    struct convert_ctx ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.now_page = (struct str_page*)&ctx.first_page;

    travrse_table(L, &ctx, 1);
    build_string(&ctx);

    lua_pushstring(L, ctx.build_str);

    clean_ctx(&ctx);
    return 1;
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