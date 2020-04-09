CC = g++
CFLAGS = -Wall -c -g -fpermissive 


INCLUDES = -I. -I.. -I$(ROOTSYS)/include -I$(ROOFIT_INCLUDE)/ 

ROOTSYS ?= ERROR_RootSysIsNotDefined

ROOTFLAG = `${ROOTSYS}/bin/root-config --cflags --libs`

EXTRALIBS := -L$(ROOTSYS)/lib -L$(ROOFIT_LIBDIR)/ -lHtml -lMathCore -lGenVector -lMinuit -lEG  -lPhysics -lz -lTreePlayer -lRooFitCore -lRooFit


asciiToTree: asciiToTree.cpp 
	$(CC) -Wall $(INCLUDES) -o asciiToTree asciiToTree.cpp $(ROOTFLAG) $(EXTRALIBS)

measToTree: measToTree.cpp 
	$(CC) -Wall $(INCLUDES) -o measToTree measToTree.cpp $(ROOTFLAG) $(EXTRALIBS)

checkPulseShape: checkPulseShape.cpp 
	$(CC) -Wall $(INCLUDES) -o checkPulseShape checkPulseShape.cpp $(ROOTFLAG) $(EXTRALIBS)

makeSpectrum: makeSpectrum.cpp
	$(CC) -Wall $(INCLUDES) -o makeSpectrum makeSpectrum.cpp $(ROOTFLAG) $(EXTRALIBS)

checkCharge: checkCharge.cpp
	$(CC) -Wall $(INCLUDES) -o checkCharge checkCharge.cpp $(ROOTFLAG) $(EXTRALIBS)

calibrazione_energia: calibrazione_energia.cpp
	$(CC) -Wall $(INCLUDES) -o calibrazione_energia calibrazione_energia.cpp $(ROOTFLAG) $(EXTRALIBS)

fitcesio: fitcesio.cpp
	$(CC) -Wall $(INCLUDES) -o fitcesio fitcesio.cpp $(ROOTFLAG) $(EXTRALIBS)

all: asciiToTree checkPulseShape measToTree makeSpectrum checkCharge fitcesio


clean:
	rm asciiToTree checkPulseShape measToTree makeSpectrum checkCharge fitcesio


