ifeq ($(build),debug)
	TARGET=EposDriverTest
else
	TARGET=EposDriverTest
endif
build=release
arch=x86_64

CP=cp

CPPFLAGS=-Wall -std=c++11 -march=native -O0 -pthread -lX11
INCLUDES=\
        
SRCS+=$(wildcard *.cpp)
OBJS:=$(patsubst %.cpp,%.o,$(SRCS))

ifeq ($(build),debug)
	CPPFLAGS+=-g
else
	CPPFLAGS+=-O2
endif


	CC=g++ 
	LDFLAGS+=-lm -lpthread -ldl -lrt \
		/opt/EposCmdLib_6.6.2.0/lib/x86/libEposCmd.so.6.6.2.0 
$(TARGET): $(OBJS)
	$(CC) $^ -o $@  $(LDFLAGS)
all:		
	$(TARGET)

.cpp.o:
	$(CC) -c $< $(CPPFLAGS) $(INCLUDES) -o $@

.PHONY:clean all install
clean:
	$(RM) $(OBJS) $(TARGET)

