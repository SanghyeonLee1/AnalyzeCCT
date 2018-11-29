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

//Draw graph function
TGraph* getWaveForm(TString infilename="../data/mm58/oscilloscope/VBB-0.0/C1600047.dat")
{
    ifstream in;
    in.open(infilename.Data());
    
    Double_t time=0, amplitude=0;
    
    TGraph *wave = new TGraph();
    
    Int_t line=0;
    while (1) {
        if (!in.good()) break;
//        if (line == 474600) break;
        if (line == 490000) break;
        in >> time >> amplitude;
//        wave->SetPoint(line-474000, (time+0.000003)*TMath::Power(10,6), amplitude*TMath::Power(10,3));
        wave->SetPoint(line-465000, time*TMath::Power(10,6), amplitude*TMath::Power(10,3));
        line++;
    }
    
    return wave;
}

//Main function
void FitOneWave()
{
    TGraph *gr = getWaveForm("../data/mm58/oscilloscope/VBB-0.0/C1600047.dat");
    gr->SetMarkerColor(kBlue);
    gr->SetLineColor(kBlue);
    gr->SetMarkerSize(3);
    gr->SetMarkerStyle(7);

    TF1 *PedestalGuide = new TF1("PedestalGuide", "pol0", -4., -3.5);
    gr->Fit(PedestalGuide, "NOR");

    TF1 *AfterHitGuide = new TF1("AfterHitGuide", "pol0", 0.5, 1.);
    gr->Fit(AfterHitGuide, "NOR");
    
    //Load the t, V informatin from gr
    Float_t t[25000];
    Float_t v[25000];
    Double_t xp, yp;
    for (Int_t i=0; i<25000; i++) {
        gr->GetPoint(i, xp, yp);
        t[i] = xp;
        v[i] = yp;
    }
    
    TH1F *h1 = new TH1F("h1", "", 60, 200, 900); {
        for (Int_t i=0; i<25000; i++) {
            h1->Fill(v[i]);
        }
    }

    Double_t sPedestal=0;
    Double_t sAfterHit=0;
    const Int_t nBins = h1->GetNbinsX();
    for (Int_t i=0; i<nBins; i++) {
        if (h1->GetBinContent(i) - h1->GetBinContent(i-1) > 0 && h1->GetBinContent(i) - h1->GetBinContent(i+1) > 0 && h1->GetBinContent(i) > 1000) {
            if (h1->GetBinCenter(i) > PedestalGuide->GetParameter(0) - 30.) {sPedestal = h1->GetBinCenter(i);}
            if (h1->GetBinCenter(i) < PedestalGuide->GetParameter(0) - 30.) {sAfterHit = h1->GetBinCenter(i);}
        }
    }
 
    TF1 *Pedestal = new TF1("Pedestal", "gaus", sPedestal-20., sPedestal+20.);
    h1->Fit(Pedestal, "NOR");
    cout << "Pedestal: " << Pedestal->GetParameter(1) << endl;
    cout << "Pedestal Std: " << Pedestal->GetParameter(2) << endl;
    
    TF1 *AfterHit = new TF1("AfterHit", "gaus", sAfterHit-10., sAfterHit+10.);
    h1->Fit(AfterHit, "NOR+");
    cout << "AfterHit: " << AfterHit->GetParameter(1) << endl;
    cout << "AfterHit Std: " << AfterHit->GetParameter(2) << endl;

    //for add "v[i] > mean" in next while loop2
    Double_t mean=0;
    mean = (Pedestal->GetParameter(1) + AfterHit->GetParameter(1)) / 2;

    //Get Hit time
    Int_t i=0;
    while(1) {
        if (i > 250000) break;
        if (PedestalGuide->GetParameter(0) - AfterHitGuide->GetParameter(0) < 15.) break;
        if ((v[i] < Pedestal->GetParameter(1) - Pedestal->GetParameter(2)) && (v[i+50] < mean) && (v[i+600] < AfterHit->GetParameter(1) + 4*AfterHit->GetParameter(2)))break;
        i++;
    }
    
    cout << "v[i]: " << v[i] << "\n mean: " << mean << endl;
    
    TF1 *ped = new TF1("ped", "pol0", t[i-600], t[i]);
    ped->SetParameter(0, Pedestal->GetParameter(1));
    ped->SetParLimits(0, Pedestal->GetParameter(1)-Pedestal->GetParameter(2), Pedestal->GetParameter(1)+Pedestal->GetParameter(2));
    ped->SetRange(t[i-600],t[i]);

    TF1 *fexpo = new TF1("fexpo", exp, t[i], t[i+600], 4);
    fexpo->SetParameter(0, ped->GetParameter(0));
    fexpo->SetParameter(1, t[i]);
    fexpo->SetParameter(2, 0.02);
    fexpo->SetParameter(3, Pedestal->GetParameter(1) - AfterHit->GetParameter(1));
    fexpo->SetRange(t[i],t[i+600]);
  
    fexpo->SetParName(0, "Pedestal");
    fexpo->SetParName(1, "Hit time");
    fexpo->SetParName(2, "Rise time");
    fexpo->SetParName(3, "Signal");

    gr->Fit(fexpo, "RB");
    gr->Fit(ped, "RB+");

    Int_t integer=t[i];
    Double_t Double=t[i];
    cout << "Float t[i]: " << t[i] << endl;

    TMultiGraph *mg = new TMultiGraph();
    
    mg->Add(gr);
    mg->SetTitle("single pixel waveform;Time (#mus);Voltage (mV)");
    mg->GetXaxis()->SetRangeUser(t[i-100],t[i+300]);
    mg->Draw("APL");
    //h1->Draw();
}

