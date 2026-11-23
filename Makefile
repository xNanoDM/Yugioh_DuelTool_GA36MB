TARGET = ygo_calc
OBJS = src/main.o

CXXFLAGS = -O2 -G0 -Wall -std=c++17 -fno-exceptions -fno-rtti
ASFLAGS = $(CXXFLAGS)

LIBDIR =
LDFLAGS =
LIBS = -lpspdisplay -lpspctrl -lpspuser

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Yu-Gi-Oh! Duel Calc

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
