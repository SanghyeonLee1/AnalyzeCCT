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

//Get number of files
Int_t get_evt_num(TString file_name) {
    Int_t beg = file_name.Last('C');
    Int_t end = file_name.Last('.');
    Int_t len = end-beg-1;
    TString evt_num = file_name(beg+1, len);
    return evt_num.Atoi();
}

//Draw graph function
TGraph* getWaveForm(TString infilename="../data/mm36/oscilloscope/VBB-6.0/C1600000.dat")
{
    ifstream in;
    in.open(infilename.Data());
    
    Double_t time=0, amplitude=0;
    
    TGraph *wave = new TGraph();
    
    Int_t line=0;
    while (1) {
        if (!in.good()) break;
//        if (line == 485050) break;
        if (line == 490000) break;
        in >> time >> amplitude;
        wave->SetPoint(line-465000, time*TMath::Power(10,6), amplitude*TMath::Power(10,3));
        line++;
    }
    
    return wave;
}

//Main function
Bool_t analyze(
               TString file_in_path = "../data/mm58/oscilloscope/VBB-0.0",
               TString file_out_path = "../data/mm58/oscilloscope/results/VBB-0.0",
               Float_t vbb = -0.0
               )
{
    TString file_runlist = file_in_path + "/list.txt";  //Read file list
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
    cout << "number of files found: " << n_files << endl;   //Read number of data file
    
    if (file_out_path.CompareTo("")==0) {
        file_out_path=file_in_path;
    }
    
    cout << "file out path: " << file_out_path.Data() << endl;
    
    TFile *f_out = new TFile(Form("%s/analyze.root", file_out_path.Data()), "RECREATE");
    f_out->cd();
    TString dirname = "vbb_"; dirname += vbb;
    f_out->mkdir(dirname.Data());
    
    TString file_name_event;
    TString fn;
    
    Float_t time = 0;
    Float_t amplitude = 0;
    
    Double_t i_file = 0;
    //loop over all files
    while(1) {
        if (i_file == n_files) break;
        f_out->cd();
        
        f_runlist >> file_name_event;
        fn = file_in_path + "/" + file_name_event;
        
        cout << "'" << fn << "'" << "is opening..." << endl;
        
        TGraph *gr = getWaveForm(fn);
        
        gr->SetMarkerColor(kBlue);
        gr->SetLineColor(kBlue);
        gr->SetMarkerSize(3);
        gr->SetMarkerStyle(7);
        gr->SetTitle("single pixel waveform;Time (#mus);Voltage (mV)");
        
        TF1 *PedestalGuide = new TF1("PedestalGuide", "pol0", -4, -3.5);
        gr->Fit(PedestalGuide, "NOR");
        
        TF1 *AfterHitGuide = new TF1("AfterHitGuide", "pol0", 0.5, 1);
        gr->Fit(AfterHitGuide, "NOR");
        
        //Load the t, V informatin from gr
        Float_t t[30000];
        Float_t v[30000];
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
        const int nBins = h1->GetNbinsX();
        for (Int_t i=0; i<nBins; i++) {
            if (h1->GetBinContent(i) - h1->GetBinContent(i-1) > 0 && h1->GetBinContent(i) - h1->GetBinContent(i+1) > 0 && h1->GetBinContent(i) > 1000) {
                if (h1->GetBinCenter(i) > PedestalGuide->GetParameter(0) - 30) {sPedestal = h1->GetBinCenter(i);}
                if (h1->GetBinCenter(i) < PedestalGuide->GetParameter(0) - 30) {sAfterHit = h1->GetBinCenter(i);}
            }
        }
        
        TF1 *Pedestal = new TF1("Pedestal", "gaus", sPedestal-20, sPedestal+20);
        h1->Fit(Pedestal, "NOR");
        cout << "Pedestal: " << Pedestal->GetParameter(1) << endl;
        cout << "Pedestal Std: " << Pedestal->GetParameter(2) << endl;
        
        TF1 *AfterHit = new TF1("AfterHit", "gaus", sAfterHit-10, sAfterHit+10);
        h1->Fit(AfterHit, "NOR+");
        cout << "AfterHit: " << AfterHit->GetParameter(1) << endl;
        cout << "AfterHit Std: " << AfterHit->GetParameter(2) << endl;
        
        //for add "v[i] > mean" in next while loop
        Double_t mean;
        mean = (Pedestal->GetParameter(1) + AfterHit->GetParameter(1)) / 2;
        
        if (Pedestal->GetParameter(1) - AfterHit->GetParameter(1) < 10) {
            i_file++;
        }
        if (Pedestal->GetParameter(1) - AfterHit->GetParameter(1) > 10) {
        //Get Hit time
        Int_t i=0;
        while(1) {
            if (i > 25000) break;
            if (PedestalGuide->GetParameter(0) - AfterHitGuide->GetParameter(0) < 15.) break;
            if ((v[i] < Pedestal->GetParameter(1) - Pedestal->GetParameter(2)) && (v[i+50] < mean) && (v[i+600] < AfterHit->GetParameter(1) + 4*AfterHit->GetParameter(2)))break;
            i++;
        }

        TF1 *ped = new TF1("ped", "pol0", t[i-600], t[i]);
        ped->SetParameter(0, PedestalGuide->GetParameter(0));
        ped->SetParLimits(0, Pedestal->GetParameter(1)-Pedestal->GetParameter(2), Pedestal->GetParameter(1)+Pedestal->GetParameter(2));
        ped->SetRange(t[i-600],t[i]);
        
        TF1 *fexpo = new TF1("fexpo", exp, t[i], t[i+600], 4);
        fexpo->SetParameter(0, PedestalGuide->GetParameter(0));
        fexpo->SetParameter(1, t[i]);
        fexpo->SetParameter(2, 0.05);
        fexpo->SetParameter(3, Pedestal->GetParameter(1) - AfterHit->GetParameter(1));
        fexpo->SetRange(t[i],t[i+600]);
        fexpo->SetParName(0, "Pedestal");
        fexpo->SetParName(1, "Hit time");
        fexpo->SetParName(2, "Rise time");
        fexpo->SetParName(3, "Signal");
        
        gr->Fit(fexpo, "BR");
        gr->Fit(ped, "BR+");
        gr->GetXaxis()->SetRangeUser(t[i-300],t[i+300]);
        
        f_out->cd(dirname.Data());
        gr->Write();
        delete gr;
        i_file++;
        }
    }
    
    f_out->cd();
    f_out->Close();
    
    return kTRUE;
}

