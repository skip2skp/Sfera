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
#include"TPad.h"


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

  double xmin[NCH] = {950, 650, 650, 750, 850, 800, 700, 750, 700, 700, 900, 950, 700, 800, 770, 750};
  double medie[NCH] ={0};
  double k[NCH]= {0};


  TH1F* hist0 = new TH1F("hist0","Spettro Cesio ch 0 ", NBIN, NMIN, NMAX);
  TH1F* hist3 = new TH1F("hist3","Spettro Cesio ch 3 ", NBIN, NMIN, NMAX);


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
        

        TF1 *f2 = new TF1("f2", "gaus", k[channel]*xmin[channel], NMAX);

        TFitResultPtr p = hist_scaled->Fit("f2", "SRQ");

        TCanvas* E = new TCanvas("hist",Form("Spettro Cesio [Ch: %d]", channel),600,800); // Nome, Titolo,x,y
        hist->SetTitle(Form("Spettro Cesio [Ch: %d];Energia (MeV);Numero Eventi",channel));
        E -> cd(); // Apre una sessione
        hist -> Draw(); // Disegna l'istogramma
        E -> SaveAs(Form("%s/Istogramma_Spettro_Cesio_%d.pdf", plotsDir.c_str(),channel));

        TCanvas* F = new TCanvas("hist_scaled",Form("Spettro Cesio Scalato [Ch: %d]", channel),600,800); // Nome, Titolo,x,y
        hist_scaled->SetTitle(Form("Spettro Cesio Scalato [Ch: %d];Energia (MeV);Numero Eventi",channel));
        F -> cd(); // Apre una sessione
        hist_scaled -> Draw(); // Disegna l'istogramma
        F -> SaveAs(Form("%s/Istogramma_Spettro_Cesio_%d_Calibrato.pdf", plotsDir.c_str(),channel));

/**
        TH1F* hist3_scaled = new TH1F("hist3_scaled","Spettro Cesio ch 3 Scalato ", NBIN, NMIN*k[channel], NMAX*k[channel]);
        TH1F* hist0_scaled = new TH1F("hist0_scaled","Spettro Cesio ch 0 Scalato ", NBIN, NMIN*k[channel], NMAX*k[channel]);

        if(channel==0){
          hist0_scaled = hist_scaled;
          hist0 = hist;
        };
        if(channel==3){
          hist3_scaled = hist_scaled;
          hist3 = hist;
        };

        **/
        delete hist_scaled;
        delete hist;
        delete F;
        delete E;


    }
/**
    TCanvas* H = new TCanvas("hist0","Sovrapposizione Istogrammi ch:1 e ch:4",600,800); // Nome, Titolo,x,y
    hist0->SetTitle("Sovrapposizione Istogrammi ch:1 e ch:4;Energia (MeV);Numero Eventi");
    H -> cd(); // Apre una sessione
    hist0 -> Draw();
    TPad* p11 = new TPad("hist3","Sovrapposizione Istogrammi ch:1 e ch:4",0,0, 1200, 2200);
    p11->cd();
    hist3->Draw();
    H->Update();
    p11->Update();
    H -> SaveAs(Form("%s/Istogrammi_ch0_ch3_sovrapposti.pdf", plotsDir.c_str()));
**/



    
    return 0;

}
