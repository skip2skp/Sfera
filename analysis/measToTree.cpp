#include "TFile.h"
#include "TTree.h"

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>


bool isNumber(const std::string& s);


int main( int argc, char* argv[] ) {

  if( argc != 2 ) {

    std::cout << "USAGE: ./measToTree [filename]" << std::endl;
    exit(1);

  }

  std::string fileName(argv[1]);

  if( fileName.rfind("../data/",0)==0 )
    fileName.erase(0, 8);

  std::ifstream fs(Form("../data/%s", fileName.c_str()));
  if( !fs.good() ) {
    std::cout << "-> No file called '" << fileName << "' found in '../data/'. Exiting." << std::endl;
    exit(1);
  }

  std::cout << "-> Opened measurements-only data file: " << fileName << std::endl;

  size_t pos = 0;
  std::string outfileName;
  if((pos = fileName.find(".")) != std::string::npos) {
    std::string prefix = fileName.substr(0, pos);
    outfileName = prefix + ".root";
  }

  TFile* outfile = TFile::Open( outfileName.c_str(), "recreate" );
  TTree* tree = new TTree( "tree", "" );


  int ev;
  int nch;
  float base     [128];
  float vamp     [128];
  float vcharge  [128];
  float letime   [128];
  float tetime   [128];
  //float ratecount[128];

  tree->Branch( "ev"       , &ev      , "ev/I"            );
  tree->Branch( "nch"      , &nch     , "nch/I"           );
  tree->Branch( "base"     , base     , "base[nch]/F"     );
  tree->Branch( "vamp"     , vamp     , "vamp[nch]/F"     );
  tree->Branch( "vcharge"  , vcharge  , "vcharge[nch]/F"  );
  tree->Branch( "letime"   , letime   , "letime[nch]/F"   );
  tree->Branch( "tetime"   , tetime   , "tetime[nch]/F"   );
  //tree->Branch( "ratecount", ratecount, "ratecount[nch]/F");


  std::string line;
  bool wasReadingEvent = false;
  int ch = -1;


  if( fs.good() ) {

    std::cout << "-> Starting parsing file." << std::endl;
    nch=0;

    while( getline(fs,line) ) {

      //std::cout << line << std::endl;
      line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());

      std::string delimiter = " ";
      size_t pos = 0;
      std::vector<std::string> words;
      std::string word;
      while ((pos = line.find(delimiter)) != std::string::npos) {
        word = line.substr(0, pos);
        line.erase(0, pos + delimiter.length());
        words.push_back(word);
      }

      std::vector< std::string > words_cleaned;
      for( unsigned i=0; i<words.size(); ++i ) {
        if( isNumber(words[i]) ) words_cleaned.push_back( words[i] );
      }


      if( words[0]=="===" && words[1]=="Event" && wasReadingEvent ) {

        if( ev % 100 == 0 ) std::cout << "   ... analyzing event: " << ev << std::endl;

        tree->Fill();
 
        nch = 0;
        ch = -1;
        wasReadingEvent = false;

      } else if( words[0]!="===" && words_cleaned.size()==7 ) {

        wasReadingEvent = true;

        nch += 1;

        ch            = atoi(words_cleaned[0].c_str());
        //ev            = atoi(words_cleaned[1].c_str());
        base     [ch] = atof(words_cleaned[2].c_str());
        vamp     [ch] = atof(words_cleaned[3].c_str());
        vcharge  [ch] = atof(words_cleaned[4].c_str());
        letime   [ch] = atof(words_cleaned[5].c_str());
        tetime   [ch] = atof(words_cleaned[6].c_str());
        //ratecount[ch] = atof(words_cleaned[15].c_str());

      }

      if( words[0]=="===" && words[1]=="Event" && wasReadingEvent==false ) {
	ev            = atoi(words[2].c_str());	
	//std::cout << ev << std::endl;
      }

    } // while get lines

  } // if file is good

  if( wasReadingEvent )
    {
      std::cout << "   ... analyzing event: " << ev << std::endl;
      tree->Fill();
    }

  fs.close();

  tree->Write();
  outfile->Close();

  std::cout << "-> Tree saved in: " << outfile->GetName() << std::endl;

  return 0;

}



bool isNumber(const std::string& s) {

  std::string::const_iterator it = s.begin();
  while (it != s.end() && (std::isdigit(*it) || (*it)==std::string(".") || (*it)==std::string("-")) ) ++it;
  return !s.empty() && it == s.end();

}

