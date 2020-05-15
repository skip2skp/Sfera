
#define ERROR_USAGE 1
#define ERROR_NOTREE 2
#define CMIN 50
#define CMAX 4000
#define NBIN 300

#define NCH 16
#define MODE_PEAKS 1
#define MODE_LIN 2

#include"TFile.h"
#include"TTree.h"
#include"TString.h"
#include"TH1F.h"
#include"TF1.h"
#include"TFitResultPtr.h"
#include"TFitResult.h"
#include"TCanvas.h"
#include"TGraphErrors.h"
#include"TLatex.h"
#include"TAttLine.h"
#include"TMultiGraph.h"

#include<iostream>
#include<stdio.h>
#include<vector>
#include<fstream>



int main(int argc, char* argv[]) {

  //int NSOR;
        int delimiter=atoi(":"); 
	/*std::cout<<"Quante sorgenti vuoi studiare?"<<std::endl;
	  std::cin >> NSOR;*/ // not necessary with this method i am implementing SP 15/05/2020
	
	int mode;
	int draw_hist;
	int analyze_linearity; //changed these names to something more understandable and defined precompiler variables with the modes in case we ever need to change and/or add new modes SP 15/05/2020
	std::cout<<"Scegliere tra i seguenti studi:"<<std::endl;
	std::cout<<MODE_PEAKS<<". Studio picchi (se già conosci i range)"<<std::endl;
	std::cout<<MODE_LIN<<". Studio linearità (se già conosci i picchi & errori)"<<std::endl;
	std::cin >> mode;
	
	while(mode!=MODE_PEAKS && mode!=MODE_LIN){ //changed if in while or it would only check this once SP 15/05/2020
		std::cout<<"L'opzione selezionata non è valida, scegliere nuovamente tra le opzioni sopra."<<std::endl;
		std::cin >> mode;
	}

	if(mode==MODE_PEAKS){

		int ev, entries;
		float base[NCH], charge[NCH];
		int npicchi;
		double_t media;
		double_t errore;

		std::ofstream out_dat;
		out_dat.open("picchi_FE.txt");

		std::ifstream range ("range.txt", std::ifstream::in);		

		std::cout<<"Premere 1 se si vuole stampare gli istogrammi, 0 altrimenti"<<std::endl;
		std::cin>> draw_hist;

		while(draw_hist!=1 && draw_hist!=0){
			std::cout<<"L'opzione selezionata non è valida, scegliere nuovamente tra le opzioni sopra."<<std::endl;
			std::cin >> draw_hist;
		}

		while(!range.eof()) {

		        TString rootFileName;
			range.ignore(200, delimiter);
			range>>rootFileName;
			TFile* rootFile = new TFile(rootFileName);
			std::cout<<"Reading data from root file "<<rootFileName<<std::endl;

			TTree* tree = (TTree*) rootFile->Get("tree");
			if(!tree) {
				std::cout<<"Error, no tree called tree in "<<rootFileName<<". Exiting."<<std::endl;
				exit(ERROR_NOTREE);
			}

			tree->SetBranchAddress("ev", &ev);
			tree->SetBranchAddress("base", &base);
			tree->SetBranchAddress("vcharge", &charge);
			entries = tree->GetEntries();

			std::string sourceName;
			range.ignore(200, delimiter);
			range>>sourceName;
 			
 			range.ignore(200, delimiter);
			range >> npicchi;

			for(int k=0; k<npicchi;k++){

				for (int j=0; j<NCH; j++) {

					TH1F* hist = new TH1F("hist", " ", NBIN, CMIN, CMAX);

					for (int entry=0; entry<entries; entry++) {
						tree->GetEntry(entry);
						hist->Fill(-charge[j]);
					}

					// LEVA COMMENT
					
					double min, max;
 					range.ignore(200, delimiter);
 					range>>min>>max;

					if(draw_hist==1){
						std::string sorgente = "source"+sourceName;
						std::string plotsDir(Form("plot_lin/%s/", sorgente.c_str()));
						system( Form("mkdir -p %s", plotsDir.c_str()));

						TCanvas* c = new TCanvas("c3","Istogramma con tutte le cariche",2000,800); // Nome, Titolo,x,y
  						c->cd(); // Apre una sessione
  						hist->SetLineColor(2);
  						hist->SetXTitle("Carica (pC)");
  						hist->SetYTitle("N eventi");
  						hist->SetStats(0);
 						hist->Draw(); // Disegna l'istogramma
 						c->SaveAs(Form("%s/istogramma_cariche_sor_%s_canale_%d.pdf", plotsDir.c_str(),sourceName.c_str(),j+1));

 						delete c;
 					}

 					if (min != 0){
 						TF1 *fitgaus = new TF1("fitgaus", "gaus", min, max); 

   						TFitResultPtr gaussian_fit = hist->Fit("fitgaus", "SRQ");  //“Q” Quiet mode  “S” result in TFitResultPtr ""R” Use the range 
   						media = gaussian_fit->Parameter(1);
   						errore = gaussian_fit->ParError(1);					
   						out_dat<< media<<" "<<errore<<std::endl;
   						delete fitgaus;
   					}
   					else std::cout<<"Nessun picco"<<std::endl;

   					std::cout<<"ev("<<sourceName<<") "<<"ch("<<j<<") "<< "min "<< min<<"max "<<max <<"media "<<media<<std::endl; 

   					delete hist;
   				}
   			}
   		} //end while

   		out_dat.close();
   		std::cout<<"Vuoi proseguire con lo studio della linearità? Premere 1 se si, 0 altrimenti:"<<std::endl;
   		std::cin>>analyze_linearity;
   		while(analyze_linearity!=1 && analyze_linearity!=0){ //changed if in while, otherwise it would only check once SP 15/05/2020
   			std::cout<<"L'opzione selezionata non è valida, scegliere nuovamente tra le opzioni sopra."<<std::endl;
   			std::cin >> analyze_linearity;
   		}
   	}

   	if(mode==MODE_LIN || analyze_linearity==1){

   		int N;
   		std::cout<<"Quanti picchi gaussiani osservi in tutto?"<<std::endl;
   		std::cin>>N;

   		double nominali[N], energie[N], errori[N];

   		std::ifstream read ("picchi_FE.txt", std::ifstream::in);

   		double picchi[N][NCH], err[N][NCH];  	

   		for(int l=0; l<N ;l++){
   			std::cout<<"Valore nomilale sorgente "<<l<<" :"<<std::endl;
   			std::cin>>nominali[l];	
   		}

   		std::string plotsDir(Form("plot_lin/linearità/"));
   		system( Form("mkdir -p %s", plotsDir.c_str()));

		Double_t *err_channel=0; // Dummy

		for(int i=0; i<N; i++){
			for(int j=0; j<NCH; j++){	
				read>>picchi[i][j]>>err[i][j]; 	//lettura picchi
			}
		}

		for(int j=0; j< NCH; j++){ 

			for(int i=0; i<N ;i++){

				energie[i]=picchi[i][j];
				errori[i]=err[i][j];
			}
			TCanvas* c2 = new TCanvas("c2", "Studio Linearità");
			c2->cd();
			TGraphErrors* gr=new TGraphErrors(N, nominali, energie, err_channel, errori);
  			TF1 *f = new TF1("f", "[0] + [1]*x"); // funzione lineare
  			gr->Fit("f", "Q"); // fit sul grafico
  			gr->GetXaxis()->SetTitle("Energia Nominale (MeV)");
  			gr->GetYaxis()->SetTitle("Carica (pC)");
  			gr->SetTitle("");
			gr->SetMarkerStyle(20);
			gr->SetMarkerSize(0.5);
			gr->Draw("AP");
			c2->SaveAs(Form("%s/studio_linearità_ch_%d.pdf", plotsDir.c_str(),j+1));
			double k = f->GetParameter(1);
  			std::cout<< "k_medio ch("<<j+1<<")= "<< k <<std::endl;
  			//double_t chi2 = gr->Chisquare(f);   // Faccio il chi 2 sul modello cost = 1
  			//std::cout<< "Chi-square test returned a value of chi-square/N dof equal to: " << chi2/f->GetNDF()<< " with Ndof: "<< f->GetNDF()<<std::endl;
			delete c2;
			delete f;
		}

	}


	return 0;

}
