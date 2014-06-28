
export PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)


# Postgres
POSTGRES = /Library/PostgreSQL/9.3
POSTGRES_INC = ${POSTGRES}/include/postgresql/server

# Installation path 
INSTPATH=${PWD}

# Compiler and options
CC=gcc
CFLAGS=-bundle -flat_namespace -undefined suppress -fpic -O -I${POSTGRES_INC}
LDFLAGS=-L/usr/local/lib -lgslcblas -lgsl

# Install
INSTALL=install

all: statsfunc.so  createall.sql

statsfunc.so: statsfunc.c
	${CC} -o $@ $< ${CFLAGS} ${LDFLAGS}

createall.sql: createall.sql.in
	sed 's|@INSTPATH@|${INSTPATH}|' $< > $@

clean:
	rm -f statsfunc.so createall.sql *.core *~ #*#
