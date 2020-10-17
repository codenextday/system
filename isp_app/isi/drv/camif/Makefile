export ARCH=arm

tool_chain=new

ifeq ($(tool_chain), old)
	export CROSS_COMPILE=arm-imi-linux-gnueabi-
	SYS_PATH=/home/tool/arm-imi-linux-gnueabi/bin
	export PATH=${SYS_PATH}:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin
else
	CROSS_COMPILE = arm-hjimi-linux-gnueabi-
endif

CC	= ${CROSS_COMPILE}gcc
CXX	= ${CC}

RM  := rm

INC_DIR = -I../../../include/ebase/	\
	-I../../include			\

LIB_DIR =

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
#cur_makefile_path := patsubst$(%/, %, dir, $(mkfile_path))
LOCAL_CFLAGS = -c  -Wall -O3 -DLINUX -DHAL_ALTERA $(INC_DIR)
ifeq ($(tool_chain), old)
	LOCAL_CFLAGS += -D__STDC_VERSION__=199901L
endif
LOCAL_LDFLAGS += $(LIB_DIR) -fPIC -shared

SRC_DIR = source
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(patsubst %.c, %.o, $(SRC))
TARGET = libcamif.so

.PHONY:  clean all install

all: $(TARGET)
	@echo "---------$(TARGET) ok-----------------\n"

$(TARGET): $(OBJS)
	$(CC) $(LOCAL_LDFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	@echo $(mkfile_path)
	#@echo $(cur_makefile_path)
	$(CC) $(LOCAL_CFLAGS) $(CFLAGS) -o $@ $<

clean:
	-$(RM) -f $(OBJS) $(TARGET)

install:
	@install -m 755 -d $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET) $(DESTDIR)/usr/lib/

