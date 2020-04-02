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
#include"TH1.h"
#include"TH1F.h"
#include"TAxis.h"
#include"TF1.h"
#include"TFitResult.h"
#include"TFitResultPtr.h"
#include"TCanvas.h"
#include"TGraphErrors.h"
#include"THistPainter.h"




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
  double errore_k[NCH] ={0};
  double kap[NCH]= {0};
  double k_media = 0;
  double errore = 0;

  TH1F* hist0= new TH1F("hist0","Spettro Cesio [Ch: 0]", NBIN, NMIN, NMAX);
  TH1F* hist6 = new TH1F("hist6","Spettro Cesio [Ch: 6]", NBIN, NMIN, NMAX);


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


    kap[channel]= MEDIA/medie[channel];
    k_media += kap[channel];

    errore_k[channel] = kap[channel]/medie[channel]*errore_medie[channel]*errore_medie[channel];
    errore += errore_k[channel];


    TH1F* hist_scaled = new TH1F("hist_scaled",Form("Spettro Cesio calibrato [Ch: %d]", channel), NBIN, NMIN*kap[channel], NMAX*kap[channel]);

    for (int entry=0; entry<nEntries ; entry++) {
      tree->GetEntry(entry);
      if(-vcharge[channel]>CMIN){
        hist_scaled -> Fill(-vcharge[channel]*kap[channel]);
      }
    }


    TF1 *f2 = new TF1("f2", "gaus", kap[channel]*xmin[channel], NMAX);

    TFitResultPtr p = hist_scaled->Fit("f2", "SRQ");

    /**
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
    
    delete F;
    delete E;
    **/

    if(channel==0){
      for (int entry=0; entry<nEntries ; entry++) {
        tree->GetEntry(entry);
        if(-vcharge[channel]>CMIN){
          hist0 -> Fill(-vcharge[channel]);
        }
      }
    }
    if(channel==6){

      for (int entry=0; entry<nEntries ; entry++) {
        tree->GetEntry(entry);
        if(-vcharge[channel]>CMIN){
          hist6 -> Fill(-vcharge[channel]);
        }
      }

      TCanvas* H = new TCanvas("hist6","Sovrapposizione Istogrammi ch:1 e ch:7"); // Nome, Titolo,x,y
      hist6->SetTitle(";Carica (pC);Numero Eventi");
      hist6->GetYaxis()->SetRangeUser(0,2500);
      H -> cd(); // Apre una sessione
      hist6 -> Draw();
      hist0 -> Draw("SAME");
      H -> Update();
      H -> SaveAs(Form("%s/Istogrammi_ch0_ch6_sovrapposti.pdf", plotsDir.c_str()));
    }

// (TH1F*) *hist_new = (THIF*)hist->Clone("hist_new")

    delete hist_scaled;
    delete hist;

  }

/**
  TCanvas* H = new TCanvas("hist0","Sovrapposizione Istogrammi ch:1 e ch:4"); // Nome, Titolo,x,y
  hist0->SetTitle(";Energia (MeV);Numero Eventi");
  H -> cd(); // Apre una sessione
  hist0 -> Draw();
  H -> SaveAs(Form("%s/Istogrammi_ch0_ch3_sovrapposti.pdf", plotsDir.c_str()));
**/
  // Get average k with and creates graph

  k_media = k_media/NCH;
  errore = sqrt(errore)/NCH;

  //For the graphs

  Double_t channel_axis[NCH];
  Double_t ki[NCH];
  Double_t err_ki[NCH];
  Double_t *err_channel=0; // Dummy

  for(int channel=0; channel<NCH; channel++){
    channel_axis[channel]=channel+1; // grazie:)
    ki[channel]=kap[channel]/k_media;
    err_ki[channel]=errore_k[channel]/k_media/k_media + ki[channel]*ki[channel]/k_media/k_media*errore*errore;
    err_ki[channel] = sqrt(err_ki[channel]);
  }

  TCanvas* c2 = new TCanvas("c2", "Rapporto k/K");
  c2->cd();
  TGraphErrors* gr=new TGraphErrors(NCH, channel_axis, ki, err_channel, err_ki);
  gr->SetTitle(" ; Canale ;Rapporto k/K");
  gr->SetMarkerStyle(21);
  gr->SetMarkerSize(0.9);
  gr->GetXaxis()->SetTitle("Canale");
  gr->GetYaxis()->SetTitle("Rapporto k/K");
  gr->Draw("AP");
  c2->SaveAs(Form("%s/Rapporto_k_media.pdf", plotsDir.c_str()));

  double errore_1 = sqrt(2)*errore/k_media;

  std::cout<<"L'errore sulla media normalizzata ad 1 vale:"<<errore_1<<std::endl;

  return 0;

}
