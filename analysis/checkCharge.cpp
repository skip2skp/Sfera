// Palmisano 29/03/2020
// Programma per integrare il profilo di impulso e confrontarlo con risultato ritornato dal digitizer.

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define DT 938E-3 // ns

#include<iostream>
#include<stdio.h>
//#include<vector>

#include"TFile.h"
#include"TTree.h"
#include"TString.h"
#include"TH1F.h"
#include"TH1.h"
#include"TCanvas.h"
#include"TGraphErrors.h"


int main(int argc, char* argv[]) {

  // Check ussage
  
  if (argc!=2) {
    std::cout<<"Usage: "<<argv[0]<<" filename.root. \n Exiting."<<std::endl;
    exit(ERROR_USAGE);
  }

  bool signalOnly=1; // flag per selezionare solo gli eventi che hanno rilasciato una carica superiore a min_charge
  float min_charge=20;
  
  // file dove leggere il Tree

  TString rootFileName(argv[1]);
  TFile* rootFile = new TFile(rootFileName);
  std::cout<<"Reading data from root file "<<argv[1]<<std::endl;

  TTree* tree = (TTree*) rootFile->Get("tree");
  if(!tree) {
    std::cout<<"Error, no tree called tree in "<<argv[1]<<". Exiting."<<std::endl;
    exit(ERROR_NOTREE);
  }
  
  // variabili da leggere: numero evento, baseline, profilo, integrale calcolato dal digitizer

  int ev;
  float base[NCH], vcharge[NCH], pshape[NCH][1024];

  tree->SetBranchAddress("ev", &ev);
  tree->SetBranchAddress("base", &base);
  tree->SetBranchAddress("vcharge", &vcharge);
  tree->SetBranchAddress("pshape", &pshape);

  int nEntries = tree->GetEntries();

  // Cartella dei grafici
  
  std::string plotsDir(Form("plots_checkcharge/"));
  system( Form("mkdir -p %s", plotsDir.c_str()) );

  // per grafico somma/carica vs canale

  Double_t channel_axis[NCH];
  Double_t charge_axis[NCH];
  Double_t err_charge[NCH];
  Double_t *err_channel=0; // Dummy
    
  for (int channel=0; channel<NCH; channel++) {

    TH1F* hist = new TH1F("hist", "distribuzione dei rapporti integrale/vcharge ", 100, -0.05, 0.5);

    for (int entry=0; entry<nEntries ; entry++) {

      tree->GetEntry(entry);
      
      float sum=0;
      
      for (int i=0; i<1024; i++) {
	
    	sum+=pshape[channel][i]-pshape[channel][0]; //sottrae valore base. Si può far meglio, facendo una media dei primi valori
	
      }
      
      if(!signalOnly) hist->Fill(sum*DT/vcharge[channel]); // Se signalOnly=0 salva tutti gli eventi
      else if(vcharge[channel] < -min_charge) hist->Fill(sum*DT/vcharge[channel]); // Altrimenti solo quelli con |carica|>min_charge
      
    }

    // per gli istogrammi
    
    TCanvas* c1 = new TCanvas("c1","Istogramma Rapporti della Carica Misurata vs. Riportata",600,800); // Nome, Titolo,x,y
    c1->cd();
    hist->SetTitle("Istogramma Rapporti della Carica Misurata vs. Riportata");
    hist->GetXaxis()->SetTitle("Rapporto della Carica Misurata vs. Riportata");
    hist->GetYaxis()->SetTitle("Numero Eventi");
    hist->Draw();
    c1->SaveAs(Form("%s/hist_charge_%d.pdf", plotsDir.c_str(),channel));
    

    // salva variabili per grafico somma/carica vs canale
    
    channel_axis[channel]=channel+1; // grazie:)
    charge_axis[channel]=hist->GetMean();
    err_charge[channel]=hist->GetStdDev();
    
    
    delete hist;
    //delete c1;
    
    
  }

  // per grafico somma/carica vs canale
  
  TCanvas* c2 = new TCanvas("c2", "Confronto Medie e Std Dev di R = Integrale/Carica ");
  c2->cd();
  TGraphErrors* gr=new TGraphErrors(NCH, channel_axis, charge_axis, err_channel, err_charge);
  gr->SetTitle("Confronto Medie e Std Dev di R = Integrale / Charge; Canale ;Media di R = Integrale/Carica");
  gr->SetMarkerStyle(21);
  gr->SetMarkerSize(0.9);
  gr->GetXaxis()->SetTitle("Canale");
  gr->GetYaxis()->SetTitle("Rapporto Integrale del profilo/carica");
  gr->Draw("AP");
  c2->SaveAs(Form("%s/confronto_medie_e_DevStandard.pdf", plotsDir.c_str()));
  
  delete gr;
  //delete c2;
 
  return 0;

}
