// Programma per integrare il profilo di impulso e confrontarlo con risultato ritornato dal digitizer.

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define DT 938E-3 // ns
#define BMAX 20
#define vmin 50


#include<iostream>
#include<stdio.h>
//#include<vector>

#include"TFile.h"
#include"TTree.h"
#include"TString.h"
#include"TH1F.h"
#include"TF1.h"
#include"TCanvas.h"
#include"TGraphErrors.h"
#include"TLatex.h"
#include"TAttLine.h"
#include"TMultiGraph.h"


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


	//------------------------------------------------- PARTE media sui singoli canali -----------------------------------------

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

	std::string plotsDir(Form("plots_checkCharge_sodio/"));
	system( Form("mkdir -p %s", plotsDir.c_str()) );

  // per grafico calibrazione

	Double_t x[NCH]={0.};
	Double_t charges[NCH]={0.};
	Double_t err_charges[NCH]={0.};
	Double_t *err_x=0;
	Double_t entries[NCH]={0.};

	TH1F* diffbaseline = new TH1F("diffbaseline", "correzione all'integrale per calcolo baseline ", 100, -0.1, 0.1);
	TH1F* min = new TH1F("min", " ", 50, -100, 10);


	for (int channel=0; channel<NCH; channel++) {

		TH1F* others = new TH1F("others", "", 100, 0.048, 0.052);
		TH1F* hist = new TH1F("hist", " ", 100, 0.048, 0.052);

		for (int j=0; j<NCH; j++) {
			if(j==channel){
				
				double basenostra =0; 
				for (int entry=0; entry<nEntries ; entry++) {
					tree->GetEntry(entry);
					float sum=0;

					min->Fill(vcharge[j]); //to assess if there is an physical event


					for(int i=0; i<BMAX; i++){
						basenostra+=pshape[j][i];
					}

					basenostra/=BMAX;
        //diffbaseline->Fill(basenostra-base[channel]);

					for (int i=BMAX; i<1024; i++) {
						sum+=pshape[j][i]-base[j];
					}

					rapporto = sum*DT/vcharge[j];

					if(vcharge[j] < -vmin) {
						if(j==channel){
							hist->Fill(rapporto);
							diffbaseline->Fill((basenostra-base[j])*(1024)/(sum*DT));      
						}
						else {
							others->Fill(rapporto);
						}

					}

				}
			}
		}

    //    	Double_t norm = hist->GetEntries();  //QUA ANDREBBE GETMAXIMUM
    //    	hist->Scale(1./norm);
    //    	norm = others->GetEntries();
    //    	others->Scale(1./norm);

    // 	TCanvas* c1 = new TCanvas("c1",Form("Istogramma rapporti con carica canale %d vs altri", channel),600,800); // Nome, Titolo,x,y
  		// c1->cd(); // Apre una sessione
  		// hist->SetLineColor(2);
  		// others->SetLineColor(4);
  		// hist->SetXTitle("I/C");
  		// hist->SetYTitle("N eventi");
  		// hist->Draw("HIST"); // Disegna l'istogramma
  		// others->Draw("SAME HIST");
  		// c1->Update();
  		// c1->SaveAs(Form("%s/hist_charge_%d.pdf", plotsDir.c_str(),channel));



		x[channel]=channel+1;
		charges[channel]=hist->GetMean();
		err_charges[channel]=hist->GetStdDev();
		entries[channel] = hist->GetEntries();

		delete hist;
		delete others;
  		// delete c1;

	}
	
	TCanvas* ao1 = new TCanvas("c1","Istogramma Cariche ",600,800); // Nome, Titolo,x,y
 	ao1->cd(); // Apre una sessione
 	ao1->SetLogy();
 	min->SetXTitle("charge");
 	min->SetYTitle("N eventi");
  	min->Draw(); // Disegna l'istogramma
  	ao1->SaveAs(Form("%s/hist_charge.pdf", plotsDir.c_str()));




	// TCanvas* c3 = new TCanvas("c3","diff delle baseline",600,800); // Nome, Titolo,x,y
 //  	c3->cd(); // Apre una sessione
 //  	diffbaseline->Draw(); // Disegna l'istogramma
 //  	c3->SaveAs(Form("%s/diffbaseline.pdf", plotsDir.c_str()));


//------------------------------------- PARTE su tutti i canali unico istogramma ---------------------------




	TH1F* totale = new TH1F("totale", " ", 100, 0.043, 0.057);
	for (int channel=0; channel<NCH; channel++) {

		for (int entry=0; entry<nEntries ; entry++) {

			tree->GetEntry(entry);
			float sum=0;

			for (int i=0; i<1024; i++) {
				sum+=pshape[channel][i]-base[channel];
			}

			rapporto = sum*=DT/vcharge[channel];

			if(vcharge[channel] < -vmin) totale->Fill(rapporto);    
		}
	}

	double s = totale->GetStdDev();
	std::cout<<s<<std::endl;


	TCanvas* ao = new TCanvas("c1","Istogramma rapporti con carica",600,800); // Nome, Titolo,x,y
 	ao->cd(); // Apre una sessione
 	ao->SetLogy();
 	totale->SetXTitle("I/C");
 	totale->SetYTitle("N eventi");
  	totale->Draw(); // Disegna l'istogramma
  	ao->SaveAs(Form("%s/hist_charge_totale.pdf", plotsDir.c_str()));


  	delete ao;


//------------------------------------------------- PARTE media sui singoli canali -----------------------------------------


  	Double_t sigma[NCH]={0.};
  	Double_t medie[NCH]={0.};

  	double M = totale->GetMean();

  	for(int i =0; i<NCH; i++){
  		if(entries[i]!=0){
  			sigma[i]= 2*s/sqrt(entries[i]);
  		}
  		else charges[i]=M;
  		medie[i] = M;
  	}



  	TCanvas* c2 = new TCanvas("c2", "Grafico Calibrazione");
  	c2->cd();
  	TMultiGraph *mg = new TMultiGraph();
  	TGraphErrors* gr=new TGraphErrors(NCH, x, medie, err_x, sigma);
  	gr->GetXaxis()->SetTitle("Canale");
  	gr->GetYaxis()->SetTitle("I/C Medio");
  	gr->SetMarkerStyle(21);
  	gr->SetMarkerSize(1.0);
  	TGraphErrors* gr1=new TGraphErrors(NCH, x, charges, err_x, err_x);
  	gr1->SetMarkerColorAlpha(kRed, 1);
  	gr1->SetMarkerStyle(20);

  	mg->Add(gr1);
  	mg->Add(gr);
  	mg->Draw("AP");


  // 	TLatex l;
  // 	l.SetTextSize(0.025);
  // 	l.SetTextAngle(30.);
  // 	l.DrawLatex(13,0.074,Form("R_avg = %f",sum));
  // 	l.DrawLatex(13,0.072,Form("R_avg_err = %f",err));
  	c2->SaveAs(Form("%s/calibrazione.pdf", plotsDir.c_str()));

  	delete gr;
  	delete gr1;
  	delete c2;
  	delete totale;



  }

