##################################################################
#
# Round for C
#
# Copyright (C) Satoshi Konno 2015
#
# This is licensed under BSD-style license, see file COPYING.
#
##################################################################

bin_PROGRAMS = roundd

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/src -I../


roundd_SOURCES = \
	..//roundd.cpp

roundd_LDADD = ../../../lib/unix/libround.a @XML_LIBS@
