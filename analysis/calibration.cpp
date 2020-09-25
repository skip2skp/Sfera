//23/09/2020
// SP, AQ, LS
// Il programma fa un fit gaussiano del picco fotoelettrico degli spettri di assorbimento della radiazione del Cesio sui vari canali e ne ricava le costanti di calibrazione confrontando le medie
// con il valore teorico dell'energia del fotone
// Stampa i risultati su file di testo.


#include<iostream>
#include<stdio.h>
#include<vector>
#include<cmath>
#include<fstream>
#include<iomanip>

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
#include "TLegend.h"
#include "TGaxis.h"
#include "TStyle.h"
#include "TPaveStats.h"
#include "TPaveText.h"
#include "TLatex.h"


#define NBIN 100 // no bins

#define CMIN 50 // min accepted value of deposited charge

#define PEAK 662 // nominal energy of emitted photon // keV

#define ERROR_USAGE 1
#define ERROR_NOTREE 2
#define ERROR_NOFILE 3

int main(int argc, char* argv[]) {

	if(argc!=3) {
		std::cout<<"Usage: ./myexe.exe myfile.root filewith.boundaries"<<std::endl;
		exit(ERROR_USAGE);
	}

// lettura Tree e controllo

  	TString rootFileName(argv[1]);
  	TFile* rootFile = new TFile(rootFileName);
  	std::cout<<"Reading data from root file "<<argv[1]<<std::endl;

  	TTree* tree = (TTree*) rootFile->Get("tree");
  	if(!tree) {
    	std::cout<<"Error, no tree called \"tree\" in "<<argv[1]<<". Exiting."<<std::endl;
    	exit(ERROR_NOTREE);
  	}
  	
  
  	
	int NCH;

	tree->SetBranchAddress("nch", &NCH);
	tree->GetEntry(0); // reads number of channels
  	int ev;
  	float base[NCH], vcharge[NCH];

  	tree->SetBranchAddress("ev", &ev);
  	tree->SetBranchAddress("base", &base);
  	tree->SetBranchAddress("vcharge", &vcharge);

  	int nEntries = tree->GetEntries();  // # di righe tree = # eventi

  	//crea cartelle e sottocartelle per i plot
  	 std::string plotsDir(Form("spettro_Cesio/"));
 	 system( Form("mkdir -p %s", plotsDir.c_str()) );


 	 double endpoint;//={2000, 700, 2000, 1200, 1100, 700, 1800, 1500, 2200, 1400, 1400, 1800, 2000, 1000, 3000, 1000};
 	 int bin;//={120, 180, 120, 120, 180, 180, 120, 120, 110, 120, 180, 110, 180, 100, 180};
  	double xmin;//={1300,470,1450,950,700,380,1250,1000,1600,950,1400,1350,550,2200,390}; //... to be initialized // starting points
  	double xmax;//={1700,600,1800,1150,900,500,1500,1200,1400,1150,1600,1700,700,2800,500};	//... to be initialized // end points


  	//conviene passarli da file

  	std::ifstream input_f;
  	std::string inf_name=argv[2];
  	input_f.open(inf_name);
  	if(!input_f.is_open()) {

		std::cout<<"Error: No file called "<<inf_name<<". Exiting."<<std::endl;
  		exit(ERROR_NOFILE);

  	} //check if file is open  	

  	std::ofstream out_k; // output file for calibration constants
  	std::string outf_name="calibration_const.dat";
  	out_k.open(outf_name);
  	if(!out_k.is_open()) {
  		std::cout<<"Error: could open file "<<outf_name<<". Exiting."<<std::endl;
  		exit(ERROR_NOFILE);
  	} //check if file is open

        std::ofstream out_fit; // output file for fit results
  	std::string outf_name2="par_fit.dat";
  	out_fit.open(outf_name2);
  	if(!out_fit.is_open()) {
  		std::cout<<"Error: could open file "<<outf_name<<". Exiting."<<std::endl;
  		exit(ERROR_NOFILE);
  	} //check if file is open

  	out_k<<"# ch.\tk\terr_k"<<std::endl; // header
	out_fit<<"#amp----mean----var----mean_err----var_err----chi2/Ndf"<< std::endl;
  	Double_t amp=0., mean=0., mean_err=0., var=0., var_err=0., chi2=0., Ndf=0.; // gaussian fit parameters
  	double k,k_err;
  /***********************************BEGIN FIT AND K CALC*********************************************************************************/

	for (int channel=0; channel<NCH; channel++) {

		input_f>>endpoint;
  		//input_f>>bin;
  		input_f>>xmin;
  		input_f>>xmax; // reads boundaries for fit on the fly from input file

		//TH1F* spettro = new TH1F("spettro","", NBIN, 0, bin);
		TH1F* spettro = new TH1F("spettro","", 100, 100, endpoint);


    	for (int entry=0; entry<nEntries; entry++) {

    		tree->GetEntry(entry); 	// prendi l'evento i-esimo

        	if(-vcharge[channel]>CMIN){spettro -> Fill(-vcharge[channel]);} //riempi l'isto
    } // end for on entries
       		TF1 *fitgaus = new TF1("fitgaus", "gaus", xmin, xmax); //fit gaussiana 1

				fitgaus->SetParameter(1, (xmin+xmax)/2.);

    			TFitResultPtr gaussian_fit = spettro->Fit("fitgaus", "SRQ");  //“Q” Quiet mode  “S” result in TFitResultPtr ""R” Use the range
				
				amp =gaussian_fit->Parameter(1);
   				mean =gaussian_fit->Parameter(1);
   				var =gaussian_fit->Parameter(2);//first estimate of mean and sigma for finer fit
				mean_err = gaussian_fit->ParError(1);
	    		var_err = gaussian_fit->ParError(2);
	    		chi2 =gaussian_fit->Chi2();
	    		Ndf = gaussian_fit->Ndf(); //all the parameters
	    			
	    		out_fit<<amp<<"\t"<<mean<<"\t"<<var<<"\t"<<mean_err<<"\t"<<var_err<<"\t"<<chi2/Ndf<< std::endl;
	    			
	    			
    	//fitgaus->SetRange(mean-var, mean+2*var); // Set range of fit around the mean returned by the first. The range is asymmetrical, being larger on the right.
		
		
		
   			
   			
   		/*	TF1* fitgauss2= new TF1("fitgauss2", "gaus", xmin, xmax);
      
    fitgauss2->SetParameter(0,amp);
    fitgauss2->SetParameter(1,mean);
    
     TFitResultPtr gaussian_fit2=spettro->Fit("fitgauss2", "SR");
		amp  = gaussian_fit2->Parameter(0);
	    	mean = gaussian_fit2->Parameter(1);
	   	var = gaussian_fit2->Parameter(2);
		mean_err = gaussian_fit2->ParError(1);
	    	var_err = gaussian_fit2->ParError(2);
	    	chi2 =gaussian_fit2->Chi2();
	    	Ndf = gaussian_fit2->Ndf(); //all the parameters
	*/
   			
    TCanvas* plot_spettro = new TCanvas("spettro","",1920,1080);
    spettro->SetStats(0); // Leva il pannello con entries mean devstd
	// spettro->SetTitle(Form("Spettro Cesio [Ch: %d];Carica (pC);Numero Eventi",channel));
	 spettro->GetYaxis()->SetTitle("Numero Eventi");
	 spettro->GetXaxis()->SetTitle("Carica [pC]");
	 plot_spettro -> cd(); // Apre una sessione
	

   spettro -> Draw(); // Disegna l'istogramma
	
 TLatex *l = new TLatex(0.83,0.87,Form( "ch[%d]",channel));
l->SetNDC();
l->SetTextSize(0.04);
l->Draw("same");


		
	plot_spettro -> SaveAs(Form("%s/Ist_Spettro_%d.pdf", plotsDir.c_str(),channel));

    delete plot_spettro;
      

      
    delete spettro;			
   			
   			
   			
   			
   			
   			
   			
   			

	    

	    	k=mean/PEAK; // calibration constant // pC/keV
	    	k_err=mean_err/PEAK; // error // pC/keV

	    	out_k<<channel<<"\t\t"<<k<<"\t\t"<<k_err<<"\t\t"<<std::endl; // prints on file

	

	} // end for on channels

	std::cout<<"Printed calibration constants on file"<<outf_name<<" ."<<std::endl;

	out_fit.close();
	out_fit.close();
	input_f.close();

	return(0);

	
}
