# Based on Makefile from <URL: http://hak5.org/forums/index.php?showtopic=2077&p=27959 >

PROGRAM = GridWalkingServer

############# Main application #################
all:    $(PROGRAM)
.PHONY: all

# source files
DEBUG_INFO = YES
#USE_ODBC_CONNECTION = YES
SOURCES = $(shell find -L . -name '*.cpp'|grep -v "/example/"|sort)
OBJECTS = $(SOURCES:.cpp=.o)
DEPS = $(OBJECTS:.o=.dep)

######## compiler- and linker settings #########
CXX = g++
CXXFLAGS = -I/usr/local/include/restbed -I/usr/local/include -I/usr/include -W -Wall -Werror -pipe -std=c++14

ifdef DEBUG_INFO
 CXXFLAGS += -g -DDBG
 LIBSFLAGS = -L/usr/local/lib/restbed -L/usr/local/lib -lPocoDatad -lPocoFoundationd
else
 CXXFLAGS += -O3
 LIBSFLAGS = -L/usr/local/lib/restbed -L/usr/local/lib -lPocoData -lPocoFoundation
endif

ifdef USE_ODBC_CONNECTION
 CXXFLAGS += -DUSE_ODBC_CONNECTION
 ifdef DEBUG_INFO
  LIBSFLAGS += -lPocoDataODBCd
 else
  LIBSFLAGS += -lPocoDataODBC
 endif
else
 ifdef DEBUG_INFO
  LIBSFLAGS += -lPocoDataMySQLd
 else
  LIBSFLAGS += -lPocoDataMySQL
 endif
endif

LIBSFLAGS += -lrestbed -lpthread

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

%.dep: %.cpp
	$(CXX) $(CXXFLAGS) -MM $< -MT $(<:.cpp=.o) > $@


############# Main application #################
$(PROGRAM):	$(OBJECTS) $(DEPS)
	$(CXX) -o $@ $(OBJECTS) $(LIBSFLAGS)

################ Dependencies ##################
ifneq ($(MAKECMDGOALS),clean)
include $(DEPS)
endif

################### Clean ######################
clean:
	find . -name '*~' -delete
	-rm -f $(PROGRAM) $(OBJECTS) $(DEPS)

install:
	strip -s $(PROGRAM)
