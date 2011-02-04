#!/bin/sh

DATABYTES=$((1024*1024*1024))

for name in sha1 skein256 ; do
	echo "benchmark ${name}"
	echo "# $name; $DATABYTES total data per size"> bench-$name.data
	for size in 1 3 4 8 9 16 31 32 64 ; do
		../benchtool $name $DATABYTES $size >> bench-$name.data
	done
done
