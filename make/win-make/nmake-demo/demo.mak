!include <Win32.Mak>

SRC = demo

all: $(OUTDIR) $(OUTDIR)\$(SRC).exe

$(OUTDIR):
	if not exist "$(OUTDIR)/$(NULL)" mkdir $(OUTDIR)

$(OUTDIR)\$(SRC).obj: $(SRC).c
	$(CC) $(CFLAGS) $(CVARS) /Fo"$(OUTDIR)\\" /Fd"$(OUTDIR)\\" $(SRC).c

$(OUTDIR)\$(SRC).exe: $(OUTDIR)\$(SRC).obj
	$(LINK) $(CONFLAGS) -out:$(OUTDIR)\$(SRC).exe $(OUTDIR)\$(SRC).obj $(CONLIBS)


clean:
	$(CLEANUP)
