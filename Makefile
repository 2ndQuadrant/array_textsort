MODULE_big = array_textsort
OBJS = array_textsort.o

EXTENSION = array_textsort
DATA = array_textsort--1.1.sql array_textsort--unpackaged--1.1.sql

REGRESS = init array_textsort array_distinct teardown

PG_CONFIG ?= pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

