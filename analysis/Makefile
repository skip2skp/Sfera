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

make checkCharge: checkCharge.cpp
	$(CC) -Wall $(INCLUDES) -o checkCharge checkCharge.cpp $(ROOTFLAG) $(EXTRALIBS)

checkCharge_finale: checkCharge_finale.cpp
	$(CC) -Wall $(INCLUDES) -o checkCharge_finale checkCharge_finale.cpp $(ROOTFLAG) $(EXTRALIBS)

calibrazione_energia_finale: calibrazione_energia_finale.cpp
	$(CC) -Wall $(INCLUDES) -o calibrazione_energia_finale calibrazione_energia_finale.cpp $(ROOTFLAG) $(EXTRALIBS)


make fitcesio: fitcesio.cpp
	$(CC) -Wall $(INCLUDES) -o fitcesio fitcesio.cpp $(ROOTFLAG) $(EXTRALIBS)

make fitcesiosplit: fitcesiosplit.cpp
	$(CC) -Wall $(INCLUDES) -o fitcesiosplit fitcesiosplit.cpp $(ROOTFLAG) $(EXTRALIBS)


all: asciiToTree checkPulseShape measToTree makeSpectrum checkCharge calibrazione_energia fitcesio fitcesiosplit


clean:
	rm asciiToTree checkPulseShape measToTree makeSpectrum checkCharge calibrazione_energia fitcesio fitcesiosplit


