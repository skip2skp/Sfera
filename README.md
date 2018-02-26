# Welcome to the repository of Sfera

Change directory to `analysis` and there you will find two programs:
`asciToTree.cpp` is the program that reads an ascii measurements file and converts it to a ROOT TTree;
`checkPulseShape.cpp` will plot a single pulse shape, to check that everything is OK.

Compile `asciToTree.cpp` with the command `make asciToTree` and then execute it through `./asciToTree [fileName]`, where `fileName` needs to be a valid file in the `../data` directory, and needs to be formatted correctly. This is automatically the case if you are using an ascii file that was produced by the digitizer, with full pulse shape readout. An example of such file can be found in `../data/test_data_64ch.dat`, which is a file with one event and 64 channels read out from the digitizer; only the first channel is filled with a square wave, whereas the others are all noise. To run on it the command is `./asciToTree test_data_64ch.dat`. The program should also support giving it the relative path (`./asciToTree ../data/test_data_64ch.dat`) but make sure that you don't have extra dots in the file name (except for the final one in `.dat`), because that will create problems.

To run on a measurements-only file you need to use the `measToTree` program: compile it (`make measToFile`) and run it (`./measToFile [fileName]`) in exactly the same way as
`asciiToFile`, but make sure to feed it a properly formatted measurements-only file (an example can be found in `data/test_data_64ch_MeasurementsOnly.dat`).

These programs will produce an output rootfile with name consistent with the input datafile.

If you ran on an ASCII file with full pulse shape information, you can use the `checkPulseShape` program to plot a single pulse shape. Compile it (`make checkPulseShape`) and then run it `./checkPulseShape [rootFileName] [event] [channel]`.
