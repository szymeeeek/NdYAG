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
    return (p[0]/2)*(1-gsl_sf_erf(((x[0]-p[1])*TMath::Sqrt(2))/p[2]));
}

Double_t waistF(Double_t *x, Double_t *p){
    return p[0]*TMath::Sqrt(1+((x[0]-p[1])/(2*p[2]))*((x[0]-p[1])/(2*p[2])));
}

Double_t beamProfPaper(Double_t *x, Double_t *p){
    Double_t c = TMath::Sqrt(TMath::Pi()/8);
    return p[0]*p[1]*c*gsl_sf_erf((TMath::Sqrt(2)*x[0])/p[1])+p[2];
}

void pumpDiode(std::string filename = "pumpDiode.txt"){
    TGraph *pumpGraph = new TGraph(filename.c_str(), "%lg %lg", "");
    TF1 *linF = new TF1("linF", "[0]*x+[1]", 0.45, 2);
    linF->SetParName(0, "m");
    linF->SetParName(1, "b");

    pumpGraph->Fit(linF, "R");

    TCanvas *c1 = new TCanvas();
    pumpGraph->SetMarkerStyle(8);
    pumpGraph->SetMarkerSize(1);
    pumpGraph->SetMarkerColor(kOcean);
    pumpGraph->SetTitle(";I [A];P [W]");
    TLatex latex;
    latex.SetNDC();
    latex.SetTextSize(0.03);
    Double_t m = linF->GetParameter(0);
    Double_t mErr = linF->GetParError(0);
    Double_t b = linF->GetParameter(1);
    Double_t bErr = linF->GetParError(1);
    TString fitInfo = Form("#splitline{m = %.3f #pm %.3f}{b = %.3f #pm %.3f}", m, mErr, b, bErr);
    c1->cd();
    pumpGraph->Draw("AP");
    latex.DrawLatex(0.4, .85, fitInfo);
    c1->SaveAs("pumpGraph.png");
}

std::vector<TF1*> beamProf(){
    std::vector<std::string> filenames = {"63d", "50d"}; //w/o file extension
    std::vector<TF1*> beamProfFits(filenames.size(), nullptr);
    std::vector<TGraphErrors*> beamProfGraphs(filenames.size(), nullptr);
    std::vector<std::string> names = {"L = 63 mm", "L = 50 mm"};
    TFile *output = new TFile("z16output.root", "UPDATE");
    if(output->IsZombie()){
        std::cout<<"The file couldn't be opened!"<<std::endl;
        return beamProfFits;
    }


    TCanvas *c1 = new TCanvas();
    c1->Divide(2, 1);
    for(int i = 0; i<filenames.size(); i++){
        std::string temp = filenames.at(i)+".txt";
        beamProfGraphs.at(i) = new TGraphErrors(temp.c_str(), "%lg %lg %lg %lg", "");

        temp = Form("%s;x[mm];P[mW]", names[i].c_str());
        beamProfGraphs.at(i)->SetTitle(temp.c_str());

        temp = filenames.at(i)+"F";
        beamProfFits.at(i) = new TF1(temp.c_str(), beamProfF, 0, 3, 3);

        if(i == 1){
            beamProfFits.at(i)->SetParameters(1, 4, 0.7); // Initial guess for P0, P1, P2
        }
        beamProfFits.at(i)->SetParLimits(0, .5, 1.2); // P0
        beamProfFits.at(i)->SetParLimits(1, 1, 4.5); // P1
        beamProfFits.at(i)->SetParLimits(2, 0, 1); // P2

        beamProfFits.at(i)->SetParNames("P0", "P1", "P2");

        beamProfGraphs.at(i)->Fit(temp.c_str(), "S V");
        c1->cd(i+1);
        beamProfGraphs.at(i)->SetMarkerColor(kOcean);
        beamProfGraphs.at(i)->SetMarkerSize(1);
        beamProfGraphs.at(i)->SetMarkerStyle(8);
        beamProfGraphs.at(i)->Draw("AP");
        beamProfGraphs.at(i)->Write();

        // Draw fit parameters on the plot
        Double_t p0 = beamProfFits.at(i)->GetParameter(0);
        Double_t p0err = beamProfFits.at(i)->GetParError(0);
        Double_t p1 = beamProfFits.at(i)->GetParameter(1);
        Double_t p1err = beamProfFits.at(i)->GetParError(1);
        Double_t p2 = beamProfFits.at(i)->GetParameter(2);
        Double_t p2err = beamProfFits.at(i)->GetParError(2);

        TString fitInfo = Form("#splitline{#splitline{P_{0} = %.3f #pm %.3f}{x_{0} = %.3f #pm %.3f}}{#splitline{w(z) = %.3f #pm %.3f}{}}", p0, p0err, p1, p1err, p2, p2err);
        TLatex latex;
        latex.SetNDC();
        latex.SetTextSize(0.03);
        latex.DrawLatex(0.15, 0.25, fitInfo);
    }
    c1->Write();
    output->Close();
    return beamProfFits;
}

// void waist(){
//     std::vector<TF1*> prevFits = beamProf();
//     std::vector<Double_t> z = {13, 15, 21.5, 29.5};
//     Double_t zErr = 0.2;
//     std::vector<Double_t> w;
//     std::vector<Double_t> wErr;

//     for(int i = 0; i<prevFits.size(); i++){
//         w.push_back(prevFits.at(i)->GetParameter(1));
//         wErr.push_back(prevFits.at(i)->GetParError(1)); 
//     }

//     TGraphErrors *wGr = new TGraphErrors(prevFits.size(), z, w, zErr, wErr);
//     TF1 *waistFit = new TF1("waist", waistF, );
// }

Bool_t timeChar(){
    TFile *output = new TFile("z16output.root", "UPDATE");
    if(output->IsZombie()){
        std::cout<<"The file couldn't be opened!"<<std::endl;
        return kFALSE;
    }

    std::vector<std::string> filenames = {"63", "51", "50"}; //w/o file extension

    std::vector<TGraphErrors*> ktpGraphs(filenames.size(), nullptr);
    std::vector<TGraphErrors*> qGraphs(filenames.size(), nullptr);

    for(int i = 0; i<filenames.size(); i++){
        std::string temp1 = filenames.at(i)+"ktp.txt";
        std::string temp2 = filenames.at(i)+"q.txt";

        ktpGraphs.at(i) = new TGraphErrors(temp1.c_str(), "%lg %lg %lg %lg", "");
        qGraphs.at(i) = new TGraphErrors(temp2.c_str(), "%lg %lg %lg %lg", "");

        TCanvas *c = new TCanvas(Form("c_time_%d", i), Form("Time Structure %s", filenames[i].c_str()), 1200, 600);
        c->Divide(2, 1);

        c->cd(1);
        ktpGraphs.at(i)->SetMarkerStyle(8);
        ktpGraphs.at(i)->SetMarkerSize(1);
        ktpGraphs.at(i)->SetMarkerColor(kAzure+2);
        ktpGraphs.at(i)->SetTitle(Form("Q-SWITCH + KTP: %s mm;Power [W];Time [us]", filenames[i].c_str()));
        ktpGraphs.at(i)->Draw("AP");
        ktpGraphs.at(i)->Write();

        c->cd(2);
        qGraphs.at(i)->SetMarkerStyle(8);
        qGraphs.at(i)->SetMarkerSize(1);
        qGraphs.at(i)->SetMarkerColor(kOrange+7);
        qGraphs.at(i)->SetTitle(Form("Q-SWITCH: %s mm;Power [W];Time [us]", filenames[i].c_str()));
        qGraphs.at(i)->Draw("AP");
        qGraphs.at(i)->Write();

        c->Write();
        c->SaveAs(Form("timeChar_%s.png", filenames[i].c_str()));
    }
    

    return kTRUE;
}

void plotDataFromFile(){
    TGraphErrors *graph = new TGraphErrors("63char.txt", "%lg %lg %lg %lg", "");
    // if (!graph || graph->GetN() == 0) {
    //     std::cout << "Failed to load data from " << filename << std::endl;
    //     return;
    // }
    TCanvas *c = new TCanvas();
    graph->SetMarkerStyle(8);
    graph->SetMarkerSize(1);
    graph->SetMarkerColor(kBlue+1);
    graph->SetTitle(";P_{pump} [W];P [mW]");
    graph->Draw("AP");
    c->SaveAs("63char.png");
}

void plotWaistFromFile(){
    TGraphErrors *graph = new TGraphErrors("w.txt", "%lg %lg %lg %lg", "");
    // if (!graph || graph->GetN() == 0) {
    //     std::cout << "Failed to load data from " << filename << std::endl;
    //     return;
    // }
    TF1 *waistFit = new TF1("waistFit", waistF, 0, 30, 3);
    waistFit->SetParNames("P0", "P1", "P2");
    graph->Fit(waistFit, "R");

    TCanvas *c = new TCanvas();
    graph->SetMarkerStyle(8);
    graph->SetMarkerSize(1);
    graph->SetMarkerColor(kBlue+1);
    graph->SetTitle(";z [cm];w [cm]");
    graph->Draw("AP");
    c->SaveAs("w.png");
}