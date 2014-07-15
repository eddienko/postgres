# Postgres
POSTGRES = /Library/PostgreSQL/9.3
POSTGRES_INC = ${POSTGRES}/include/postgresql/server

# WCSLIB 
WCSLIB = /usr/local

# Installation path 
INSTPATH=${PWD}

# Compiler and options
CC=gcc
CFLAGS=-bundle -flat_namespace -undefined suppress -fpic -O -I${POSTGRES_INC}
LDFLAGS=-I${WCSLIB}/include/wcslib -L${WCSLIB}/lib -lwcs

# Install
INSTALL=install

all: wcsfunc.so createall.sql

wcsfunc.so: wcsfunc.c
	${CC} -o $@ $< ${CFLAGS} ${LDFLAGS}

createall.sql: createall.sql.in
	sed 's|@INSTPATH@|${INSTPATH}|' $< > $@

install: wcsfunc.so
	${INSTALL} -d -u 0 -g 0 -m 755 ${INSTPATH}
	${INSTALL} -u 0 -g 0 -m 755 wcsfunc.so ${INSTPATH}

clean:
	rm -f wcsfunc.so createall.sql *.core *~ #*#
