//Questo programma prende in esame il file del cesio e fa la calibraqzione dei canali

//Francesco 01/04/20

//rev Lorenzo 03/04/20  commenti & plot 1920*1080 & cambiati nomi alle variabili & eliminate alcune righe inutilizzate
//    eliminati warnings di compilazione & new-delete for all & cartelle per i plot

// Lorenzo Controllo fit passo passo 08/04/20

#define ERROR_USAGE 1
#define ERROR_NOTREE 2


#define NCH 16
#define CMIN 30
#define NMIN 0
#define NMAX 1200
#define FIT_START 350
#define NBIN 100
#define MEDIA 661.7

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


Double_t fermiDirac(Double_t *x, Double_t *par) {

	return par[0]/(1+exp((x[0]-par[1])*par[2])); // NB par[0] = ampiezza, par[1]="potenziale chimico", par[2]=1/"temperatura"

}

/*Double_t MyExp(Double_t *x, Double_t *par) {

	return par[0]*exp(x[0]/par[1]);

}*/

// Double_t gaussian(Double_t *x, Double_t *par) {

//   return par[0]*exp(-(x[0]-par[1])*(x[0]-par[1])/(2*par[2])); // NB par[0]=ampiezza, par[1]=media, par[2]= varianza della distribuzione, non la sigma

// }

Double_t background(Double_t *x, Double_t *par) {

  return par[0] + x[0]*par[1] + x[0]*x[0]*par[2];// + x[0]*x[0]*x[0]*par[3];
  //  return par[0]*exp(par[1]*x[0])
  
}

Double_t fitFunc(Double_t *x, Double_t *par) { 
  return fermiDirac(x, &par[3])+ par[0]*TMath::Gaus(x[0], par[1], par[2]);

}

Double_t fitFunc1(Double_t *x, Double_t *par) { 
  return fermiDirac(x, &par[3]) + fermiDirac(x, &par[6]) + par[0]*TMath::Gaus(x[0], par[1], par[2]);

}

Double_t fitFunc2(Double_t *x, Double_t *par) { 
  return fermiDirac(x, &par[3]) + fermiDirac(x, &par[6]) + par[0]*TMath::Gaus(x[0], par[1], par[2])+background(x,&par[9]);

}

/*Double_t fitFunc3(Double_t *x, Double_t *par) { 
  return fermiDirac(x, &par[3]) + fermiDirac(x, &par[6]) + par[0]*TMath::Gaus(x[0], par[1], par[2])+MyExp(x,&par[9]);

}*/


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
  //genero file per risultati dei fit
	std::ofstream out_dat;
	out_dat.open("risultati.txt");
  

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
        if(-vcharge[channel]>CMIN){spettro -> Fill(-vcharge[channel]);} //riempi l'isto 
      }

		
  //*****************I fit solo gaussiano**********************************************************
    TF1 *fitgaus1 = new TF1("fitgaus1", "gaus", xmin[channel], NMAX); //fit gaussiana 1
					
    TFitResultPtr gaussian_fit = spettro->Fit("fitgaus1", "SRQ");  //“Q” Quiet mode  “S” result in TFitResultPtr ""R” Use the range 
		Double_t Gamp1 =gaussian_fit->Parameter(0);	
		Double_t Gmean1 =gaussian_fit->Parameter(1);
		Double_t Gvar1 =gaussian_fit->Parameter(2);
		Double_t chi1 =gaussian_fit->Chi2();		
		Double_t Ndf1=gaussian_fit->Ndf();							
		out_dat<<"CH["<<channel<<"]"<<std::endl;		
		out_dat<<"G\t"<< Gamp1<<"\t"<<Gmean1<<"\t"<<Gvar1<<"\t"<<"chisq:"<<chi1<<"\t"<<Ndf1<<std::endl;
		
		//*****************II fit gauss+FD**********************************************************
    TF1* rootfitFunc = new TF1("rootfitFunc", fitFunc, FIT_START, NMAX,6);
      
    rootfitFunc->SetParameter(0,Gamp1);
    rootfitFunc->SetParameter(1,Gmean1);
    rootfitFunc->SetParameter(2,Gvar1);
		rootfitFunc->SetParameter(3,600);
    rootfitFunc->SetParameter(4,650);
    rootfitFunc->SetParameter(5,0.01);
      

    TFitResultPtr fit_result_1=spettro->Fit("rootfitFunc", "SQR");
		Double_t Gamp2 =fit_result_1->Parameter(0);	
		Double_t Gmean2 =fit_result_1->Parameter(1);
		Double_t Gvar2 =fit_result_1->Parameter(2);	
		Double_t chi2 =fit_result_1->Chi2();		
		Double_t Ndf2=fit_result_1->Ndf();														
		out_dat<<"G+FD\t"<< Gamp2<<"\t"<<Gmean2<<"\t"<<Gvar2<<"\t"<<"chisq:"<<chi2<<"\t"<<Ndf2<<std::endl;
	     

		//*****************II fit gauss+FD+FD**********************************************************
    TF1* rootfitFunc2 = new TF1("rootfitFunc2", fitFunc1, FIT_START, NMAX,9);
      
    rootfitFunc2->SetParameter(0,Gamp1);
    rootfitFunc2->SetParameter(1,Gmean1);
    rootfitFunc2->SetParameter(2,Gvar1);
		rootfitFunc2->SetParameter(3,600);
    rootfitFunc2->SetParameter(4,600);
    rootfitFunc2->SetParameter(5,0.01);
    rootfitFunc2->SetParameter(6,200);
    rootfitFunc2->SetParameter(7,200);
    rootfitFunc2->SetParameter(8,0.03);

    TFitResultPtr fit_result_2=spettro->Fit("rootfitFunc2", "SQR");
		Double_t Gamp3 =fit_result_2->Parameter(0);	
		Double_t Gmean3 =fit_result_2->Parameter(1);
		Double_t Gvar3 =fit_result_2->Parameter(2);	
		Double_t chi3 =fit_result_2->Chi2();		
		Double_t Ndf3=fit_result_2->Ndf();														
		out_dat<<"G+FD+FD\t"<< Gamp3<<"\t"<<Gmean3<<"\t"<<Gvar3<<"\t"<<"chisq:"<<chi3<<"\t"<<Ndf3<<std::endl;
	      

			
		//*****************III fit gauss+FD+FD+BG(lin-IIorder)**********************************************************
    TF1* rootfitFunc3= new TF1("rootfitFunc3", fitFunc2, FIT_START, NMAX, 12);
      
    rootfitFunc3->SetParameter(0,Gamp1);
    rootfitFunc3->SetParameter(1,Gmean1);
    rootfitFunc3->SetParameter(2,Gvar1);
		rootfitFunc3->SetParameter(3,300);
    rootfitFunc3->SetParameter(4,300);
    rootfitFunc3->SetParameter(5,0.01);
    rootfitFunc3->SetParameter(6,200);
    rootfitFunc3->SetParameter(7,200);
    rootfitFunc3->SetParameter(8,0.03);
		rootfitFunc3->SetParameter(9,0);
    rootfitFunc3->SetParameter(10,0);
    rootfitFunc3->SetParameter(11,0);

    TFitResultPtr fit_result_3=spettro->Fit("rootfitFunc3", "SQR");
		Double_t Gamp4 =fit_result_3->Parameter(0);	
		Double_t Gmean4 =fit_result_3->Parameter(1);
		Double_t Gvar4 =fit_result_3->Parameter(2);	
		Double_t chi4 =fit_result_3->Chi2();		
		Double_t Ndf4=fit_result_3->Ndf();														
		out_dat<<"G+FD+FD+BG\t"<< Gamp4<<"\t"<<Gmean4<<"\t"<<Gvar4<<"\t"<<"chisq:"<<chi4<<"\t"<<Ndf4<<std::endl;
	  //out_dat<<"**"<<std::endl;

    /*//*****************III fit gauss+FD+FD+BGexp(lin-IIorder)**********************************************************
    TF1* rootfitFunc4= new TF1("rootfitFunc4", fitFunc3, FIT_START, NMAX, 11);
      
    rootfitFunc4->SetParameter(0,Gamp1);
    rootfitFunc4->SetParameter(1,Gmean1);
    rootfitFunc4->SetParameter(2,Gvar1);
		rootfitFunc4->SetParameter(3,300);
    rootfitFunc4->SetParameter(4,300);
    rootfitFunc4->SetParameter(5,0.01);
    rootfitFunc4->SetParameter(6,200);
    rootfitFunc4->SetParameter(7,200);
    rootfitFunc4->SetParameter(8,0.03);
		rootfitFunc4->SetParameter(9,0.1);
    rootfitFunc4->SetParameter(10,0.1);

    TFitResultPtr fit_result_4=spettro->Fit("rootfitFunc4", "SQR");
		Double_t Gamp5 =fit_result_4->Parameter(0);	
		Double_t Gmean5 =fit_result_4->Parameter(1);
		Double_t Gvar5 =fit_result_4->Parameter(2);	
		Double_t chi5 =fit_result_4->Chi2();		
		Double_t Ndf5=fit_result_4->Ndf();														
		out_dat<<"G+FD+FD+BGexp\t"<< Gamp5<<"\t"<<Gmean5<<"\t"<<Gvar5<<"\t"<<"chisq:"<<chi5<<"\t"<<Ndf5<<std::endl;
	  out_dat<<"**"<<std::endl;*/



     




		medie[channel] = fit_result_1->Parameter(1);
    errore_medie[channel] = fit_result_1->ParError(1);
    errore += errore_medie[channel]*errore_medie[channel];
        

    kap[channel]= MEDIA/medie[channel];
    k_media += kap[channel];

      // TH1F* spettro_calib = new TH1F("spettro_calib",Form("Spettro Cesio calibrato [Ch: %d]", channel), NBIN, NMIN*kap[channel], NMAX*kap[channel]);
      
      // for (int entry=0; entry<nEntries ; entry++) {
      //   tree->GetEntry(entry);
      //   if(-vcharge[channel]>CMIN){
      //     spettro_calib -> Fill(-vcharge[channel]*kap[channel]);
      //   }
      // }
        //TF1 *fitgaus2 = new TF1("fitgaus2", "gaus", kap[channel]*xmin[channel], NMAX);  //fit gaussiana 2
      //TFitResultPtr fit_result_2 = spettro_calib->Fit("fitgaus2", "SRQ");
      
    //Stampiamo  tutto
    TCanvas* plot_spettro = new TCanvas("spettro",Form("Spettro Cesio [Ch: %d]", channel),1920,1080);
    spettro->SetTitle(Form("Spettro Cesio [Ch: %d];Energia (MeV);Numero Eventi",channel));
    plot_spettro -> cd(); // Apre una sessione
    spettro -> Draw(); // Disegna l'istogramma
    plot_spettro -> SaveAs(Form("%s/Ist_Spettro_Cs_%d.pdf", plotsDir1.c_str(),channel));
      
      
    // TCanvas* plot_spettro_calib = new TCanvas("spettro_calib",Form("Spettro Cesio scalato [Ch: %d]", channel),1920,1080); 
    // spettro_calib->SetTitle(Form("Spettro Cesio Scalato [Ch: %d];Energia (MeV);Numero Eventi",channel));
    // plot_spettro_calib -> cd(); // Apre una sessione
    // spettro_calib -> Draw(); // Disegna l'istogramma
     // plot_spettro_calib -> SaveAs(Form("%s/Ist_Spettro_Cs_%d_Calib.pdf", plotsDir2.c_str(),channel));
      
      
      
    delete spettro;
    //delete spettro_calib;
    //delete plot_spettro_calib;
    delete plot_spettro;
    delete rootfitFunc;
    //delete fitgaus1;
    //delete fitgaus2;
  } 

  // fine for sui canali

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
    //std::cout<<err_ki[channel] <<" "<<k_media<<std::endl;

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




	//chiudi file dei dati
	out_dat.close();
    
    
  return 0;

}