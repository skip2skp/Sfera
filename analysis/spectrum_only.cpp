// Lorenzo

// versione 15/09/20

//Questo programma fa i grafici dalle measurement

#define ERROR_USAGE 1
#define ERROR_NOTREE 2


#define NCH 16
#define CMIN 50
#define NMIN 0
#define NMAX 2300
#define FIT_START 500
#define NBIN 100
#define MEDIA 661.7

#include<iostream>
#include<stdio.h>
#include<vector>
#include<cmath>
#include<fstream>
#include<iomanip>

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
#include "TLegend.h"
#include "TGaxis.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TPaveText.h"
#include "TLatex.h"

int main(int argc, char* argv[]) {
int bin=atoi(argv[2]);
int min=atoi(argv[3]);
int max=atoi(argv[4]);
//int scelta2=atoi(arv[4]);
  //richieste in terminale
  if (argc!=5  ) {
    std::cout<<"Usage: "<<argv[0]<<" filename.root.\t bin \t min \t max \n Exiting."<<std::endl;
		  exit(1);
}
char folder[20];
std::cout<<	"name of folder?"<<std::endl;
std::cin>>folder;
char element[20];
std::cout<<	"name for graphs)?"<<std::endl;
std::cin>>element;
  // lettura Tree e controllo

  TString rootFileName(argv[1]);
  TFile* rootFile = new TFile(rootFileName);
  std::cout<<"Reading data from root file "<<argv[1]<<std::endl;

  TTree* tree = (TTree*) rootFile->Get("tree");
  if(!tree) {
    std::cout<<"Error, no tree called tree in "<<argv[1]<<". Exiting."<<std::endl;
    exit(ERROR_NOTREE);
 	 }
	
	
  // variabili da leggere: baseline, profilo, integrale calcolato dal digitizer

  int ev;
  float base[NCH], vcharge[NCH];

  tree->SetBranchAddress("ev", &ev);
  tree->SetBranchAddress("base", &base);
  tree->SetBranchAddress("vcharge", &vcharge);

  int nEntries = tree->GetEntries();  // # di righe tree = # eventi
  

  //crea cartelle e sottocartelle per i plot
  std::string plotsDir(Form("%s",folder));
  system( Form("mkdir -p %s", plotsDir.c_str()) );



  for (int channel=0; channel<NCH; channel++) {  
    TH1F* spettro = new TH1F("spettro",Form("Spettro %s [Ch: %d]",element, channel), bin, min, max);
			
		

    
		
			for (int entry=0; entry<nEntries ; entry++) {
        tree->GetEntry(entry); 	// prendi l'evento i-esimo 
        if(-vcharge[channel]>CMIN){spettro -> Fill(-vcharge[channel]);} //riempi l'isto 
      }


	  
  //Stampiamo  tutto
   TCanvas* plot_spettro = new TCanvas("spettro","",1920,1080);
    spettro->SetStats(0); // Leva il pannello con entries mean devstd
	// spettro->SetTitle(Form("Spettro Cesio [Ch: %d];Carica (pC);Numero Eventi",channel));
	 spettro->GetYaxis()->SetTitle("Numero Eventi");
	 spettro->GetXaxis()->SetTitle("Carica [pC]");
	 plot_spettro -> cd(); // Apre una sessione
	

   spettro -> Draw(); // Disegna l'istogramma
	
 TLatex *l = new TLatex(0.85,0.87,Form( "ch[%d]",channel));
l->SetNDC();
l->SetTextSize(0.04);
l->Draw("same");


		
	plot_spettro -> SaveAs(Form("%s/Ist_Spettro_%s_%d.pdf",element, plotsDir.c_str(),channel));

    delete plot_spettro;
      

      
    delete spettro;
    

  } 

  // fine for sui canali


	
  return 0;

}
