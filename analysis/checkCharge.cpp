// Palmisano 29/03/2020
// Programma per integrare il profilo di impulso e confrontarlo con risultato ritornato dal digitizer.

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define DT 938E-3 // ns
#define BMAX 20

#include<iostream>
#include<stdio.h>
//#include<vector>

#include"TFile.h"
#include"TTree.h"
#include"TString.h"
#include"TH1F.h"
#include"TCanvas.h"
#include"TGraphErrors.h"
#include"TLatex.h"


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



  // variabili da leggere: numero evento, baseline, profilo, integrale calcolato dal digitizer

	int ev;
	float base[NCH], vcharge[NCH], pshape[NCH][1024];

	tree->SetBranchAddress("ev", &ev);
	tree->SetBranchAddress("base", &base);
	tree->SetBranchAddress("vcharge", &vcharge);
	tree->SetBranchAddress("pshape", &pshape);

	int nEntries = tree->GetEntries();
	float rapporto;
	float vmin = 50;

	std::string plotsDir(Form("Plot/"));
	system( Form("mkdir -p %s", plotsDir.c_str()) );

  // per grafico calibrazione

	Double_t x[NCH];
	Double_t charges[NCH];
	Double_t err_charges[NCH];
	Double_t *err_x=0;

	TH1F* diffbaseline = new TH1F("diffbaseline", "differenza baseline nostra e sua ", 100, -0.01, 0.01);


	for (int channel=0; channel<NCH; channel++) {

		TH1F* hist = new TH1F("hist", "distribuzione dei rapporti integrale/vcharge ", 100, -2, 2);
		double basenostra =0; 
		for (int entry=0; entry<nEntries ; entry++) {
			tree->GetEntry(entry);
			float sum=0;

			for(int i=0; i<BMAX; i++){
				basenostra+=pshape[channel][i];
			}
			
			basenostra/=BMAX;
			diffbaseline->Fill(base[channel]-basenostra);

			for (int i=BMAX; i<1024; i++) {
				sum+=pshape[channel][i]-basenostra;
			}

			rapporto = sum*=DT/vcharge[channel];

			if(vcharge[channel] < -vmin) hist->Fill(rapporto);

		}


	/*  TCanvas* c1 = new TCanvas("c1","Istogramma rapporti con carica",600,800); // Nome, Titolo,x,y
  	c1->cd(); // Apre una sessione
  	hist->Draw(); // Disegna l'istogramma
  	c1->SaveAs(Form("%s/hisd_charge_%d.pdf", plotsDir.c_str(),channel));*/

		x[channel]=channel;
		charges[channel]=hist->GetMean();
		err_charges[channel]=hist->GetStdDev();

		delete hist;
		// delete c1;

	}

	TCanvas* c3 = new TCanvas("c3","diff delle baseline stefanoèstupido",600,800); // Nome, Titolo,x,y
  	c3->cd(); // Apre una sessione
  	diffbaseline->Draw(); // Disegna l'istogramma
  	c3->SaveAs(Form("%s/diffbaseline.pdf", plotsDir.c_str()));


//------------------------------------------------- PARTE media sui singoli canali -----------------------------------------

  	double sum = 0;
  	double err = 0;
  	for (int channel=0; channel<NCH; channel++){
  		double temp = 0;
  		sum+=charges[channel];
  		temp = err_charges[channel]*err_charges[channel];
  		err+=temp;
  	}
  	sum/=NCH;
  	err = sqrt(err)/NCH;

  	TCanvas* c2 = new TCanvas("c2", "Grafico Calibrazione");
  	c2->cd();
  	TGraphErrors* gr=new TGraphErrors(NCH, x, charges, err_x, err_charges);
  	gr->SetTitle("Confronto medie e std_dev di R = Integrale/Vcharge");
  	gr->GetXaxis()->SetTitle("Numero del canale");
  	gr->GetYaxis()->SetTitle("Integrale/Vcharge");
  	gr->SetMarkerStyle(21);
  	gr->SetMarkerSize(1.0);
  	gr->Draw("AP");

  	TLatex l;
  	l.SetTextSize(0.025);
  //l.SetTextAngle(30.);
  	l.DrawLatex(13,0.074,Form("R_avg = %f",sum));
  	l.DrawLatex(13,0.072,Form("R_avg_err = %f",err));

  	c2->SaveAs(Form("%s/calibrazione.pdf", plotsDir.c_str()));

  	delete gr;
  	delete c2;


//------------------------------------- PARTE su tutti i canali unico istogramma ---------------------------


/*  TH1F* hist = new TH1F("hist", "distribuzione dei rapporti integrale/vcharge ", 100, -2, 2);
  for (int channel=0; channel<NCH; channel++) {

  	double basenostra =0; 
    
    for (int entry=0; entry<nEntries ; entry++) {
      
      tree->GetEntry(entry);
      float sum=0;

      for(int i=0; i<BMAX; i++){
      	basenostra+=pshape[channel][i];
      }

      basenostra/=BMAX;
      
      for (int i=BMAX; i<1024; i++) {
        sum+=pshape[channel][i]-basenostra;
      }

      rapporto = sum*=DT/vcharge[channel];

      if(vcharge[channel] < -vmin) hist->Fill(rapporto);    
    }
  }*/

/*	TCanvas* c1 = new TCanvas("c1","Istogramma rapporti con carica",600,800); // Nome, Titolo,x,y
 	c1->cd(); // Apre una sessione
 	c1->SetLogy();
  	hist->Draw(); // Disegna l'istogramma
  	c1->SaveAs(Form("%s/hisd_charge_totale.pdf", plotsDir.c_str()));

  	double R = hist->GetMean();
  	double R_err = hist->GetStdDev();

  	std::cout << "R_avg = " << R << std::endl;
  	std::cout << "R_err = " << R_err << std::endl;

  	delete hist;
  	delete c1;
*/


  }

