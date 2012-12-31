# Makefile for SST
# By Steven Smith

CC= clang
ANALYZE= clang --analyze
WARNS= -W -Werror -Wall
CFLAGS= -g $(WARNS)
LFLAGS=
# Frameworks are a part of OS X compilation
FRAMEWORKS= Cocoa OpenGL IOKit
LIBS= glfw3
SRC= src
BUILD= build

# Example source(s)
EXAMPLES= $(EXAMPLE1) ${EXAMPLE2} ${EXAMPLE3}
EXAMPLE1= example1
EXAMPLE1_S= example.c
EXAMPLE2= example2
EXAMPLE2_S= example2.c
EXAMPLE3= example3
EXAMPLE3_S= example3.c

# SST Sources
SST_S= sst.c sst_matrix.c

# Helper function
getobjs= $(patsubst %.c,$(BUILD)/%.o,$(1))

# Derivative values
FWS= $(foreach fw,$(FRAMEWORKS),-framework $(fw))
LBS= $(foreach lb,$(LIBS),-l$(lb))

all: $(EXAMPLES)

$(EXAMPLE1): $(call getobjs, $(EXAMPLE1_S) $(SST_S))
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LBS) $(FWS)

$(EXAMPLE2): $(call getobjs, $(EXAMPLE2_S) $(SST_S))
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LBS) $(FWS)

$(EXAMPLE3): $(call getobjs, $(EXAMPLE3_S) $(SST_S))
	$(CC) $(CFLAGS) $(LFLAGS) -o $@ $^ $(LBS) $(FWS)

$(BUILD):
	mkdir $(BUILD)

$(BUILD)/%.o: $(SRC)/%.c | $(BUILD)
	$(ANALYZE) $(WARNS) $<
	$(CC) $(CFLAGS) -I $(SRC) -c -o $@ $<

clean:
	$(if $(wildcard $(BUILD)/*.o), $(RM) $(wildcard $(BUILD)/*.o))
	$(if $(wildcard $(BUILD)), rmdir $(BUILD))
	$(if $(wildcard $(EXAMPLES)), $(RM) $(EXAMPLES))

.PHONY: clean
