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

make calibrazione_energia_finale: calibrazione_energia_finale.cpp
	$(CC) -Wall $(INCLUDES) -o calibrazione_energia_finale calibrazione_energia_finale.cpp $(ROOTFLAG) $(EXTRALIBS)

make fitcesio: fitcesio.cpp
	$(CC) -Wall $(INCLUDES) -o fitcesio fitcesio.cpp $(ROOTFLAG) $(EXTRALIBS)

make fitcesiosplit: fitcesiosplit.cpp
	$(CC) -Wall $(INCLUDES) -o fitcesiosplit fitcesiosplit.cpp $(ROOTFLAG) $(EXTRALIBS)

make cobalto: cobalto.cpp
	$(CC) -Wall $(INCLUDES) -o cobalto cobalto.cpp $(ROOTFLAG) $(EXTRALIBS)

make fondo: fondo.cpp
	$(CC) -Wall $(INCLUDES) -o fondo fondo.cpp $(ROOTFLAG) $(EXTRALIBS)

make sodio: sodio.cpp
	$(CC) -Wall $(INCLUDES) -o sodio sodio.cpp $(ROOTFLAG) $(EXTRALIBS)

make make_fit: make_fit.cpp
	$(CC) -Wall $(INCLUDES) -o make_fit make_fit.cpp $(ROOTFLAG) $(EXTRALIBS)

make studio_lin: studio_lin.cpp
	$(CC) -Wall $(INCLUDES) -o studio_lin studio_lin.cpp $(ROOTFLAG) $(EXTRALIBS)

make ris_en: ris_en.cpp
	$(CC) -Wall $(INCLUDES) -o ris_en ris_en.cpp $(ROOTFLAG) $(EXTRALIBS)

make spectrum_only: spectrum_only.cpp
	$(CC) -Wall $(INCLUDES) -o spectrum_only spectrum_only.cpp $(ROOTFLAG) $(EXTRALIBS)

make meas1: meas1.cpp
	$(CC) -Wall $(INCLUDES) -o meas1 meas1.cpp $(ROOTFLAG) $(EXTRALIBS)
	
make calibration: calibration.cpp
	$(CC) -Wall $(INCLUDES) -o calibration calibration.cpp $(ROOTFLAG) $(EXTRALIBS)	
	
	

all: asciiToTree checkPulseShape measToTree makeSpectrum checkCharge calibrazione_energia fitcesio fitcesiosplit cobalto sodio fondo make_fit studio_lin ris_en spectrum_only meas1 calibration



clean:
	rm asciiToTree checkPulseShape measToTree makeSpectrum checkCharge calibrazione_energia fitcesio fitcesiosplit cobalto sodio fondo make_fit  studio_lin ris_en spectrum_only meas1 calibration


