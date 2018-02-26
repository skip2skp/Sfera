#include <iostream>
#include <stdlib.h>

#include "TFile.h"
#include "TTree.h"
#include "TH1D.h"
#include "TCanvas.h"


int main( int argc, char* argv[] ) {

  if( argc!= 3 ) {
    std::cout << "USAGE: ./makeSpectrum [rootFileName] [channel]" << std::endl;
    exit(1);
  }

  std::string fileName(argv[1]);
  int channel(atoi(argv[2]));


  TFile* file = TFile::Open(fileName.c_str());
  TTree* tree = (TTree*)file->Get("tree");

  std::cout << "-> Opened file " << fileName.c_str() << std::endl;
  std::cout << "-> Will check spectrum of channel: " << channel << std::endl;

  TCanvas* c1 = new TCanvas("c1", "c1", 600, 600);
  c1->cd();

  int ev;
  int nch;
  float pshape[128][1024];
  float vcharge [128];

  tree->SetBranchAddress( "ev" , &ev     );
  tree->SetBranchAddress( "nch"   , &nch    );
  tree->SetBranchAddress( "pshape", &pshape );
  tree->SetBranchAddress( "vcharge", &vcharge );

  TH1D* h_charge = new TH1D("h_charge", "h_charge", 1000, -100, 20000 );

  int nentries = tree->GetEntries();

  for( unsigned iEntry=0; iEntry<nentries; ++iEntry ) {

    tree->GetEntry(iEntry);

    if( channel>=nch ) {
      std::cout << "Event " << ev << " does not have channel " << channel << " (nch=" << nch << ")." << std::endl;
      exit(11);
    }

    std::cout << "charge = " << vcharge[channel] << std::endl;
      
    h_charge->Fill(-vcharge[channel]);

    //for( unsigned i=0; i<1024; ++i ) 
    //h1->SetBinContent( i+1, pshape[channel][i] );

  } // for entries

  //h1->Draw();
  
  size_t pos = 0;
  std::string prefix;
  if((pos = fileName.find(".")) != std::string::npos) {
    prefix = fileName.substr(0, pos);
  }

  std::string plotsDir(Form("plots/%s", prefix.c_str()));
  system( Form("mkdir -p %s", plotsDir.c_str()) );

  //c1->SaveAs(Form("%s/pulseShape_ev%d_ch%d.eps",plotsDir.c_str(),event,channel));
  //c1->SaveAs(Form("%s/pulseShape_ev%d_ch%d.pdf",plotsDir.c_str(),event,channel));

  TFile* outfile = TFile::Open("output.root","recreate");
  outfile->cd();
  h_charge->Write();
  outfile->Close();

  return 0;

}
