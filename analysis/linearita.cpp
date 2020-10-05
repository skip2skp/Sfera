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

#deine ERROR_NOFILE 8
#define ERROR_SIZEOFDATA 26

std::map<std::string, std::vector<double>> nominal;

nominal["Cs"]={662};
nominal["Na"]={511, 1275};
nominal["Co"]={1173, 1332};
nominal["Ba"]={81, 160, 276, 302, 356}; // probabilmente non si vedranno tutti e cinque dalle misure, bisogna ridefinire accordingly

int main(int argc, char* argv[]) {

  std::ifstream input_f; // input file deve avere una riga per ciascuna sorgente, che cominci con il nome della sorgente (Cs, Co, Na, ...) e contenga a seguire picco, errore, picco, errore...
  std::string inf_name = argv[2];
  input_f.open(inf_name);
  if(!input_f.is_open()) {
    std::cout<<"Could not open file "<<inf_name<<". Exiting."<<std::endl;
    exit(ERROR_NOFILE);
  } // check

  std::vector<double[NCH]> x, y, errors, err_x; // nominali, misurati, errori su misurati, dummy variable per errori su nominali

  std::string line,word,delimiter = " ";
  std::vector<std::string> words;
  int ch=0;

  while( getline(input_f,line) ) {

    size_t pos = 0;
    std::string word;
    while ((pos = line.find(delimiter)) != std::string::npos) {
      word = line.substr(0, pos);
      line.erase(0, pos + delimiter.length());
      words.push_back(word);
    }

    if (words[0]="channel") ch++; // if the line is "" channel   N "", increases a counter

    else {

      x[ch].push_back(nominal[words[0]]); // adds nominal peaks of element indicated in first word of line to x axis
      while(int i<words.size()) {

        y[ch].push_back(atof(words[i++]));
        errors[ch].push_back(atof(words[i++])); // adds measured peaks and errors written in the line to y axis
        err_x[ch].push_back(0); // dummy errors on x

      }

    } // else, it's reading values of photopeaks

  } //while getline

  input_f.close();

  if(x.size()!=y.size()||y.size()!=errors.size()) {
    std::cout<<"Error, different number of nominal and measured peaks and/or different number of measured peaks and errors. Exiting."<<std::endl;
    exit(ERROR_SIZEOFDATA);
  } // check

  std::string plotsDir(Form("plot_linearità/")); //plot directory

  for(int i=0;i<NCH;i++) {

    TCanvas* canv = new TCanvas("canv", "Studio Linearità");
    canv->cd();
    TGraphErrors* gr = new TGraphErrors(x[ch].size(), x[ch], y[ch], err_x[ch], errors[ch]);
    TF1 *f = new TF1("f", "[0] + [1]*x"); // funzione lineare
    gr->Fit("f", "Q"); // fit sul grafico
    gr->GetXaxis()->SetTitle("Energia Nominale (MeV)");
    gr->GetYaxis()->SetTitle("Carica (pC)");
    gr->SetTitle("");
    gr->SetMarkerStyle(20);
    gr->SetMarkerSize(0.5);
    gr->Draw("AP");
    canv->SaveAs(Form("linearita_ch%d.pdf", i);
    //double_t chi2 = gr->Chisquare(f);   // Faccio il chi 2 sul modello cost = 1
    //std::cout<< "Chi-square test returned a value of chi-square/N dof equal to: " << chi2/f->GetNDF()<< " with Ndof: "<< f->GetNDF()<<std::endl;
    delete canv;

    // makes a plot for each channel

  }

  return(0);

}
