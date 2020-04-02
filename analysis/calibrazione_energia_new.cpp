//Questo programma prende in esame il file del cesio e fa la calibraqzione dei canali

//Francesco 01/04/20

//rev Lorenzo 03/04/20  commenti & plot 1920*1080 & cambiati nomi alle variabili & eliminate alcune righe inutilizzate
//    eliminati warnings di compilazione & new-delete for all & cartelle per i plot

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

  //richieste in terminale
  if (argc!=2) {
    std::cout<<"Usage: "<<argv[0]<<" filename.root. \n Exiting."<<std::endl;
    exit(ERROR_USAGE);
  }

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
  std::string plotsDir(Form("spettro_Cesio/"));
  system( Form("mkdir -p %s", plotsDir.c_str()) );

  
  std::string plotsDir1(Form("spettro_Cesio/spettro/"));
  std::string plotsDir2(Form("spettro_Cesio/spettro_calib/"));

  system( Form("mkdir -p %s", plotsDir1.c_str()) );
  system( Form("mkdir -p %s", plotsDir2.c_str()) );
  

	//minimo energia per fit gaussiana calcolato visivamente [come implementarlo meglio?] fit solo coda destra
	  double xmin[NCH] = {950, 650, 650, 750, 850, 800, 700, 750, 700, 700, 900, 950, 700, 800, 770, 750};
	  double medie[NCH] ={0};
  	double errore_medie[NCH] ={0};
  	double kap[NCH]= {0};
  	float  k_media = 0;
  	double errore = 0;

 // TH1F* hist0 = new TH1F("hist0","Spettro Cesio ch 0 ", NBIN, NMIN, NMAX);
  //TH1F* hist3 = new TH1F("hist3","Spettro Cesio ch 3 ", NBIN, NMIN, NMAX);

/***********************************BEGIN FIT AND K CALC*********************************************************************************/

  for (int channel=0; channel<NCH; channel++) {
        
      TH1F* spettro = new TH1F("spettro",Form("Spettro Cesio [Ch: %d]", channel), NBIN, NMIN, NMAX);


      for (int entry=0; entry<nEntries ; entry++) {
          tree->GetEntry(entry); 	// prendi l'evento i-esimo 
            
          if(-vcharge[channel]>CMIN){
              spettro -> Fill(-vcharge[channel]); //riempi l'isto
          }

      }
      
	
      TF1 *fitgaus1 = new TF1("fitgaus1", "gaus", xmin[channel], NMAX); //fit gaussiana 1
      TFitResultPtr fit_result_1=spettro->Fit("fitgaus1", "SRQ");

	medie[channel] = fit_result_1->Parameter(1);
	errore_medie[channel] = fit_result_1->ParError(1);
	errore += errore_medie[channel]*errore_medie[channel];
        

	kap[channel]= MEDIA/medie[channel];
	k_media += kap[channel];

        TH1F* spettro_calib = new TH1F("spettro_calib",Form("Spettro Cesio calibrato [Ch: %d]", channel), NBIN, NMIN*kap[channel], NMAX*kap[channel]);
        
        for (int entry=0; entry<nEntries ; entry++) {
          tree->GetEntry(entry);
          if(-vcharge[channel]>CMIN){
             spettro_calib -> Fill(-vcharge[channel]*kap[channel]);
          }
        }
        
	
        TF1 *fitgaus2 = new TF1("fitgaus2", "gaus", kap[channel]*xmin[channel], NMAX);  //fit gaussiana 2
        TFitResultPtr fit_result_2 = spettro_calib->Fit("fitgaus2", "SRQ");

	//Printiamo  tutto
        TCanvas* plot_spettro = new TCanvas("spettro",Form("Spettro Cesio [Ch: %d]", channel),1920,1080);
        spettro->SetTitle(Form("Spettro Cesio [Ch: %d];Energia (MeV);Numero Eventi",channel));
        plot_spettro -> cd(); // Apre una sessione
        spettro -> Draw(); // Disegna l'istogramma
        plot_spettro -> SaveAs(Form("%s/Ist_Spettro_Cs_%d.pdf", plotsDir1.c_str(),channel));
 	

	 TCanvas* plot_spettro_calib = new TCanvas("spettro_calib",Form("Spettro Cesio scalato [Ch: %d]", channel),1920,1080); 
        spettro_calib->SetTitle(Form("Spettro Cesio Scalato [Ch: %d];Energia (MeV);Numero Eventi",channel));
        plot_spettro_calib -> cd(); // Apre una sessione
        spettro_calib -> Draw(); // Disegna l'istogramma
        plot_spettro_calib -> SaveAs(Form("%s/Ist_Spettro_Cs_%d_Calib.pdf", plotsDir2.c_str(),channel));

        
	
        delete spettro;
        delete spettro_calib;
       	delete plot_spettro_calib;
	delete plot_spettro;
	delete fitgaus1;
	delete fitgaus2;

 } // fine for sui canali

/****************************END FIT AND K CALC****************************************************************************************/

/*****************************************II PART : K AVERAGE**************************************************************************/

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

  TCanvas* c2 = new TCanvas("c2", "Rapporto k/k_media",1920,1080);
  c2->cd();
  TGraphErrors* gr=new TGraphErrors(NCH, channel_axis, ki, err_channel, err_ki);
  gr->SetTitle(" ; Canale ;Rapporto k/k_media");
  gr->SetMarkerStyle(21);
  gr->SetMarkerSize(0.9);
  gr->GetXaxis()->SetTitle("Canale");
  gr->GetYaxis()->SetTitle("Rapporto k/k_media");
  gr->Draw("AP");
  c2->SaveAs(Form("%s/Rapporto_k_media.pdf", plotsDir.c_str()));






    
    return 0;

}
