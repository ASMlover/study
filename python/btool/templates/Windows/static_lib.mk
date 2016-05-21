# library makefile

OUT = {out}
RM = del
CC = cl -c -nologo
AR = lib -nologo
CFLAGS = -GS -Zi -Fd"vc.pdb" -EHsc{cflags}{preprocessor}{inc_dir}
OBJS = {objs}
SRCS = {srcs}

all: $(OUT)

rebuild: clean all

clean:
	$(RM) $(OUT) $(OBJS) *.pdb *.ilk

$(OUT): $(OBJS)
	$(AR) -out:$(OUT) $(OBJS)

$(OBJS): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS)
