// Lorenzo & Andrea 
// versione 22/04/20
//Questo programma può:
// - controllare se c'è una dipendenza tra lo spostamento del picco del cesio all' aumentare delle funzioni aggiunte al fit
// fittare sia i padti "pari" e "quelli dispari separatamente "
//0 : stampa solo gaussiana
//1 : stampa G+FD ;risultati G e G+FD
//2 : stampa G+FD+FD ;risultati G G+FD G+FD+FD
//3 : stampa G+FD+FD+BKLIN ;risultati G G+FD G+FD+FD G+FD+FD++BKLIN

#define ERROR_USAGE 1
#define ERROR_NOTREE 2


#define NCH 16
#define CMIN 50
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

Double_t retta(Double_t *x, Double_t *par) {

	return par[0]+x[0]*par[1];

}

Double_t back(Double_t *x,Double_t *par){

return par[0]*exp(par[1]*(par[2]-x[0]));

}

Double_t background(Double_t *x, Double_t *par) {

  return par[0] + x[0]*par[1] + x[0]*x[0]*par[2];// + x[0]*x[0]*x[0]*par[3];

  
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

Double_t fitFunc3(Double_t *x, Double_t *par) { 
 
 return fermiDirac(x, &par[3]) + fermiDirac(x, &par[6]) + par[0]*TMath::Gaus(x[0], par[1], par[2])+back(x,&par[9]);
 
}


int main(int argc, char* argv[]) {
int scelta=atoi(argv[2]);
int scelta1=atoi(argv[3]);
//int scelta2=atoi(arv[4]);
  //richieste in terminale
  if (argc!=4 && scelta<4) {
    std::cout<<"Usage: "<<argv[0]<<" filename.root.\t type fit  \n Exiting."<<std::endl;
		std::cout<<"1 G \t2 G+FD \t3 G+FD+FD\t4 G+FD+FD+BKLIN "<<std::endl;
    exit(ERROR_USAGE);
  }

double split;
if(scelta1==0) split=1;
if(scelta1==1) split=0.5;


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
  std::string plotsDir(Form("spettro_Cesio/"));
  system( Form("mkdir -p %s", plotsDir.c_str()) );

  //std::string plotsDir1(Form("spettro_Cesio/spettro/"));
  //std::string plotsDir2(Form("spettro_Cesio/spettro_calib/"));

  //system( Form("mkdir -p %s", plotsDir1.c_str()) );
  //system( Form("mkdir -p %s", plotsDir2.c_str()) );
  //genero file per risultati dei fit
	std::ofstream out_dat;
  std::ofstream out_even;
	std::ofstream out_odd;  
	out_dat.open("spettro_Cesio/risultati.txt");
  if(split!=1) {
	  out_odd.open("spettro_Cesio/odd.txt");  
	  out_even.open("spettro_Cesio/even.txt");  
  }

  

  double xmin[NCH] = {950, 650, 650, 750, 850, 800, 700, 750, 700, 700, 900, 950, 700, 800, 770, 750};


	//		double param[12];
			double_t PS[12];

/***********************************BEGIN FIT AND K CALC*********************************************************************************/

  for (int channel=0; channel<NCH; channel++) {  
    TH1F* spettro = new TH1F("spettro",Form("Spettro Cesio [Ch: %d]", channel), NBIN, NMIN, NMAX);
		
		
		  int parity=0;

		while(parity<2){      
		
			for (int entry=0; entry<nEntries*split ; entry++) {
        tree->GetEntry(entry/split + parity); 	// prendi l'evento i-esimo 
        if(-vcharge[channel]>CMIN){spettro -> Fill(-vcharge[channel]);} //riempi l'isto 
      }


		
if(scelta<=4){
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

		
		
if(scelta>=1){
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
		out_dat<<"PK2/PK1="<<Gmean2/Gmean1<<std::endl;
	     }
if(scelta>=2){
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
	    out_dat<<"PK3/PK1="<<Gmean3/Gmean1<<std::endl;

			}
if(scelta>=3){
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
		Double_t Gmean4err=fit_result_3->ParError(1);														
		out_dat<<"G+FD+FD+BG\t"<< Gamp4<<"\t"<<Gmean4<<"\t"<<Gvar4<<"\t"<<"chisq:"<<chi4<<"\t"<<Ndf4<<std::endl;
		 out_dat<<"PK4/PK1="<<Gmean4/Gmean1<<std::endl;
		rootfitFunc3->GetParameters(PS);
				
			//	param[0]=Gamp4;
				//param[1]=Gmean4;
				//param[2]=Gvar4;
		if(split!=1) {
      float a;
      a = (Gmean4-Gmean1)/2;
      if(parity==0) out_even<<Gmean4<<"  "<<Gmean4err<<"  "<<a<<"  "<<sqrt((Gmean4err*Gmean4err) + a*a)<<std::endl;
		  else        	out_odd<<Gmean4<<"  "<<Gmean4err<<"  "<<a<<"  "<<sqrt((Gmean4err*Gmean4err) + a*a)<<std::endl;
    }


	  //out_dat<<"**"<<std::endl;

		}
		}
			//if(scelta=4){
    //*****************III fit gauss+FD+FD+BGexp**********************************************************
   /*TF1* rootfitFunc4= new TF1("rootfitFunc4", back, FIT_START, NMAX,3);
      
    rootfitFunc4->SetParameter(0,1);
    rootfitFunc4->SetParameter(1,1);
    rootfitFunc4->SetParameter(2,800);
		rootfitFunc4->SetParameter(3,300);
    rootfitFunc4->SetParameter(4,300);
    rootfitFunc4->SetParameter(5,0.01);
    rootfitFunc4->SetParameter(6,200);
    rootfitFunc4->SetParameter(7,200);
    rootfitFunc4->SetParameter(8,0.03);
		rootfitFunc4->SetParameter(9,1);
    rootfitFunc4->SetParameter(10,1);
     rootfitFunc4->SetParameter(11,800);

    TFitResultPtr fit_result_4=spettro->Fit("rootfitFunc4","SQR");
		Double_t Gamp5 =fit_result_4->Parameter(0);	
		Double_t Gmean5 =fit_result_4->Parameter(1);
		Double_t Gvar5 =fit_result_4->Parameter(2);	
		Double_t chi5 =fit_result_4->Chi2();		
		Double_t Ndf5=fit_result_4->Ndf();														
		out_dat<<"G+FD+FD+BGexp\t"<< Gamp5<<"\t"<<Gmean5<<"\t"<<Gvar5<<"\t"<<"chisq:"<<chi5<<"\t"<<Ndf5<<std::endl;
}
	*/


		
 out_dat<<"-------------------------------------------------------"<<std::endl;
  

	TF1* fgaus = new TF1("fgaus","gaus",FIT_START, NMAX);
	TF1* fback = new TF1("fgaus",background,FIT_START, NMAX,3);
	TF1* fd1 = new TF1("fgaus",fermiDirac,FIT_START, NMAX,3);
	TF1* fd2 = new TF1("fgaus",fermiDirac,FIT_START, NMAX,3);	
	fgaus->SetLineColor(kBlack);
	fback->SetLineColor(kBlue);
	fd1->SetLineColor(kGreen);
	fd2->SetLineColor(kOrange);
  fgaus->SetParameter(0,PS[0]);
  fgaus->SetParameter(1,PS[1]);
  fgaus->SetParameter(2,PS[2]);
	fd1->SetParameter(0,PS[3]);
	fd1->SetParameter(1,PS[4]);
	fd1->SetParameter(2,PS[5]);
	fd2->SetParameter(0,PS[6]);
	fd2->SetParameter(1,PS[7]);
	fd2->SetParameter(2,PS[8]);
	fback->SetParameter(0,PS[9]);
	fback->SetParameter(1,PS[10]);
	fback->SetParameter(2,PS[11]);
	

      
  //Stampiamo  tutto
   TCanvas* plot_spettro = new TCanvas("spettro",Form("Spettro Cesio [Ch: %d]", channel),1920,1080);
	 spettro->SetTitle(Form("Spettro Cesio [Ch: %d];Energia (MeV);Numero Eventi",channel));
	 plot_spettro -> cd(); // Apre una sessione
   spettro -> Draw(); // Disegna l'istogramma
	
    fgaus->Draw("same");
		fd1->Draw("same");
	  fd2->Draw("same");
    fback->Draw("same");
		plot_spettro -> SaveAs(Form("%s/Ist_Spettro_Cs_%d.pdf", plotsDir.c_str(),channel));
      
      
    
          delete plot_spettro;
      

	if(split!=1) parity++;
  else parity = 2;
 }//while
      
    delete spettro;
    

  } 

  // fine for sui canali


	//chiudi file dei dati
	out_dat.close();
  if(split!=1) {
    out_odd.close();
    out_even.close();
  } 
  return 0;

}
