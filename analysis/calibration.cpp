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

#define NCH 16 // no channels
#define NBIN 100 // no bins

#define CMIN 50 // min accepted value of deposited charge

#define PEAK 662 // nominal energy of emitted photon // keV

#define ERROR_USAGE 1
#define ERROR_NOTREE 2
#define ERROR_NOFILE 3

int main(int argc, char* argv[]) {

	if(argc!=2) {
		std::cout<<"Usage: ./myexe.exe myfile.root"<<std::endl;
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

  	int ev;
  	float base[NCH], vcharge[NCH];

  	tree->SetBranchAddress("ev", &ev);
  	tree->SetBranchAddress("base", &base);
  	tree->SetBranchAddress("vcharge", &vcharge);

  	int nEntries = tree->GetEntries();  // # di righe tree = # eventi

  	//crea cartelle e sottocartelle per i plot
  	//std::string plotsDir(Form("spettro_Cesio/"));
 	  //system( Form("mkdir -p %s", plotsDir.c_str()) );


  	Double_t xmin[NCH]={1200, 480,0,780,700, 400, 1250, 1000, 1600, 980, 650, 1330, 580, 2300, 0, 0}; //... to be initialized // starting points
  	Double_t xmax[NCH]={1500, 520,0,1000, 800, 450,1400, 1150, 1900, 1100, 750, 1550, 620, 2700, 0, 0};	//... to be initialized // end points


  	std::ofstream out_k; // output file for calibration constants
  	std::string outf_name="calibration_const.dat";
  	out_k.open(outf_name);

  	if(!out_k.is_open()) {
  		std::cout<<"Error: could open file "<<outf_name<<". Exiting."<<std::endl;
  		exit(ERROR_NOFILE);
  	} //check if file is open

  	out_k<<"# ch.\tk\terr_k"<<std::endl; // header

  	Double_t amp=0., mean=0., mean_err=0., var=0., var_err=0., chi2=0., Ndf=0.; // gaussian fit parameters
  	Double_t k=0., k_err=0.; // calib constant and error

  /***********************************BEGIN FIT AND K CALC*********************************************************************************/

	for (int channel=0; channel<NCH; channel++) {

		TH1F* spettro = new TH1F("spettro","", NBIN, 0, xmax[channel]);

    	for (int entry=0; entry<nEntries; entry++) {

    		tree->GetEntry(entry); 	// prendi l'evento i-esimo

        	if(-vcharge[channel]>CMIN){spettro -> Fill(-vcharge[channel]);} //riempi l'isto

       		TF1 *fitgaus = new TF1("fitgaus", "gaus", xmin[channel], xmax[channel]); //fit gaussiana 1

					fitgaus->SetParameter(1, (xmin[channel]+xmax[channel])/2.);

    			TFitResultPtr gaussian_fit = spettro->Fit("fitgaus", "SRQ");  //“Q” Quiet mode  “S” result in TFitResultPtr ""R” Use the range

   				mean =gaussian_fit->Parameter(1);
   				var =gaussian_fit->Parameter(2);//first estimate of mean and sigma for finer fit

    			fitgaus->SetRange(mean-var, mean+2*var); // Set range of fit around the mean returned by the first. The range is asymmetrical, being larger on the right.

   				gaussian_fit = spettro->Fit("fitgaus1", "SRQ");  //“Q” Quiet mode  “S” result in TFitResultPtr ""R” Use the range
    			amp  = gaussian_fit->Parameter(0);
	    		mean = gaussian_fit->Parameter(1);
	   			var = gaussian_fit->Parameter(2);
					mean_err = gaussian_fit->ParError(1);
	    		var_err = gaussian_fit->ParError(2);
	    		chi2 =gaussian_fit->Chi2();
	    		Ndf = gaussian_fit->Ndf(); //all the parameters


	    /********************* calcolo costanti calibrazione con errore *******************************************/

	    	k=mean/PEAK; // calibration constant // pC/keV
	    	k_err=mean_err/PEAK; // error // pC/keV

	    	out_k<<channel<<"\t"<<k<<"\t"<<k_err<<"\t"<<std::endl; // prints on file

	    } // end for on entries

	} // end for on channels

	std::cout<<"Printed calibration constants on file"<<outf_name<<" ."<<std::endl;

	return(0);

}
