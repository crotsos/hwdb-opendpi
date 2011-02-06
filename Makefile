all: dpilogger dpipersist

dpilogger: dpilogger.o srpc.o tslist.o endpoint.o ctable.o stable.o crecord.o mem.o
	libtool --mode=link gcc -g -O -o dpilogger opendpi/src/lib/libopendpi.la -lpcap \
	-lpthread dpilogger.o srpc.o tslist.o endpoint.o ctable.o stable.o crecord.o mem.o \
	-lm libhashish/lib/libhashish.a

dpipersist: dpipersist.o srpc.o tslist.o endpoint.o ctable.o stable.o crecord.o mem.o
	libtool --mode=link gcc -g -O -o dpilogger opendpi/src/lib/libopendpi.la -lpcap \
	-lpthread dpipersist.o srpc.o tslist.o endpoint.o ctable.o stable.o crecord.o mem.o \
	-lm libhashish/lib/libhashish.a

dpilogger.o: dpilogger.c config.h srpcdefs.h
	gcc -Iopendpi/src/include/ -Ilibhashish/include/ -g -c dpilogger.c

dpipersist.o: dpipersist.c config.h srpcdefs.h
	gcc -Iopendpi/src/include/ -Ilibhashish/include/ -g -c dpipersist.c

srpc.o: srpc.c srpc.h 
	gcc -g -c srpc.c


tslist.o: tslist.c tslist.h mem.h
	gcc -g -c tslist.c

endpoint.o: endpoint.c endpoint.h mem.h
	gcc -g -c endpoint.c

ctable.o: ctable.c ctable.h endpoint.h crecord.h
	gcc -g -c ctable.c 

mem.o: mem.c mem.h
	gcc -g -c mem.c

stable.o: stable.c stable.h mem.h tslist.h
	gcc -g -c stable.c

crecord.o: stable.c crecord.h ctable.h mem.h endpoint.h stable.h
	gcc -g -c crecord.c

clean:
	rm -rf *~ *.o dpilogger .libs/

debug:
	libtool --mode=execute gdb dpilogger