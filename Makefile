# ©2013 YUICHIRO NAKADA

#CROSS=mipsel-uclibc-
CC=$(CROSS)g++
LD=$(CROSS)ld
AR=$(CROSS)ar
RANLIB=$(CROSS)ranlib

#CCFLAGS+=-Os -Wall -I. -Wno-pointer-sign -fomit-frame-pointer
CCFLAGS+=-Os -Wall -I. -I./include -fomit-frame-pointer -Wno-missing-braces
LDFLAGS+=-s -L./lib.x86 -L/usr/X11R6/lib
LIBS=-lcatcake -lasound -lmad -lfreetype -lpng -ljpeg -lz -lGL -lXxf86vm -lpthread -lX11

PACKAGE=kikyu
OBJS=$(PACKAGE).o


# for Windows (Digital Mars C++)
WCC=		wine ./dm/bin/dmc
WCFLAGS=	-I./dm/stlport/stlport -I. -D__LCC__
## http://curl.haxx.se/download.html
# wine dm/bin/implib.exe /s curl.lib libcurl.dll
# wine dm/bin/implib.exe /s miniupnpc.lib miniupnpc.dll
#WLDFLAGS=	SoftPixelEngine.lib
WOBJS=$(PACKAGE).cpp


all: $(PACKAGE) pyon 3d

$(PACKAGE): $(OBJS)
	$(CC) $(CCFLAGS) $(LDFLAGS) $(OBJS) -o $(PACKAGE) $(LIBS)

pyon: pyon.o
	$(CC) $(CCFLAGS) $(LDFLAGS) pyon.o -o pyon $(LIBS)
3d: 3d.o
	$(CC) $(CCFLAGS) $(LDFLAGS) 3d.o -o 3d $(LIBS)

%.o: %.cpp
	$(CC) $(CCFLAGS) -c $<

%.o: %.c
	$(CC) $(CCFLAGS) -c $<

win: $(WOBJS)
	$(WCC) $(WCFLAGS) $(WOBJS) $(WLDFLAGS)

android:
	sh /root/prog/catcake/project/android/android-app/build.sh kikyu Kikyu kikyu.cpp assets
	sh /root/prog/catcake/project/android/android-app/build.sh pyon Pyon pyon.cpp assets.pyon

clean:
	rm -f *.o *.obj *.map $(PACKAGE)
