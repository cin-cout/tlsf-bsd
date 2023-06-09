OUT = build

TARGETS = \
	test \
	bench
TARGETS := $(addprefix $(OUT)/,$(TARGETS))

all: $(TARGETS)

run: all
	sudo taskset -c 5 ./build/bench

CFLAGS += \
  -std=gnu11 -g -O2 \
  -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wconversion -Wc++-compat \
  -DTLSF_ENABLE_ASSERT -DTLSF_ENABLE_CHECK

OBJS = tlsf.o
OBJS := $(addprefix $(OUT)/,$(OBJS))
deps := $(OBJS:%.o=%.o.d)

$(OUT)/test: $(OBJS) test.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OUT)/bench: $(OBJS) bench.c
	$(CC) $(CFLAGS) -o $@ -MMD -MF $@.d $^ $(LDFLAGS)

$(OUT)/%.o: %.c
	@mkdir -p $(OUT)
	$(CC) $(CFLAGS) -c -o $@ -MMD -MF $@.d $<

CMDSEP = ; echo "Please wait..." ;
check: $(TARGETS)
	MALLOC_CHECK_=3 $(foreach prog,$(TARGETS),./$(prog) $(CMDSEP))

clean:
	$(RM) $(TARGETS) $(OBJS) $(deps)

.PHONY: all check clean test

-include $(deps)
