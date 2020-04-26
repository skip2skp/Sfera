// Programma per integrare il profilo di impulso e confrontarlo con risultato ritornato dal digitizer.

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define DT 938E-3 // ns
#define BMAX 20
#define vmin 50


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

  // Check usage
  
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
	

  	// variabili da leggere: numero evento, baseline, profilo, integrale calcolato dal digitizer

	int ev;
	float base[NCH], vcharge[NCH], pshape[NCH][1024];

	tree->SetBranchAddress("ev", &ev);
	tree->SetBranchAddress("base", &base);
	tree->SetBranchAddress("vcharge", &vcharge);
	tree->SetBranchAddress("pshape", &pshape);

	int nEntries = tree->GetEntries();

	// Crea directory per i plot
	
	std::string plotsDir(Form("plots_checkCharge_sodio/"));
	system( Form("mkdir -p %s", plotsDir.c_str()) );

	// per grafico calibrazione

	/*Double_t x[NCH]={0.};
	Double_t charges[NCH]={0.};
	Double_t err_charges[NCH]={0.};*/
	Double_t *err_x=0;
	//Double_t entries[NCH]={0.};


	// calcolo baseline
	
	TH1F* diffbaseline = new TH1F("diffbaseline", "correzione all'integrale per calcolo baseline ", 100, -0.1, 0.1);
	TH1F* min = new TH1F("min", " ", 50, -100, 10);


	//ISTOGRAMMI SOVRAPPOSTI 

	// for (int channel=0; channel<NCH; channel++) {

	// 	TH1F* others = new TH1F("others", "", 100, 0.048, 0.052);
	// 	TH1F* hist = new TH1F("hist", " ", 100, 0.048, 0.052);

	// 	for (int j=0; j<NCH; j++) {
	// 		if(j==channel){

	// 			double basenostra =0; 
	// 			for (int entry=0; entry<nEntries ; entry++) {
	// 				tree->GetEntry(entry);
	// 				float sum=0;

	// 				min->Fill(vcharge[j]); //to assess if there is an physical event


	// 				for(int i=0; i<BMAX; i++){
	// 					basenostra+=pshape[j][i];
	// 				}

	// 				basenostra/=BMAX;
	//        //diffbaseline->Fill(basenostra-base[channel]);

	// 				for (int i=BMAX; i<1024; i++) {
	// 					sum+=pshape[j][i]-base[j];
	// 				}

	// 				rapporto = sum*DT/vcharge[j];

	// 				if(vcharge[j] < -vmin) {
	// 					if(j==channel){
	// 						hist->Fill(rapporto);
	// 						diffbaseline->Fill((basenostra-base[j])*(1024)/(sum*DT));      
	// 					}
	// 					else {
	// 						others->Fill(rapporto);
	// 					}

	// 				}

	// 			}
	// 		}
	// 	}

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



	std::vector<double> means;
	std::vector<double> err_means;
	std::vector<double> x;

	
	for (int j=0; j<NCH; j++) {
	  
	  TH1F* hist = new TH1F("hist", " ", 100, 0.048, 0.052); // rapporto integrale/carica
	  double basenostra =0;
	  
	  for (int entry=0; entry<nEntries ; entry++) {
	    
	    tree->GetEntry(entry);
	    
	    float sum=0;
	    
	    min->Fill(vcharge[j]); //to assess if there is a physical event
	    
	    for(int i=0; i<BMAX; i++){
	      basenostra+=pshape[j][i]; // media sui primi BMAX valori
	    }
	    basenostra/=BMAX;
	    //diffbaseline->Fill(basenostra-base[channel]);
	    
	    for (int i=BMAX; i<1024; i++) {
	      sum+=pshape[j][i]-base[j];
	    }
	    
	    if(vcharge[j] < -vmin) {
	      hist->Fill(sum*DT/vcharge[j]); // rapporto integrale/carica
	      diffbaseline->Fill((basenostra-base[j])*(1024)/(sum*DT));  // confronto tra baseline    

	    }
	  }
	  
	  if(hist->GetEntries()>0) {
	    means.push_back(hist->GetMean());
	    err_means.push_back(hist->GetStdDev());
	    x.push_back(j+1);
	    
	  }
			    

	  
	  
	}

	/*int n=0;
	for(int i=0; i<NCH; i++){
		if(entries[i]>0){
			means.push_back(charges[i]);
			err_means.push_back(err_charges[i]);
			x_new.push_back(x[i]);
			n++;
			}
			}*/
	

      }
      
      //rapporto = sum*DT/vcharge[channel];
      //if(rapporto>0.2 && vcharge[channel] < -vmin){std::cout<<channel<<" "<<entry<<" "<< vcharge[channel]<<" "<< sum*DT<<" "<<rapporto<<std::endl;}
      //if(channel==9 && vcharge[channel] < -vmin){std::cout<<channel<<" "<<entry<<" "<< vcharge[channel]<<" "<< sum*DT<<" "<<rapporto<<std::endl;}
      
      
      if(!signalOnly) hist->Fill(sum*DT/vcharge[channel]); // Se signalOnly=0 salva tutti gli eventi
      else if(vcharge[channel] < -min_charge) hist->Fill(sum*DT/vcharge[channel]); // Altrimenti solo quelli con |carica|>min_charge
      
      charge->Fill(vcharge[channel]);
      
      if(vcharge[channel] < -min_charge){
        hist->Fill(sum*DT/vcharge[channel]);
      }
      
    }
    
    TCanvas* c1 = new TCanvas("c1","Istogramma Rapporti della Carica Misurata vs. Riportata",600,800); // Nome, Titolo,x,y
    c1->cd();
    hist->SetTitle("Istogramma Rapporti della Carica Misurata vs. Riportata");
    hist->GetXaxis()->SetTitle("Rapporto della Carica Misurata vs. Riportata");
    hist->GetYaxis()->SetTitle("Numero Eventi");
    
    TCanvas* c3 = new TCanvas("c2","Istogramma della",600,800); // Nome, Titolo,x,y
    c3->cd();
    charge->SetTitle("Istogramma della Carica");
    charge->GetXaxis()->SetTitle("Carica");
    charge->GetYaxis()->SetTitle("Numero Eventi");

    // Apre una sessione
    hist->Draw(); // Disegna l'istogramma
    c1->SaveAs(Form("%s/hist_charge_%d_ratio.pdf", plotsDir.c_str(),channel));
    
    charge->Draw();
    c3->SaveAs(Form("%s/his_charge_%d.pdf", plotsDir.c_str(),channel));
    
    
    channel_axis[channel]=channel+1;
    charge_axis[channel]=hist->GetMean();
    err_charge[channel]=hist->GetStdDev();
    
    
    delete hist;
    delete charge;
    //delete c1, c3;
    
    
    // per gli istogrammi
    
    TCanvas* c2 = new TCanvas("c1","Istogramma Rapporti della Carica Misurata vs. Riportata",600,800); // Nome, Titolo,x,y
    c2->cd();
    hist->SetTitle("Istogramma Rapporti della Carica Misurata vs. Riportata");
    hist->GetXaxis()->SetTitle("Rapporto della Carica Misurata vs. Riportata");
    hist->GetYaxis()->SetTitle("Numero Eventi");
    hist->Draw();
    c2->SaveAs(Form("%s/hist_charge_%d.pdf", plotsDir.c_str(),channel));
    
    // salva variabili per grafico somma/carica vs canale
    
    channel_axis[channel]=channel+1; // grazie:)
    charge_axis[channel]=hist->GetMean();
    err_charge[channel]=hist->GetStdDev();
    
    
    delete hist;
    //delete c1;
    
    

  }

