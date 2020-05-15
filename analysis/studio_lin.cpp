
#define ERROR_USAGE 1
#define ERROR_NOTREE 2
#define CMIN 50
#define CMAX 4000
#define NBIN 300

#define NCH 16

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

	int NSOR;

	std::cout<<"Quante sorgenti vuoi studiare?"<<std::endl;
	std::cin >> NSOR;
	
	int a;
	int b;
	int c;
	std::cout<<"Scegliere tra i seguenti studi:"<<std::endl;
	std::cout<<"1. Studio picchi (se già conosci i range)"<<std::endl;
	std::cout<<"2. Studio linearità (se già conosci i picchi & errori)"<<std::endl;
	std::cin >> a;
	
	if(a!=1 && a!=2){
		std::cout<<"L'opzione selezionata non è valida, scegliere nuovamente tra le opzioni sopra."<<std::endl;
		std::cin >> a;
	}

	if(a==1){

		int ev[NSOR], entries[NSOR];
		float base[NSOR][NCH], charge[NSOR][NCH];
		int npicchi;
		double_t media;
		double_t errore;

		std::ofstream out_dat;
		out_dat.open("picchi_FE.txt");

		std::ifstream range ("range.txt", std::ifstream::in);		

		std::cout<<"Premere 1 se si vuole stampare gli istogrammi, 0 altrimenti"<<std::endl;
		std::cin>> b;

		if(b!=1 && b!=0){
			std::cout<<"L'opzione selezionata non è valida, scegliere nuovamente tra le opzioni sopra."<<std::endl;
			std::cin >> b;
		}

		for(int i=0; i<NSOR; i++){

			TString rootFileName(argv[i+1]);
			TFile* rootFile = new TFile(rootFileName);
			std::cout<<"Reading data from root file "<<argv[i+1]<<std::endl;

			TTree* tree = (TTree*) rootFile->Get("tree");
			if(!tree) {
				std::cout<<"Error, no tree called tree in "<<argv[i+1]<<". Exiting."<<std::endl;
				exit(ERROR_NOTREE);
			}

			tree->SetBranchAddress("ev", &ev[i]);
			tree->SetBranchAddress("base", &base[i]);
			tree->SetBranchAddress("vcharge", &charge[i]);
			entries[i] = tree->GetEntries();
 			
 			range.ignore(200,' ');
			range >> npicchi;

			for(int k=0; k<npicchi;k++){

				for (int j=0; j<NCH; j++) {

					TH1F* hist = new TH1F("hist", " ", NBIN, CMIN, CMAX);

					for (int entry=0; entry<entries[i] ; entry++) {
						tree->GetEntry(entry);
						hist->Fill(-charge[i][j]);
					}

					// LEVA COMMENT
					
					double min, max;
 					range.ignore(200,' ');
 					range>>min>>max;

					if(b==1){
						std::string sorgente = "source"+std::to_string(i+1);
						std::string plotsDir(Form("plot_lin/%s/", sorgente.c_str()));
						system( Form("mkdir -p %s", plotsDir.c_str()));

						TCanvas* c = new TCanvas("c3","Istogramma con tutte le cariche",2000,800); // Nome, Titolo,x,y
  						c->cd(); // Apre una sessione
  						hist->SetLineColor(2);
  						hist->SetXTitle("Carica (pC)");
  						hist->SetYTitle("N eventi");
  						hist->SetStats(0);
 						hist->Draw(); // Disegna l'istogramma
 						c->SaveAs(Form("%s/istogramma_cariche_sor_%d_canale_%d.pdf", plotsDir.c_str(),i+1,j+1));

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

   					std::cout<<"ev("<<i<<") "<<"ch("<<j<<") "<< "min "<< min<<"max "<<max <<"media "<<media<<std::endl; 

   					delete hist;
   				}
   			}
   		}

   		out_dat.close();
   		std::cout<<"Vuoi proseguire con lo studio della linearità? Premere 1 se si, 0 altrimenti:"<<std::endl;
   		std::cin>>c;
   		if(c!=1 && c!=0){
   			std::cout<<"L'opzione selezionata non è valida, scegliere nuovamente tra le opzioni sopra."<<std::endl;
   			std::cin >> c;
   		}
   	}

   	if(a==2 || c==1){

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