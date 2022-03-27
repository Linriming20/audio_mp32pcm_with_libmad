src := main.c
static_lib := lib/libmad.a

CC := gcc
CFLAGS := -I./include
LDFLAGS :=

ifeq ($(DEBUG), 1)
CFLAGS += -DENABLE_DEBUG
endif

all: mp32pcm


mp32pcm: $(src) $(static_lib)
	$(CC) $^ $(CFLAGS) $(LDFLAGS) -o $@

clean:
	rm -f mp32pcm out*
.PHONY := clean
