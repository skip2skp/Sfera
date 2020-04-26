
#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define DT 938E-3 // ns
#define BMAX 20
#define cmin -1000
#define cmax -50


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

#include<iostream>
#include<stdio.h>
#include<vector>


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

	int ev;
	float base[NCH], charge[NCH], pshape[NCH][1024];

	tree->SetBranchAddress("ev", &ev);
	tree->SetBranchAddress("base", &base);
	tree->SetBranchAddress("vcharge", &charge);
	tree->SetBranchAddress("pshape", &pshape);

	int nEntries = tree->GetEntries();


	// Crea directory per i plot
	
	std::string plotsDir(Form("plots_checkCharge_sodio/"));
	system( Form("mkdir -p %s", plotsDir.c_str()) );



	std::cout<<"Scegliere tra i seguenti programmi:"<<std::endl;
	std::cout<<"1. Check baseline"<<std::endl;
	std::cout<<"2. Sovrapposizione istogramma dei rapporti su un singolo canale con istogramma rapporti su tutti gli altri"<<std::endl;
	std::cout<<"3. Studio della media dei rapporti su tutti i canali"<<std::endl;
	std::cout<<"4. Istogramma di tutte le cariche"<<std::endl;
	std::cout<<"5. Istogramma dei rapporti totale (somma su tutti i canali)"<<std::endl;


	int a=0;
	std::cin >> a;

	if(a!=1 && a!=2 && a!=3 && a!=4 && a!=5){
		std::cout<<"L'opzione selezionata non è valida, scegliere nuovamente tra le opzioni sopra."<<std::endl;
		std::cin >> a;
	}

	//	PROGRAMMA NUMERO UNO




	//	PROGRAMMA NUMERO DUE

	if (a==2){

		for (int channel=0; channel<NCH; channel++) {

			TH1F* others = new TH1F("others", "", 100, 0.048, 0.052);
			TH1F* hist = new TH1F("hist", " ", 100, 0.048, 0.052);
			double rapporto;

			for (int j=0; j<NCH; j++) {

				for (int entry=0; entry<nEntries ; entry++) {
					tree->GetEntry(entry);
					float sum=0;

					for (int i=BMAX; i<1024; i++) {
						sum+=pshape[j][i]-base[j];
					}

					rapporto = sum*DT/charge[j];

					if(charge[j] < cmin && charge[j]<cmax) {
						if(j==channel){
							hist->Fill(rapporto);
						}
						else {
							others->Fill(rapporto);
						}
					}
				}
			}

			Double_t norm = hist->GetEntries();  
			hist->Scale(1./norm);
			norm = others->GetEntries();
			others->Scale(1./norm);

    		TCanvas* c1 = new TCanvas("c1",Form("Istogramma rapporti con carica canale %d vs altri", channel),600,800); // Nome, Titolo,x,y
  			c1->cd(); // Apre una sessione
  			hist->SetLineColor(2);
  			others->SetLineColor(4);
  			hist->SetXTitle("I/C");
  			hist->SetYTitle("N eventi");
  			hist->Draw("HIST"); // Disegna l'istogramma
  			others->Draw("SAME HIST");
  			c1->Update();
  			c1->SaveAs(Form("%s/overlap_hist_ch%d.pdf", plotsDir.c_str(),channel));

  			delete hist;
  			delete others;
  			delete c1;
  		}
  	}

	//	PROGRAMMA NUMERO TRE

  	if(a==3){
  		

  		std::vector<double> means;
  		std::vector<double> err_means;
  		std::vector<double> x;
  		Double_t *err_x=0;


  		for (int j=0; j<NCH; j++) {

			TH1F* hist = new TH1F("hist", " ", 100, 0.048, 0.052); // rapporto integrale/carica

			for (int entry=0; entry<nEntries ; entry++) {
				tree->GetEntry(entry);
				float sum=0;

				for (int i=BMAX; i<1024; i++) sum+=pshape[j][i]-base[j];
				if(charge[j]<cmax && charge[j]>cmin) hist->Fill(sum*DT/charge[j]); 

			}

			if(hist->GetEntries()>0) {
				means.push_back(hist->GetMean());
				err_means.push_back(hist->GetMeanError());
				x.push_back(j+1);
			}

			delete hist;
		
		}

		TCanvas* c2 = new TCanvas("c2", "Grafico Calibrazione");
		c2->cd();
  		TGraphErrors* gr=new TGraphErrors(x.size(), &x[0], &means[0], err_x, &err_means[0]); // plot integrale/carica vs canale
		TF1 *f = new TF1("f", "[0]"); // funzione costante
		gr->Fit(f); // fit sul grafico
		double_t chi2 = gr->Chisquare(f);
		gr->GetXaxis()->SetTitle("Canale");
		gr->GetYaxis()->SetTitle("I/C Medio");
		gr->SetMarkerStyle(21);
		gr->SetMarkerSize(1.0);
		gr->Draw("AP");
		c2->SaveAs(Form("%s/calibrazione.pdf", plotsDir.c_str()));

		std::cout<< "Chi2/Ndof = "<< chi2/f->GetNDF()<< std::endl;

		delete c2;

	}

	//	PROGRAMMA NUMERO QUATTRO

	if(a==4){

		TH1F* hist = new TH1F("hist", " ", 100, -10000, 10); // rapporto integrale/carica

		for (int j=0; j<NCH; j++) {
			for (int entry=0; entry<nEntries ; entry++) {
				tree->GetEntry(entry);
				hist->Fill(charge[j]);
			}
		}


    	TCanvas* c3 = new TCanvas("c3","Istogramma con tutte le cariche",600,800); // Nome, Titolo,x,y
  		c3->cd(); // Apre una sessione
  		c3->SetLogy();
  		hist->SetLineColor(2);
  		hist->SetXTitle("Carica (pC)");
  		hist->SetYTitle("N eventi");
  		hist->Draw("HIST"); // Disegna l'istogramma
  		c3->SaveAs(Form("%s/istogramma_totale_cariche.pdf", plotsDir.c_str()));

  		delete hist;
  		delete c3;


  	}


	//	PROGRAMMA NUMERO CINQUE

  	if(a==5){

  		TH1F* totale = new TH1F("totale", " ", 100, 0.043, 0.057);
  		double rapporto;

  		for (int channel=0; channel<NCH; channel++) {

  			for (int entry=0; entry<nEntries ; entry++) {

  				tree->GetEntry(entry);
  				float sum=0;

  				for (int i=0; i<1024; i++) {
  					sum+=pshape[channel][i]-base[channel];
  				}

  				rapporto = sum*=DT/charge[channel];

  				if(charge[channel]>cmin && charge[channel]<cmax) totale->Fill(rapporto);    
  			}
  		}

		TCanvas* c4 = new TCanvas("c4","Istogramma rapporti totale",600,800); // Nome, Titolo,x,y
 		c4->cd(); // Apre una sessione
 		c4->SetLogy();
 		totale->SetXTitle("I/C");
 		totale->SetYTitle("N eventi");
  		totale->Draw(); // Disegna l'istogramma
  		c4->SaveAs(Form("%s/hist_charge_totale.pdf", plotsDir.c_str()));
  		
  		delete c4;

  	}























}