// Palmisano 29/03/2020
// Programma per integrare il profilo di impulso e confrontarlo con risultato ritornato dal digitizer.

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define CMIN 30
#define NMIN 0
#define NMAX 1200
#define NBIN 100
#define MEDIA 661.7

#include<iostream>
#include<stdio.h>
#include<vector>

#include"TFile.h"
#include"TTree.h"
#include"TString.h"
#include"TH1F.h"
#include"TAxis.h"
#include"TF1.h"
#include"TFitResult.h"
#include"TFitResultPtr.h"
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
  float base[NCH], vcharge[NCH];

  tree->SetBranchAddress("ev", &ev);
  tree->SetBranchAddress("base", &base);
  tree->SetBranchAddress("vcharge", &vcharge);

  int nEntries = tree->GetEntries();
  int flag_fit = 1;


  std::string plotsDir(Form("Istogramma_spettro_Cesio/"));
  system( Form("mkdir -p %s", plotsDir.c_str()) );

  double xmin[NCH] = {950, 650, 650, 750, 850, 800, 700, 750, 700, 700, 900, 950, 700, 800, 700, 700};
  double medie[NCH] ={0};
  double k[NCH]= {0};


    for (int channel=0; channel<NCH; channel++) {
        
        TH1F* hist = new TH1F("hist",Form("Spettro Cesio [Ch: %d]", channel), NBIN, NMIN, NMAX);

        for (int entry=0; entry<nEntries ; entry++) {
            tree->GetEntry(entry);
            
            if(-vcharge[channel]>CMIN){
                hist -> Fill(-vcharge[channel]);
            }

        }

        TF1 *f1 = new TF1("f1", "gaus", xmin[channel], NMAX);

        TFitResultPtr r = hist->Fit("f1", "SRQ");

        medie[channel] = r->Parameter(1);
        

        k[channel]= MEDIA/medie[channel];

        TH1F* hist_scaled = new TH1F("hist_scaled",Form("Spettro Cesio calibrato [Ch: %d]", channel), NBIN, NMIN*k[channel], NMAX*k[channel]);
        
        for (int entry=0; entry<nEntries ; entry++) {
          tree->GetEntry(entry);
          if(-vcharge[channel]>CMIN){
             hist_scaled -> Fill(-vcharge[channel]*k[channel]);
          }
        }

        TCanvas* E = new TCanvas("hist","Spettro Cesio",600,800); // Nome, Titolo,x,y
        E -> cd(); // Apre una sessione
        hist -> Draw(); // Disegna l'istogramma
        E -> SaveAs(Form("%s/Istogramma_Spettro_Cesio_%d.pdf", plotsDir.c_str(),channel));

        TCanvas* F = new TCanvas("hist","Spettro Cesio Scalato",600,800); // Nome, Titolo,x,y
        F -> cd(); // Apre una sessione
        hist_scaled -> Draw(); // Disegna l'istogramma
        F -> SaveAs(Form("%s/Istogramma_Spettro_Cesio_%d_Calibrato.pdf", plotsDir.c_str(),channel));


        delete hist;
        delete hist_scaled;

    }







    
    return 0;

}
