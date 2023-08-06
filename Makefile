LUA_INC=../../lua-5.4.6/
LUA_CLIB=../../lua-5.4.6/liblua.a
CFLAGS= -g -Wall -fPIC -I$(LUA_INC) -std=gnu99
SHARED= --shared

all : tabletolua.so

tabletolua.so : tabletolua.c
	gcc $(CFLAGS) $(SHARED) $^ -o $@

clean :
	rm -rf ./tabletolua.so