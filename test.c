#include <stdio.h>
#include "tabletolua.c"

int main() {
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

    clean_ctx(&ctx);

    printf("\?");
    return 0;
}