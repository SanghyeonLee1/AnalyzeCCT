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

Double_t exp(Double_t *x, Double_t *par) {
    return par[0] + par[3]*(TMath::Exp(-(x[0] - par[1]) / par[2]) - 1);
}

/*
Double_t pedestal(Double_t *x, Double_t *par) {
    return 757;
}
*/

//Draw graph function
TGraph* getWaveForm(TString infilename="../data/test/oscilloscope/VBB-0.0/C1600072.dat")
{
    ifstream in;
    in.open(infilename.Data());
    
    Double_t time=0, amplitude=0;
    
    TGraph *wave = new TGraph();
    
    Int_t line=0;
    while (1) {
        if (!in.good()) break;
//        if (line == 485050) break;
        if (line == 474600) break;
        in >> time >> amplitude;
        wave->SetPoint(line-474000, (time+0.000003)*TMath::Power(10,6), amplitude*TMath::Power(10,3));
//        wave->SetPoint(line-475000, time*TMath::Power(10,6), amplitude*TMath::Power(10,3));
        line++;
    }
    
    return wave;
}

//Main function
void oneDrawforms()
{
    TGraph *gr = getWaveForm("../data/test/oscilloscope/VBB-0.0/C1600072.dat");
    gr->SetMarkerColor(kBlue);
    gr->SetLineColor(kBlue);
    gr->SetMarkerSize(3);
    gr->SetMarkerStyle(7);
    
    TCanvas *c1 = new TCanvas("c1", "waveform", 800, 600);
    c1->cd();

    TF1 *fexpo = new TF1("fexpo", exp, 0.835, 0.92, 4);
    fexpo->SetParameter(0, 753);
    fexpo->SetParameter(1, 0.84);
    fexpo->SetParameter(2, 0.3);
    fexpo->SetParameter(3, 40);
/*    fexpo->SetParLimits(0, 744., 770.);
    fexpo->SetParLimits(1, 0.83, 0.85);
    fexpo->SetParLimits(2, 0.2, 0.2);
    fexpo->SetParLimits(3, 30., 50.);
    */
    TF1 *ped = new TF1("ped", "pol0", 0.79, 0.835);
    ped->SetParameter(0,756);
//    ped->SetParLimits(0, 750, 770);
    ped->SetParLimits(0, 750, 760);

    gr->Fit(fexpo, "R");
    gr->Fit(ped, "R+");
    
    TMultiGraph *mg = new TMultiGraph();
    
    mg->Add(gr);
    mg->SetTitle(";Time (#mus);Voltage (mV)");
    mg->Draw("APL");
}

