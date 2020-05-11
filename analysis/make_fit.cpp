
#define ERROR_USAGE 1
#define ERROR_NOTREE 2


#define NCH 16

#include<iostream>
#include<stdio.h>
#include<vector>
#include<cmath>
#include<fstream>

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

  //richieste in terminale
  if (argc!=4) {
    std::cout<<"Usage: "<<argv[0]<<" filename.root. \n Exiting."<<std::endl;
    exit(ERROR_USAGE);
  }


 int CMIN=atoi(argv[2]);    //chiedi soglia
 int channel=atoi(argv[3]); //chiedi canale
	
 
 
	
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
  //int flag_fit = 1;


  //crea cartelle e sottocartelle per i plot
  std::string plotsDir(Form("spettro/"));
  system( Form("mkdir -p %s", plotsDir.c_str()) );

  
  //genero file per risultati dei fit
	std::ofstream out_dat;
	out_dat.open("risultati.txt");
  

  
 

/********************************** PRINT SPECTRUM*********************************************************************************/
  	double Nmax=0,Nmin=0;
		
		
	for (int entry=0; entry<nEntries ; entry++) {
        tree->GetEntry(entry); 	// prendi l'evento i-esimo 
        if(-vcharge[channel]>Nmax){Nmax=-vcharge[channel];} 
				
      }
	int NBIN=100;

    TH1F* spettro = new TH1F("spettro",Form("Spettro  [Ch: %d]", channel), NBIN, CMIN, Nmax);
      for (int entry=0; entry<nEntries ; entry++) {
        tree->GetEntry(entry); 	// prendi l'evento i-esimo 
        if(-vcharge[channel]>CMIN){spettro -> Fill(-vcharge[channel]);} //riempi l'isto 
      }

		
  //*****************I fit solo gaussiano**********************************************************
   // TF1 *fitgaus1 = new TF1("fitgaus1", "gaus", xmin[channel], NMAX); //fit gaussiana 1
					
   /* TFitResultPtr gaussian_fit = spettro->Fit("fitgaus1", "SRQ");  //“Q” Quiet mode  “S” result in TFitResultPtr ""R” Use the range 
		Double_t Gamp1 =gaussian_fit->Parameter(0);	
		Double_t Gmean1 =gaussian_fit->Parameter(1);
		Double_t Gvar1 =gaussian_fit->Parameter(2);
		Double_t chi1 =gaussian_fit->Chi2();		
		Double_t Ndf1=gaussian_fit->Ndf();							
		out_dat<<"CH["<<channel<<"]"<<std::endl;		
		out_dat<<"G\t"<< Gamp1<<"\t"<<Gmean1<<"\t"<<Gvar1<<"\t"<<"chisq:"<<chi1<<"\t"<<Ndf1<<std::endl;*/
		
    //Stampiamo  tutto
    TCanvas* plot_spettro = new TCanvas("spettro",Form("Spettro  [Ch: %d]", channel),1920,1080);
    spettro->SetTitle(Form("Spettro  [Ch: %d];Energia (MeV);Numero Eventi",channel));
		plot_spettro->SetLogy();
    plot_spettro -> cd(); // Apre una sessione
    spettro -> Draw(); // Disegna l'istogramma
    plot_spettro -> SaveAs(Form("%s/Spettro_CH_%d.pdf", plotsDir.c_str(),channel));
     
    delete spettro;
    delete plot_spettro;
   // delete rootfitFunc;

//---------------------------------------------------------

std::cout<<"Choose minimum :"<<std::endl;
std::cin>>Nmin;
std::cout<<"\nChoose maximum :"<<std::endl;
std::cin>>Nmax;
std::cout<<"\nChoose bins :"<<std::endl;
std::cin>>NBIN;



	for(int channel=0;channel<16;channel++){
 TH1F* fit = new TH1F("spettro",Form("Spettro  [Ch: %d]", channel), NBIN, Nmin, Nmax);
      for (int entry=0; entry<nEntries ; entry++) {
        tree->GetEntry(entry); 	// prendi l'evento i-esimo 
    		fit -> Fill(-vcharge[channel]); //riempi l'isto 
      }
 TCanvas* plot_fit = new TCanvas("fit",Form("fit  [Ch: %d]", channel),1920,1080);
    fit->SetTitle(Form("fit  [Ch: %d];Energia (MeV);Numero Eventi",channel));
		//plot_fit->SetLogy();
    plot_fit -> cd(); // Apre una sessione
    fit -> Draw(); // Disegna l'istogramma
    plot_fit -> SaveAs(Form("%s/fit_CH_%d.pdf", plotsDir.c_str(),channel));
     
    delete fit;
    delete plot_fit;

}





	//chiudi file dei dati
	out_dat.close();
    
    
  return 0;

}
