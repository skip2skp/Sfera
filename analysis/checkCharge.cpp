// Palmisano 29/03/2020
// Programma per integrare il profilo di impulso e confrontarlo con risultato ritornato dal digitizer.

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define DT 938E-3
#define CONVERSION 1

#include<iostream>
#include<vector>

#include"TFile.h"
#include"TTree.h"
#include"TString.h"
#include"TRandom3.h"
#include"TGraph.h"

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

  std::vector<double> diffs;
  std::vector<int> channels;

  // variabili da leggere: baseline, profilo, integrale calcolato dal digitizer

  long int ev;
  int nch;
  double tetime;
  double* base[NCH], vcharge[NCH], pshape[NCH][1024];

  tree->SetBranchAddres("ev", &ev);
  tree->SetBranchAddres("nch", &nch);
  tree->SetBranchAddres("tetime", &tetime);
  tree->SetBranchAddres("base", &base);
  tree->SetBranchAddres("vcharge", &vcharge);
  tree->SetBranchAddres("pshape", &pshape);

  long int nEntries = tree->GetEntries();

  for (long int entry=0; entry<nEntries ; entry++) {

    tree->GetEntry(entry);

    dobule diff=0;

    for (channel=0; channel<nch; channel++) {

      double sum=0;
      
      for (int i=0; i<1024; i++) {
      
	sum+=2*pshape[channel][i];
	
      }

      diff=base[channel]-(sum-pshape[channel][0]-pshape[channel][1024])*CONVERSION*DT/2.; // fattore di conversione per portare in pC, DT/2 dal metodo dei trapezi
      
  }

}
