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

INC_DIR = include_priv/
SRC_DIR = source

mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
#cur_makefile_path := patsubst$(%/, %, dir, $(mkfile_path))
LOCAL_CFLAGS = -c  -Wall -O3 -I $(INC_DIR) -DLINUX -DHAL_ALTERA -I ../../include_priv -I ../../../include/ -I ../../include/
ifeq ($(tool_chain), old)
	LOCAL_CFLAGS += -D__STDC_VERSION__=199901L
endif
LOCAL_LDFLAGS += -L ../../../prebuildlib  -lcam_engine -fPIC -shared


SRC = $(wildcard $(SRC_DIR)/*.c)

OBJS = $(patsubst %.c, %.o, $(SRC))
TARGET = libmipi_dphy.so
TARGET_Z = mipi_dphy.drv

.PHONY:  clean all install

all: $(TARGET)
	cp $(TARGET) $(TARGET_Z)
	@echo "---------$(TARGET) ok-----------------\n"

$(TARGET): $(OBJS)
	$(CC) $(LOCAL_LDFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.c
	@echo $(mkfile_path)
	#@echo $(cur_makefile_path)
	$(CC) $(LOCAL_CFLAGS) $(CFLAGS) -o $@ $<

clean:
	-$(RM) -f $(OBJS) $(TARGET) $(TARGET_Z)

install:
	@install -m 755 -d $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET) $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET_Z) $(DESTDIR)/usr/lib/

