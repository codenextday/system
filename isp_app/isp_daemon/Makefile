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
CXX	= ${CROSS_COMPILE}g++

RM  := rm

INC_DIR = -Iinclude		\
	-I../include/		\
	-I$(LINUX_INC)		\
	-I../isi/include/	\
	-I../include/debug/	\
	-I$(QT_XML_INCLUDE_DIR)	\
	-I$(QT_INCLUDE_DIR)	\
	-I$(QT_CORE_INCLUDE_DIR)\

LIB_DIR = -L ../prebuildlib	\
	-lQtCore		\
	-lQtXml			\
	-lcam_engine		\
	-lcamif			\
	-lgpio			\
	-lsom_ctrl		\
	-lembUnit		\
	-ldl			\
	-pthread		\
	-lc			\
	-lm			\

QT_INCLUDE_DIR = ../qt_include/include/
QT_CORE_INCLUDE_DIR = ../qt_include/include/QtCore/
QT_XML_INCLUDE_DIR = ../qt_include/include/QtXml/

ifdef LINUX_OVERRIDE_SRCDIR
	LINUX_INC = $(LINUX_OVERRIDE_SRCDIR)/include/soc/imi
else
	LINUX_INC = ../../../linux-4.14/include/soc/imi
endif

LOCAL_CXXFLAGS = -c  -Wall -O3 $(INC_DIR) -DLINUX -DHAL_ALTERA
ifeq ($(tool_chain), old)
	LOCAL_CXXFLAGS += -D__STDC_VERSION__=199901L
endif
LOCAL_CXXFLAGS += -Wno-error=write-strings

LOCAL_LDFLAGS += $(LIB_DIR)

SRC_DIR = source
SRC = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(patsubst %.cpp, %.o, $(SRC))
TARGET = isp_app

.PHONY:  clean all install

all: $(TARGET)
	@echo "---------$(TARGET) ok-----------------\n"

$(TARGET): $(OBJS)
	$(CXX) $(LOCAL_LDFLAGS) $(LDFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(LOCAL_CXXFLAGS) $(CXXFLAGS) -o $@ $<

clean:
	-$(RM) -f $(OBJS) $(TARGET)

install:
	@install -m 755 -d $(DESTDIR)/usr/lib/
	@install -m 755 -D $(TARGET) $(DESTDIR)/usr/lib/
