#include <iostream>
#include <TMath.h>
#include <gsl/gsl_sf_erf.h>

/* procedura:
    - ustawic dlugosc wneki rezonansowej,
    - wykonac pomiar P(I)
    - zbadac profil wiázki metoda zardzewialej blaszki
    - zbadac strukture czasowa z Q-SWITCH (moc sie w nim gromadzi i powoduje ze w malym okienku czasowym wydziela sie duza moc) i KPT (zamienia kolor lasera na zielony) oraz dla samego Q-SWITCH
    - profil wiazki w zaleznosci od odleglosci |plytka-dioda|
*/
Double_t beamProfF(Double_t *x, Double_t *p){
    return (p[0]/2)*(1-gsl_sf_erf(((x[0]-x[1])*TMath::Sqrt(2))/p[1]));
}

Double_t waistF(Double_t *x, Double_t *p){
    return p[0]*TMath;::Sqrt(1+((x[0]-x[1])/(2*p[1]))*((x[0]-x[1])/(2*p[1])));
}

void pumpDiode(std::string filename = "pumpDiode.txt"){
    TGraph *pumpGraph = new TGraph(filename.c_str(), "%lg %lg", "");
    TF1 *linF = new TF1("linF", "[0]*x+[1]");

    pumpGraph->Fit(linF, "S V");

    TCanvas *c1 = new TCanvas();
    pumpGraph->SetMarkerStyle(8);
    pumpGraph->SetMarkerSize(1);
    pumpGraph->SetMarkerColor(kOcean);
    c1->cd();
    pumpGraph->Draw();
    c1->SaveAs("pumpGraph.png");
}

std::vector<TF1*> beamProf(){
    std::vector<std::string> filenames = {"d13", "d15", "d21_5", "d29_5"}; //w/o file extension
    std::vector<TF1*> beamProfFits(filenames.size(), nullptr);
    std::vector<TGraphErrors*> beamProfGraphs(filenames.size(), nullptr);
    TFile *output = new TFile("z16output.root", "UPDATE");
    if(output->IsZombie()){
        std::cout<<"The file couldn't be opened!"<<std::endl;
        return beamProfFits;
    }


    TCanvas *c1 = new TCanvas();
    c1->Divide(2, 2);
    for(int i = 0; i<filenames.size(); i++){
        std::string temp = filenames.at(i)+".txt";
        beamProfGraphs.at(i) = new TGraphErrors(temp.c_str(), "%lg %lg %lg %lg", "");

        temp = Form("%s;P[mW];x[mm]", filenames[i].c_str());
        beamProfGraphs.at(i)->SetTitle(temp.c_str());

        temp = filenames.at(i)+"F";
        beamProfFits.at(i) = new TF1(temp.c_str(), beamProfF, 0, 3, 2);

        beamProfGraphs.at(i)->Fit(temp.c_str(), "S V");
        c1->cd(i+1);
        beamProfGraphs.at(i)->SetMarkerColor(kOcean);
        beamProfGraphs.at(i)->SetMarkerSize(1);
        beamProfGraphs.at(i)->SetMarkerStyle(8);
        beamProfGraphs.at(i)->Draw("AP");
    }
    c1->Write();
    return beamProfFits;
}

void waist(){
    std::vector<TF1*> prevFits = beamProf();
    std::vector<Double_t> z = {13, 15, 21.5, 29.5};
    Double_t zErr = 0.2;
    std::vector<Double_t> w;
    std::vector<Double_t> wErr;

    for(int i = 0; i<prevFits.size(); i++){
        w.push_back(prevFits.at(i)->GetParameter(1));
        wErr.push_back(prevFits.at(i)->GetParError(1)); 
    }

    TGraphErrors *wGr = new TGraphErrors(prevFits.size(), z, w, zErr, wErr);
    TF1 *waistFit = new TF1("waist", waistF, );
}