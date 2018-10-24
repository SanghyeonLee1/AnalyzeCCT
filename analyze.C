#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include <TCanvas.h>
#include <TChain.h>
#include <TObject.h>
#include <TTree.h>
#include <TFile.h>
#include <TH2F.h>
#include <TMath.h>
#include <TColor.h>
#include <TStyle.h>
#include <TString.h>
#include <TROOT.h>
#include <TGraph.h>
#include <TMultiGraph.h>

using namespace std;

//Get number of files
Int_t get_evt_num(TString file_name) {
    Int_t beg = file_name.Last('C');
    Int_t end = file_name.Last('.');
    Int_t len = end-beg-1;
    TString evt_num = file_name(beg+1, len);
    return evt_num.Atoi();
}

//Draw graph function
TGraph* getWaveForm(TString infilename="../data/test/oscilloscope/VBB-0.0/C1600000.dat")
{
    ifstream in;
    in.open(infilename.Data());
    
    Double_t time=0, amplitude=0;
    
    TGraph *wave = new TGraph();
    
    Int_t line=0;
    while (1) {
        if (!in.good()) break;
        in >> time >> amplitude;
        wave->SetPoint(line-460000, time*TMath::Power(10,6), amplitude*TMath::Power(10,3));
        line++;
    }
    
    return wave;
}

//Main function
Bool_t analyze(
               TString file_in_path = "../data/test/oscilloscope/VBB-0.0",
               TString file_out_path = "../data/test/oscilloscope/results/VBB-0.0",
               Float_t vbb = -0.0
               )
{
    TString file_runlist = file_in_path + "/list.txt";
    ifstream f_runlist(file_runlist.Data());
    if (!f_runlist.is_open()) {
        cout << "runlist file not found! please check list.txt file" << endl;
        return kFALSE;
    }
    string line;
    Int_t n_cnt = 0;
    while (getline(f_runlist, line)) {
        ++n_cnt;
    }
    f_runlist.clear();
    f_runlist.seekg(0, ios::beg);
    const Int_t n_files = n_cnt;
    cout << "number of files found: " << n_files << endl; //Read number of data file
    
    if (file_out_path.CompareTo("")==0) {
        file_out_path=file_in_path;
    }
    
    cout << "file out path: " << file_out_path.Data() << endl;
    
    TFile *f_out = new TFile(Form("%s/analyze.root", file_out_path.Data()), "RECREATE");
    f_out->cd();
    TString dirname = "vbb_"; dirname += vbb;
    f_out->mkdir(dirname.Data());
    
    TString file_name_event;
    Int_t i;
    Int_t i_file;
    TString fn;
    
    Float_t time = 0;
    Float_t amplitude = 0;
    
    //loop over all files
    for (Int_t i_file=0; i_file<n_files; i_file++) {
        f_out->cd();
        
        f_runlist >> file_name_event;
        fn = file_in_path + "/" + file_name_event;
        
        cout << "'" << fn << "'" << "is opening..." << endl;
        
        TGraph *gr = getWaveForm(fn);
        
        gr->SetMarkerColor(kBlue);
        gr->SetLineColor(kBlue);
        gr->SetMarkerSize(3);
        gr->SetMarkerStyle(7);
        gr->SetTitle(";Time (#mus);Voltage (mV)");
        
        /*        TMultiGraph *mg = new TMultiGraph();
         mg->Add(gr);
         mg->SetName("%i_th", i_file);
         mg->SetTitle(";Time (#mus);Voltage (mV)");
         mg->Write();
         */
        f_out->cd(dirname.Data());
        gr->Write();
        delete gr;
    }
    
    f_out->cd();
    f_out->Close();
    
    return kTRUE;
}

