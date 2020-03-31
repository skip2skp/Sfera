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

  int ev;
  float base[NCH], vcharge[NCH], pshape[NCH][1024];

  tree->SetBranchAddress("ev", &ev);
  tree->SetBranchAddress("base", &base);
  tree->SetBranchAddress("vcharge", &vcharge);
  tree->SetBranchAddress("pshape", &pshape);

  int nEntries = tree->GetEntries();
  float rapporto;
  float vmin = 20;

  std::string plotsDir(Form("Istogramma_charge/"));
  system( Form("mkdir -p %s", plotsDir.c_str()) );
    
  for (int channel=0; channel<NCH; channel++) {

    TH1F* hist_e = new TH1F("hist_e", "distribuzione dei rapporti integrale/vcharge |Vcharge| superiori a 20 pC ", 100, -2, 2);
    TH1F* hist_e_tight = new TH1F("hist_e", "distribuzione dei rapporti integrale/vcharge |Vcharge| superiori a 20 pC ", 100, -0.01, 0.1);
    TH1F* hist = new TH1F("hist", "distribuzione dei rapporti integrale/vcharge ", 100, -2, 2);

    for (int entry=0; entry<nEntries ; entry++) {

      tree->GetEntry(entry);
      
      float sum=0;
      
      for (int i=0; i<1024; i++) {
	
    	sum+=pshape[channel][i]-pshape[channel][0];
	
      }

      rapporto = sum*=DT/vcharge[channel];

      if(vcharge[channel] < -vmin){
        hist_e -> Fill(rapporto);
        hist_e_tight -> Fill(rapporto);
      }
      hist -> Fill(rapporto);
    }


  TCanvas* C = new TCanvas("C","Istogramma rapporti con carica",600,800); // Nome, Titolo,x,y
  C -> cd(); // Apre una sessione
  hist_e -> Draw(); // Disegna l'istogramma
  C -> SaveAs(Form("%s/Istogramma_%d_carica.pdf", plotsDir.c_str(),channel));

  TCanvas* D = new TCanvas("C","Istogramma rapporti totale",600,800); // Nome, Titolo,x,y
  D -> cd(); // Apre una sessione
  hist -> Draw(); // Disegna l'istogramma
  D -> SaveAs(Form("%s/Istogramma_%d_totale.pdf", plotsDir.c_str(),channel));

  TCanvas* E = new TCanvas("C","Istogramma rapporti con carica Stretto",600,800); // Nome, Titolo,x,y
  E -> cd(); // Apre una sessione
  hist_e_tight -> Draw(); // Disegna l'istogramma
  E -> SaveAs(Form("%s/Istogramma_%d_carica_tight.pdf", plotsDir.c_str(),channel));

  delete hist; 
  delete hist_e;   

  }


}
