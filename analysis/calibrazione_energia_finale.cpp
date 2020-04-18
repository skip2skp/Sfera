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



//QUA DOBBIAMO 4 DUE ARRAY DI VMAX DAL PROGRAMMA DEL FIT (VMAX E INCERTEZZA PER DATI PARI E DISPARI) E DUE CANALI PER LA SOVRAPPOSIZIONE//*
int main() {

	TString rootFileName(argv[1]);
	TFile* rootFile = new TFile(rootFileName);
	std::cout<<"Reading data from root file "<<argv[1]<<std::endl;

	TTree* tree = (TTree*) rootFile->Get("tree");
	if(!tree) {
		std::cout<<"Error, no tree called tree in "<<argv[1]<<". Exiting."<<std::endl;
		exit(ERROR_NOTREE);
	}

	int ev;
	float base[NCH], vcharge[NCH];

	tree->SetBranchAddress("ev", &ev);
	tree->SetBranchAddress("base", &base);
	tree->SetBranchAddress("vcharge", &vcharge);

	int nEntries = tree->GetEntries();
	int flag_fit = 1;

	std::string plotsDir(Form("Istogramma_spettro_Cesio/"));
	system( Form("mkdir -p %s", plotsDir.c_str()) );


	//QUESTI DUE VANNO PASSATI DAL PROGRAMMA DEL FIT--PARI
	double picco_FE_p[NCH];
	double inc_picco_FE_p[NCH];
	
	//QUESTI DUE VANNO PASSATI DAL PROGRAMMA DEL FIT--DISPARI
	double picco_FE_d[NCH];
	double inc_picco_FE_d[NCH];

	double k_cal[NCH] = {0.};	//k sarà il rapporto tra la MEDIA  e il picco FE per ogni canale QUA PER CALIBRARE
	double inc_k_cal[NCH] = {0.};
	double k_medio_cal = 0;
	double inc_k_medio_cal = 0;
	double picco_test[NCH] = {0.};	//k sarà il rapporto tra la MEDIA  e il picco FE per ogni canale QUA PER TESTARE
	double inc_picco_test[NCH] = {0.};
	double k_medio_test = 0;
	double inc_k_medio_test = 0;
	double k_test[NCH] = {0.};	//k sarà il rapporto tra la MEDIA  e il picco FE per ogni canale QUA PER TESTARE
	double inc_k_test[NCH] = {0.};

	for(int channel=0; channel<NCH; channel++){

		k_cal[channel] = MEDIA/picco_FE_p[channel];
		inc_k_cal[channel] = ((k_cal[channel]/picco_FE_p[channel])^2)*(inc_picco_FE_p[channel]^2); //ERRORE QUADRATO PROPAGATO

		k_medio_cal += k_cal[channel];
		inc_k_medio_cal += inc_k_cal[channel];

		picco_test[channel] = picco_FE_d[channel]*k_cal[channel];
		inc_picco_test[channel] = ((1./k_cal[channel])^2)*(inc_picco_FE_d[channel])^2 + ((picco_test[channel]/k_cal[channel])^2)*(inc_k_cal[channel])^2;

		k_test[channel] = MEDIA/picco_test[channel];
		inc_k_test[channel] = ((k_test[channel]/picco_test[channel])^2)*(inc_picco_test[channel]^2); //ERRORE QUADRATO PROPAGATO

		k_medio_test += k_test[channel];
		inc_k_medio_test += inc_k_test[channel];

	}

	k_medio_cal = k_medio_cal/NCH;
	inc_k_medio_cal = sqrt(inc_k_medio_cal)/NCH;


	k_medio_test = k_medio_test/NCH;
	inc_k_medio_test = sqrt(inc_k_medio_test)/NCH;



  	Double_t channel_axis[NCH];

  	Double_t k_cal_normalizzato[NCH];
	Double_t inc_k_cal_normalizzato[NCH];

	Double_t k_test_normalizzato[NCH];
	Double_t inc_k_test_normalizzato[NCH];

	Double_t *err_channel=0; // Dummy
  	

  	for(int channel=0; channel<NCH; channel++){

    	channel_axis[channel]=channel+1; 

    	k_cal_normalizzato[channel]=k_cal[channel]/k_medio_cal;
    	inc_k_cal_normalizzato[channel]=inc_k_cal[channel]/(k_medio_cal^2) + (k_cal_normalizzato[channel]^2)/(k_medio_cal^2)*(inc_k_medio_cal^2);
    	inc_k_cal_normalizzato[channel] = sqrt(inc_k_cal_normalizzato[channel]);
    	
    	k_test_normalizzato[channel]=k_test[channel]/k_medio_test;
    	inc_k_test_normalizzato[channel]=inc_k_test[channel]/(k_medio_test^2) + (k_test_normalizzato[channel]^2)/(k_medio_test^2)*(inc_k_medio_test^2);
    	inc_k_test_normalizzato[channel] = sqrt(inc_k_test_normalizzato[channel]);  	
  	}

  	TCanvas* c2 = new TCanvas("c2", "Grafico Calibrazione Energie");
  	c2->cd();
  	TMultiGraph *mg = new TMultiGraph();
  	TGraphErrors* gr=new TGraphErrors(NCH, channel_axis, k_test_normalizzato, err_channel, inc_k_test_normalizzato);
  	gr->GetXaxis()->SetTitle("Canale");
  	gr->GetYaxis()->SetTitle("k Normalizzato");
  	gr->SetMarkerStyle(21);
  	gr->SetMarkerSize(1.0);
  	TGraphErrors* gr1=new TGraphErrors(NCH, channel_axis, k_cal_normalizzato, err_channel, inc_k_cal_normalizzato);
  	gr1->SetMarkerColorAlpha(kRed, 1);
  	gr1->SetMarkerStyle(20);

  	mg->Add(gr1);
  	mg->Add(gr);
  	mg->Draw("AP");
  	
  	c2->SaveAs(Form("%s/calibrazione_energie.pdf", plotsDir.c_str()));





	//A E B DEVONO ESSERE I DUE CANALI CHE GLI PASSIAMO

	TH1F* hist1 = new TH1F("hist1",Form("Spettro Cesio [Ch: %d]",Variabile A), NBIN, NMIN, NMAX);
	TH1F* hist2 = new TH1F("hist2",Form("Spettro Cesio [Ch: %d]",Variabile B), NBIN, NMIN, NMAX);


	TH1F* hist1_scaled = new TH1F("hist1_scaled",Form("Spettro Cesio scalato [Ch: %d]",Variabile A), NBIN, k_cal[A]*NMIN, k_cal[A]*NMAX);
	TH1F* hist2_scaled = new TH1F("hist2_scaled",Form("Spettro Cesio scalato [Ch: %d]",Variabile B), NBIN, k_cal[B]*NMIN, k_cal[B]*NMAX);	

	for (int entry=0; entry<nEntries/2; entry++) {
		tree->GetEntry(entry*2+1);
		if(-vcharge[A]>CMIN){
			hist1 -> Fill(-vcharge[A]);
			hist1_scaled -> Fill(-vcharge[A]*k_cal[A]);
		}
	}

	for (int entry=0; entry<nEntries/2 ; entry++) {
		tree->GetEntry(entr*2+1);
		if(-vcharge[A]>CMIN){
			hist2 -> Fill(-vcharge[B]);
			hist2_scaled -> Fill(-vcharge[B]*k_cal[B]);
		}
	}

    TCanvas* H = new TCanvas("hist1","Sovrapposizione Istogrammi non Scalati"); // Nome, Titolo,x,y
    hist1->SetTitle(";Carica (pC);Numero Eventi");
    hist1->GetYaxis()->SetRangeUser(0,2500);
    H -> cd(); // Apre una sessione
    hist1 -> Draw();
    hist2 -> Draw("SAME");
    H -> Update();
    H -> SaveAs(Form("%s/Istogrammi_ch%d_ch%d_sovrapposti.pdf", plotsDir.c_str(), A, B));

    delete hist1;
    delete hist2;

    TCanvas* I = new TCanvas("hist1_scaled","Sovrapposizione Istogrammi Scalati"); // Nome, Titolo,x,y
    hist1_scaled->SetTitle(";Energia (MeV);Numero Eventi");
    hist1_scaled->GetYaxis()->SetRangeUser(0,2500);
    I -> cd(); // Apre una sessione
    hist1_scaled -> Draw();
    hist2_scaled -> Draw("SAME");
    I -> Update();
    I -> SaveAs(Form("%s/Istogrammi_ch%d_ch%d_scalati_sovrapposti.pdf", plotsDir.c_str(), A, B));


    delete hist1_scaled;
    delete hist2_scaled;

    return 0;
}