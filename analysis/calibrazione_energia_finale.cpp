#define ERROR_USAGE 1
#define ERROR_NOTREE 2

#define NCH 16
#define CMIN 30
#define NMIN 0
#define NMAX 1200
#define NBIN 100
#define MEDIA 661.7

#include<iostream>
#include<fstream>
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
#include"TMultiGraph.h"
#include"TAttLine.h"



//QUA DOBBIAMO 4 DUE ARRAY DI VMAX DAL PROGRAMMA DEL FIT (VMAX E INCERTEZZA PER DATI PARI E DISPARI) E DUE CANALI PER LA SOVRAPPOSIZIONE

int main(int argc, char* argv[]) {

  std::ifstream efs (argv[2], std::ifstream::in);
  std::ifstream ofs (argv[3], std::ifstream::in);

  //QUESTI DUE VANNO PASSATI DAL PROGRAMMA DEL FIT--PARI
  double picco_FE_p[NCH]={0.};
  double inc_picco_FE_p[NCH]={0.};

  //QUESTI DUE VANNO PASSATI DAL PROGRAMMA DEL FIT--DISPARI
  double picco_FE_d[NCH]={0.};
  double inc_picco_FE_d[NCH]={0.};
  for(int i=0;i<NCH;i++){
    double a=0;
    double b=0;
    efs >>picco_FE_p[i]>>a>>b>>inc_picco_FE_p[i];
  }
  for(int i=0;i<NCH;i++){
    double c=0;
    double d=0;
    ofs >>picco_FE_d[i]>>c>>d>>inc_picco_FE_d[i];
  }


  TString rootFileName(argv[1]);
  TFile* rootFile = new TFile(rootFileName);
  std::cout<<"Reading data from root file "<<argv[1]<<std::endl;

  TTree* tree = (TTree*) rootFile->Get("tree");
  if(!tree) {
    std::cout<<"Error, no tree called tree in "<<argv[1 ]<<". Exiting."<<std::endl;
    exit(ERROR_NOTREE);
  }

  int ev;
  float base[NCH], vcharge[NCH];

  tree->SetBranchAddress("ev", &ev);
  tree->SetBranchAddress("base", &base);
  tree->SetBranchAddress("vcharge", &vcharge);

  int nEntries = tree->GetEntries();

  std::string plotsDir(Form("Istogramma_spettro_Cesio/"));
  system( Form("mkdir -p %s", plotsDir.c_str()) );

  double k_cal[NCH] = {0.}; //k sarà il rapporto tra la MEDIA  e il picco FE per ogni canale QUA PER CALIBRARE
  double inc_k_cal[NCH] = {0.};
  double k_medio_cal = 0;
  double inc_k_medio_cal = 0;
  double picco_test[NCH] = {0.};  //k sarà il rapporto tra la MEDIA  e il picco FE per ogni canale QUA PER TESTARE
  double inc_picco_test[NCH] = {0.};
  double k_medio_test = 0;
  double inc_k_medio_test = 0;
  double k_test[NCH] = {0.};  //k sarà il rapporto tra la MEDIA  e il picco FE per ogni canale QUA PER TESTARE
  double inc_k_test[NCH] = {0.};

  for(int channel=0; channel<NCH; channel++){

    k_cal[channel] = MEDIA/picco_FE_p[channel];
    inc_k_cal[channel] = ((k_cal[channel]/picco_FE_p[channel])*(k_cal[channel]/picco_FE_p[channel]))*(inc_picco_FE_p[channel]*inc_picco_FE_p[channel]); //ERRORE QUADRATO PROPAGATO

    k_medio_cal += k_cal[channel];
    inc_k_medio_cal += inc_k_cal[channel];

    picco_test[channel] = picco_FE_d[channel]*k_cal[channel];
    inc_picco_test[channel] = (k_cal[channel]*k_cal[channel])*(inc_picco_FE_d[channel])*(inc_picco_FE_d[channel]) + (picco_FE_d[channel]*picco_FE_d[channel])*(inc_k_cal[channel]);

    k_test[channel] = MEDIA/picco_test[channel];
    inc_k_test[channel] = ((k_test[channel]/picco_test[channel])*(k_test[channel]/picco_test[channel]))*(inc_picco_test[channel]); //ERRORE QUADRATO PROPAGATO

    k_medio_test += k_test[channel];
    inc_k_medio_test += inc_k_test[channel];

  }

  k_medio_cal = k_medio_cal/NCH;
  inc_k_medio_cal = sqrt(inc_k_medio_cal)/NCH;


  k_medio_test = k_medio_test/NCH;
  inc_k_medio_test = sqrt(inc_k_medio_test)/NCH;

  Double_t channel_axis[NCH] = {0.};

  Double_t k_cal_normalizzato[NCH] = {0.};
  Double_t inc_k_cal_normalizzato[NCH] = {0.};

  Double_t k_test_normalizzato[NCH] = {0.};
  Double_t inc_k_test_normalizzato[NCH] = {0.};

  Double_t *err_channel=0; // Dummy


  for(int channel=0; channel<NCH; channel++){

    channel_axis[channel]=channel+1; 

    k_cal_normalizzato[channel]=k_cal[channel]/k_medio_cal;
    inc_k_cal_normalizzato[channel]= inc_k_cal[channel]/(k_medio_cal*k_medio_cal) + (k_cal_normalizzato[channel]*k_cal_normalizzato[channel])/(k_medio_cal*k_medio_cal)*(inc_k_medio_cal*inc_k_medio_cal);
    inc_k_cal_normalizzato[channel] = sqrt(inc_k_cal_normalizzato[channel]);

    k_test_normalizzato[channel]=k_test[channel]/k_medio_test;
    inc_k_test_normalizzato[channel]=inc_k_test[channel]/(k_medio_test*k_medio_test) + (k_test_normalizzato[channel]*k_test_normalizzato[channel])/(k_medio_test*k_medio_test)*(inc_k_medio_test*inc_k_medio_test);
    inc_k_test_normalizzato[channel] = sqrt(inc_k_test_normalizzato[channel]);
    std::cout <<"channel: "<< channel << "K: " << k_test_normalizzato[channel] << "Inc: " << inc_k_test_normalizzato[channel] << std::endl;
  }







  TCanvas* c2 = new TCanvas("c2", "Grafico Calibrazione Energie");
  c2->cd();
  TMultiGraph *mg = new TMultiGraph();
  TGraphErrors* gr=new TGraphErrors(NCH, channel_axis, k_test_normalizzato, err_channel, inc_k_test_normalizzato);

  TF1 *f = new TF1("f", "[0]"); // funzione costante
  gr->Fit("f", "QN"); // fit sul grafico
  double_t chi2 = gr->Chisquare(f);   // Faccio il chi 2 sul modello cost = 1
  std::cout<< " chi-square test returned a value of chi-square/N dof equal to: " << chi2/f->GetNDF()<< " with Ndof: "<< f->GetNDF()<<std::endl;
  gr->SetMarkerStyle(21);
  gr->SetMarkerSize(0.3);
  TGraphErrors* gr1=new TGraphErrors(NCH, channel_axis, k_cal_normalizzato, err_channel, inc_k_cal_normalizzato);
  gr1->SetMarkerColorAlpha(kRed, 1);
  gr1->SetMarkerStyle(20);
  mg->GetXaxis()->SetTitle("Canale");
  mg->GetYaxis()->SetTitle("k Normalizzato");

  mg->Add(gr1);
  mg->Add(gr);
  mg->Draw("AP");

  c2->SaveAs(Form("%s/calibrazione_energie.pdf", plotsDir.c_str()));












    //A E B DEVONO ESSERE I DUE CANALI CHE GLI PASSIAMO

  int A, B;

    // Si fa passare i due canali da riga di comando

  std::cout<<"Scegliere i due canali per la sovrapposizione delgi istogrammi:"<<std::endl;
  std::cout<<"Canale: ";
  std::cin>>A;
  std::cout<<"\nCanale: ";
  std::cin>>B;
  std::cout<<std::endl;

    // istogrammi da sovrapporre
    // prima...

  TH1F* hist1 = new TH1F("hist1",Form("Spettro Cesio [Ch: %d]", A), NBIN, NMIN, NMAX);
  TH1F* hist2 = new TH1F("hist2",Form("Spettro Cesio [Ch: %d]", B), NBIN, NMIN, NMAX);

    // scalati
    // dopo...

  TH1F* hist1_scaled = new TH1F("hist1_scaled",Form("Spettro Cesio scalato [Ch: %d]", A), NBIN, k_cal[A]*NMIN, k_cal[A]*NMAX);
  TH1F* hist2_scaled = new TH1F("hist2_scaled",Form("Spettro Cesio scalato [Ch: %d]", B), NBIN, k_cal[B]*NMIN, k_cal[B]*NMAX);  

  for (int entry=0; entry<nEntries/2; entry++) {
    tree->GetEntry(entry*2+1);

    if(-vcharge[A]>CMIN){
      hist1 -> Fill(-vcharge[A]);
      hist1_scaled -> Fill(-vcharge[A]*k_cal[A]);
    }

    if(-vcharge[B]>CMIN){
      hist2 -> Fill(-vcharge[B]);
      hist2_scaled -> Fill(-vcharge[B]*k_cal[B]);
    }

  }  

  TCanvas* H = new TCanvas("hist1","Sovrapposizione Istogrammi non Scalati"); // Nome, Titolo,x,y
  hist1->SetTitle(";Carica (pC);Numero Eventi");
  hist1->GetYaxis()->SetRangeUser(0,2500);
  H -> cd(); // Apre una sessione
  hist1->SetStats(0);
  hist1 -> Draw();  
  hist2->SetLineColor(kRed);
  hist2 -> Draw("SAME");
  H -> Update();
  H -> SaveAs(Form("%s/Istogrammi_ch%d_ch%d_sovrapposti.pdf", plotsDir.c_str(), A, B));

  delete hist1;
  delete hist2;

  TCanvas* I = new TCanvas("hist1_scaled","Sovrapposizione Istogrammi Scalati"); // Nome, Titolo,x,y
  hist1_scaled->SetTitle(";Energia (MeV);Numero Eventi");
  hist1_scaled->GetYaxis()->SetRangeUser(0,2500);
  hist1_scaled->SetStats(0);
  I -> cd(); // Apre una sessione
  hist1_scaled -> Draw();
  hist2_scaled->SetLineColor(kRed);
  hist2_scaled -> Draw("SAME");

  I -> Update();
  I -> SaveAs(Form("%s/Istogrammi_ch%d_ch%d_scalati_sovrapposti.pdf", plotsDir.c_str(), A, B));


  delete hist1_scaled;
  delete hist2_scaled;


  return 0;
}




