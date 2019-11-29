#!/bin/sh

rm -f configure
autoconf
autoheader
sed -i 's/XrmInitialize ()/XOpenDisplay (0)/' configure
rm -rf autom4te.cache
