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
#include"TGraphErrors.h"



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
  double errore_medie[NCH] ={0};
  double kap[NCH]= {0};
  float  k_media = 0;
  double errore = 0;

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
      errore_medie[channel] = r->ParError(1);

      errore += errore_medie[channel]*errore_medie[channel];
        

      kap[channel]= MEDIA/medie[channel];
      k_media += kap[channel];

        TH1F* hist_scaled = new TH1F("hist_scaled",Form("Spettro Cesio calibrato [Ch: %d]", channel), NBIN, NMIN*kap[channel], NMAX*kap
[channel]);
        
        for (int entry=0; entry<nEntries ; entry++) {
          tree->GetEntry(entry);
          if(-vcharge[channel]>CMIN){
             hist_scaled -> Fill(-vcharge[channel]*kap[channel]);
          }
        }
        

        TF1 *f2 = new TF1("f2", "gaus", kap[channel]*xmin[channel], NMAX);

        TFitResultPtr p = hist_scaled->Fit("f2", "SRQ");
/*
*
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

        **/

/**
        TH1F* hist3_scaled = new TH1F("hist3_scaled","Spettro Cesio ch 3 Scalato ", NBIN, NMIN*k[channel], NMAX*k[channel]);
        TH1F* hist0_scaled = new TH1F("hist0_scaled","Spettro Cesio ch 0 Scalato ", NBIN, NMIN*k[channel], NMAX*k[channel]);

        if(channel==0){
          hist0_scaled->SetBinContent(channel,hist_scaled[channel]);
          hist0->SetBinContent(channel,hist[channel]);
        };
        if(channel==3){
          hist3_scaled->SetBinContent(channel,hist_scaled[channel]);
          hist3->SetBinContent(channel,hist[channel]);
        };

        **/
        delete hist_scaled;
        delete hist;
        /**
        delete F;
        delete E;
        **/
    }

    // Get average k with and creates graph

    k_media = k_media/NCH;
    errore = sqrt(errore);

  Double_t channel_axis[NCH];
  Double_t ki[NCH];
  Double_t err_ki[NCH];
  Double_t *err_channel=0; // Dummy

    for(int channel=0; channel<NCH; channel++){

    channel_axis[channel]=channel+1; // grazie:)
    ki[channel]=kap[channel]/k_media;
    err_ki[channel]=errore_medie[channel]/k_media; //VA AGGIUNTO UN CONTRIBUTO
    std::cout<<err_ki[channel] <<" "<<k_media<<std::endl;

    }

  TCanvas* c2 = new TCanvas("c2", "Rapporto k/k_media",600,800);
  c2->cd();
  TGraphErrors* gr=new TGraphErrors(NCH, channel_axis, ki, err_channel, err_ki);
  gr->SetTitle(" ; Canale ;Rapporto k/k_media");
  gr->SetMarkerStyle(21);
  gr->SetMarkerSize(0.9);
  gr->GetXaxis()->SetTitle("Canale");
  gr->GetYaxis()->SetTitle("Rapporto k/k_media");
  gr->Draw("AP");
  c2->SaveAs(Form("%s/Rapporto_k_media.pdf", plotsDir.c_str()));





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
