

#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH  16
#define VMAX -400
#define VMIN -600
#define TRIG 200


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

void plot(TH1F *east, TH1F *west, int, int, int);

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

	std::cout<<"Scegliere canale 1: ";
	std::cin>> int ch;

	std::string plotsDir(Form("Resolution/"));
	system( Form("mkdir -p %s", plotsDir.c_str()) );

	int ev;
	float base[2], charge[2], pshape[2][1024];

	tree->SetBranchAddress("ev", &ev);
	tree->SetBranchAddress("base", &base);
	tree->SetBranchAddress("vcharge", &charge);
	tree->SetBranchAddress("pshape", &pshape);

	double k[NCH] = {0.46, 1.36, 0, 0.95, 0.89, 1.62, 0.5, 0.64, 0.38, 0.66, 0.96, 0.44, 1.11, 0.27, 1.54, 0};


	int nEntries = tree->GetEntries();

	double difference_one = 0;		//2 exp
	double difference_two = 0; 		//2 exp mod
	//double difference_three = 0;	//fitcern	
	double difference_four = 0;		//soglia


	int count=0;//questo per la stampa dell'entry

	//questi 4 sono per la differenza varcata una soglia
	int trig_east=0; 
	int trig_west=0;
	double t_east=0;
	double t_west=0;


	TF1 *fit_doppioexp = new TF1("fit_doppioexp","[0]*(1-1/(1+exp(-100*(x-[1]))))+ ([6]*exp(-[2]*(x-[3]))+[4]*exp(-[5]*(x-[3]))) /(1+exp(-100*(x-[1])))",300,700); // doppio esponente normale
	TF1 *fit_doppioexp_mod = new TF1("fit_doppioexp_mod","[0]*(1-1/(1+exp(-100*(x-[1]))))+ ( ([0]-[4]*exp(-[5]*([1]-[3]))) /(exp(-[2]*([1]-[3]))) *exp(-[2]*(x-[3])) + [4]*exp(-[5]*(x-[3])) ) /(1+exp(-100*(x-[1])))",300,700); // doppio esponente con intercetta fissata alla baseline
	// TF1 *fit_cern = new TF1("fit_cern","[0]*(1-1/(1+exp(-100*(x-[1]))))+pow([3],[2]*(1+(x-[5])/([3]*[4])))*exp(-(x-[5])/[4])/(1+exp(-100*(x-[1])))",300,700); // 3 par_cern

	TH1F* resolution1 = new TH1F("resolution1", "", 20, -1, 1);		//2 exp
	TH1F* resolution2 = new TH1F("resolution2", "", 80, -11, 11);		//2 exp mod
	// TH1F* resolution3 = new TH1F("resolution3", "", 80, -11, 11);		//fitcern
	TH1F* resolution4 = new TH1F("resolution4", "", 15, -11, 11);		//soglia

	for(int ent=0; ent<nEntries; ent++){

		tree->GetEntry(ent);

		if(k[ch]*charge[0]<VMAX && k[ch+8]*charge[1]<VMAX && k[ch]*charge[0]>VMIN && k[ch+8]*charge[1]>VMIN && abs(k[ch]*charge[0]-k[ch+8]*charge[1])<TRIG ){

			TH1F* east = new TH1F("east", "", 1024, 0., 1024*1.2);
			TH1F* west = new TH1F("west", "", 1024, 0., 1024*1.2);

			for( unsigned i=0; i<1024; ++i ) {

				if(-pshape[0][i]>0.04 && trig_east==0){
					t_east=i*1.2;
					trig_east++;
				}

				if(-pshape[1][i]>0.04 && trig_west==0){
					t_west=i*1.2;
					trig_west++;
				}

				east->SetBinContent( (i+1), -pshape[0][i] );
				west->SetBinContent( (i+1), -pshape[1][i] );

			}//fill histograms east and west


				//METHOD 1

			fit_doppioexp -> SetParLimits(0, -0.01, 0.01);
			fit_doppioexp -> SetParLimits(1, 350, 500);
			fit_doppioexp -> SetParLimits(2, 0.005, 0.052);
			fit_doppioexp -> SetParLimits(3, 300, 1300);
			fit_doppioexp -> SetParLimits(4, -0.9, -0.2);
			fit_doppioexp -> SetParLimits(5, 0.01, 0.04);				
			fit_doppioexp -> SetParLimits(6, 0.1, 0.7);


			TFitResultPtr east_fit_one = east->Fit("fit_doppioexp", "SRQB");
			TFitResultPtr west_fit_one = west->Fit("fit_doppioexp", "SRQB");

			if(ent==2780 &&0==0) plot(east,west, ch, ent, 1);

			difference_one = east_fit_one->Parameter(1) - west_fit_one->Parameter(1);
			if(difference_one<1 && difference_one>-1) resolution1->Fill(difference_one);


				//METHOD 2

			fit_doppioexp_mod -> SetParLimits(0, -0.01, 0.01);
			fit_doppioexp_mod -> SetParLimits(1, 350, 500);
			fit_doppioexp_mod -> SetParLimits(2, 0.005, 0.052);
			fit_doppioexp_mod -> SetParLimits(3, 300, 1300);
			fit_doppioexp_mod -> SetParLimits(4, -0.9, -0.2);
			fit_doppioexp_mod -> SetParLimits(5, 0.01, 0.04);			


			TFitResultPtr east_fit_two = east->Fit("fit_doppioexp_mod", "SRQB");
			TFitResultPtr west_fit_two = west->Fit("fit_doppioexp_mod", "SRQB");

			if(ent==2780 &&0==0) plot(east,west, ch, ent, 2);

			difference_two = east_fit_two->Parameter(1) - west_fit_two->Parameter(1);
			resolution2->Fill(difference_two);


			//METHOD 3

			// fit_cern -> SetParLimits(0, -0.01, 0.01);
			// fit_cern -> SetParLimits(1, 350, 500);
			// fit_cern -> SetParLimits(2, 0.1, 0.7);
			// fit_cern -> SetParLimits(3, 0.5, 1);
			// fit_cern -> SetParLimits(4, 80, 120);
			// fit_cern -> SetParLimits(5, 300, 550);


			// TFitResultPtr east_fit_three = east->Fit("fit_cern", "SR");
			// TFitResultPtr west_fit_three = west->Fit("fit_cern", "SRQ");
			
			// if(ent==2780 &&0==0) plot(east,west, a, ent, 3);

			// difference_three = east_fit_three->Parameter(1) - west_fit_three->Parameter(1);
			// resolution3->Fill(difference_three);


			// METHOD 4

			difference_four = t_east-t_west;
			resolution4->Fill(difference_four);


			//std::cout<<"EVENT:"<<ent<<"\t"<<0<<"\t"<<difference<<"\t"<<t_east-t_west<<std::endl;

			delete east;
			delete west;

			t_east=0;
			t_west=0;
			trig_east=0;
			trig_west=0;


		}//end if condition on events

		if( count % 20000 == 0 ) std::cout << "   ... analyzing event: " << count << std::endl;
		count++;

	}//end for cycle on events

	TCanvas* method_one = new TCanvas("resolution1", "", 600, 800);
		//resolution->SetStats(0);
	method_one->cd();
	resolution1->Draw();
	method_one->SaveAs(Form("%s/resolution_ch%d_ch%d_method1.pdf", plotsDir.c_str(), ch, ch+8));

	TCanvas* method_two = new TCanvas("resolution2", "", 600, 800);
		//resolution->SetStats(0);
	method_two->cd();
	resolution2->Draw();
	method_two->SaveAs(Form("%s/resolution_ch%d_ch%d_method2.pdf", plotsDir.c_str(), ch, ch+8));

		// TCanvas* method_three = new TCanvas("resolution3", "", 600, 800);
		// //resolution->SetStats(0);
		// method_three->cd();
		// resolution3->Draw();\\
		// method_three->SaveAs(Form("%s/resolution3_ch%d_ch%d.pdf", plotsDir.c_str(), a, a+N0+1));

	TCanvas* method_four = new TCanvas("resolution4", "", 600, 800);
		//resolution->SetStats(0);
	method_four->cd();
	resolution4->Draw();
	method_four->SaveAs(Form("%s/resolution_ch%d_ch%d_method4.pdf", plotsDir.c_str(), ch, ch+8));

	delete resolution1;
	delete resolution2;
		// delete resolution3;
	delete resolution4;

	delete method_one;
	delete method_two;
		//delete method_three;
	delete method_four;

	count=0;


	return 0;
}


void plot(TH1F *east, TH1F *west, int a, int event, int method){

	std::string plotsDir(Form("Resolution/ev%d",event));
	system( Form("mkdir -p %s", plotsDir.c_str()) );

	TCanvas* checkone = new TCanvas("east", "", 600, 800);
	east->SetStats(0);
	checkone->cd();
	east->Draw();
	checkone->SaveAs(Form("%s/east_ch%d_event%d_method%d.pdf", plotsDir.c_str(), a, event, method));

	TCanvas* checktwo = new TCanvas("west", "", 600, 800);
	west->SetStats(0);
	checktwo->cd();
	west->Draw();
	checktwo->SaveAs(Form("%s/west_ch%d_event%d_method%d.pdf", plotsDir.c_str(), a+N0+1, event, method));

	delete checkone;
	delete checktwo;
}





