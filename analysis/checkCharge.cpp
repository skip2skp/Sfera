// Palmisano 29/03/2020
// Programma per integrare il profilo di impulso e confrontarlo con risultato ritornato dal digitizer.

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define DT 938E-3 // ns
#define CONVERSION 1

#define EVENT 5637
#define CHANNEL 10

#include<iostream>
#include<stdio.h>
//#include<vector>

#include"TFile.h"
#include"TTree.h"
#include"TString.h"
#include"TH1F.h"
#include"TCanvas.h"


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

  // istrogrammi
 

  
  // variabili da leggere: baseline, profilo, integrale calcolato dal digitizer

  int ev, nch;
  float base[NCH], vcharge[NCH], pshape[NCH][1024];

  tree->SetBranchAddress("ev", &ev);
  tree->SetBranchAddress("base", &base);
  tree->SetBranchAddress("vcharge", &vcharge);
  tree->SetBranchAddress("pshape", &pshape);

  int nEntries = tree->GetEntries();
  float rapporto;


  std::string plotsDir(Form("Istogramma_charge/"));
  system( Form("mkdir -p %s", plotsDir.c_str()) );
    
  for (int channel=0; channel<NCH; channel++) {

    TH1F* hist = new TH1F("hist", "distribuzione dei rapporti integrale/vcharge", 100, -2, 2);

    for (int entry=0; entry<nEntries ; entry++) {

      tree->GetEntry(entry);
      
      float sum=0;
      
      for (int i=0; i<1024; i++) {
	
    	sum+=pshape[channel][i]-pshape[channel][0];
	
      }

      rapporto = sum*=DT/vcharge[channel];
      hist -> Fill(rapporto);

    }


  TCanvas* C = new TCanvas("C","Primo Canvas",600,800); // Nome, Titolo,x,y
  C -> cd(); // Apre una sessione
  hist -> Draw(); // Disegna l'istogramma
  C -> SaveAs(Form("Istogramma%d.pdf",channel));

  delete hist;    

  }


}
