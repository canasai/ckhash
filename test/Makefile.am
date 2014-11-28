bin_PROGRAMS = test-cuckoo

test_cuckoo_SOURCES = test-cuckoo.c

AM_CPPFLAGS = -W -Wall -I$(top_srcdir)/cuckoo_hash

test_cuckoo_LDADD = -L$(top_srcdir)/cuckoo_hash -lckhash -lm
test_cuckoo_DEPENDENCIES = $(top_srcdir)/cuckoo_hash/libckhash.a
