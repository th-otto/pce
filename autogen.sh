#!/bin/sh

rm -f configure
autoconf
autoheader
rm -rf autom4te.cache
