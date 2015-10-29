all:test

CPPFLAGS+=-std=c++1z -Wall -pedantic
CPPFLAGS+=-g -O2 -DDEBUG -UNDEBUG
#BOOST_DIR=/mnt/LARGE/MODULAR_BOOST/modular-boost
BOOST_DIR=/home/sehe/custom/boost_1_59_0
CPPFLAGS+=-isystem /home/sehe/custom/nonius/include
CPPFLAGS+=-isystem $(BOOST_DIR)

# CPPFLAGS+=-fopenmp
CPPFLAGS+=-pthread
CPPFLAGS+=-march=native

LDFLAGS+=-L $(BOOST_DIR)/stage/lib/ -Wl,-rpath,$(BOOST_DIR)/stage/lib
LDFLAGS+=-lboost_system -lboost_regex -lboost_thread -lboost_iostreams -lboost_serialization
LDFLAGS+=-lboost_date_time -lboost_program_options -lboost_filesystem

#CXX=g++-5
#CXX=g++-4.9
# CXX=/usr/lib/gcc-snapshot/bin/g++
# CC=/usr/lib/gcc-snapshot/bin/gcc
CXX=clang++-3.6 #-stdlib=libc++
# CC=clang

%.S:%.cpp
	$(CXX) $(CPPFLAGS) $^ -S -masm=intel -o - | egrep -v '\s*\.' | c++filt > $@

%:%.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@ $(LDFLAGS)
