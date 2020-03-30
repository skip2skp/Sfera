// Palmisano 29/03/2020
// Programma per integrare il profilo di impulso e confrontarlo con risultato ritornato dal digitizer.

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define DT 938E-3
#define CONVERSION 1

#define EVENT 5637
#define CHANNEL 10

#include<iostream>
//#include<vector>

#include"TFile.h"
#include"TTree.h"
#include"TString.h"
#include"TRandom3.h"
//#include"TGraph.h"

int main(int argc, char* argv[]) {
  
  if (argc!=2) {
    std::cout<<"Usage: "<<argv[0]<<" filename.root. \n Exiting."<<std::endl;
    exit(ERROR_USAGE);
  }

  // file dove leggere il Tree

  TString rootFileName(argv[1]);
  TFile* rootFile = new TFile(rootFileName);
  std::cout<<"Reading data from root file "<<argv[1]<<std::endl;

  TTree* tree = (TTree*) rootFile->Get("tree");
  if(!tree) {
    std::cout<<"Error, no tree called tree in "<<argv[1]<<". Exiting."<<std::endl;
    exit(ERROR_NOTREE);
  }

  //std::vector<double> diffs;
  //std::vector<int> channels;

  // variabili da leggere: baseline, profilo, integrale calcolato dal digitizer

  int ev;
  int nch;
  double base[NCH], vcharge[NCH], pshape[NCH][1024];

  tree->SetBranchAddress("ev", &ev);
  tree->SetBranchAddress("nch", &nch);
  tree->SetBranchAddress("base", &base);
  tree->SetBranchAddress("vcharge", &vcharge);
  tree->SetBranchAddress("pshape", &pshape);

  long int nEntries = tree->GetEntries();

  for (long int entry=0; entry<nEntries ; entry++) {

    tree->GetEntry(entry);
    
    double diff=0;

    for (int channel=0; channel<nch; channel++) {
      
      double sum=0;
      
      for (int i=0; i<1024; i++) {
	
    	sum+=pshape[channel][i];
	
      }
      
      diff=base[channel]-sum*CONVERSION*DT; // fattore di conversione per portare in pC
      
    }
  

  }

}
