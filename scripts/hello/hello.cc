
#include <JANA/JEventProcessor.h>
#include <JANA/Services/JGlobalRootLock.h>
#include <TH1D.h>
#include <TFile.h>

class helloProcessor: public JEventProcessor {
private:
    std::string m_tracking_alg = "genfit";
    std::shared_ptr<JGlobalRootLock> m_lock;
    TFile* dest_file;
    TDirectory* dest_dir; // Virtual subfolder inside dest_file used for this specific processor
    
    /// Declare histograms here
    TH1D* h1d_pt_reco;

public:
    helloProcessor() {
        SetTypeName(NAME_OF_THIS); // Provide JANA with this class's name
    }
    
    void Init() override {
        auto app = GetApplication();
        m_lock = app->GetService<JGlobalRootLock>();

        /// Set parameters to control which JFactories you use
        app->SetDefaultParameter("tracking_alg", m_tracking_alg);

        /// Set up histograms
        m_lock->acquire_write_lock();
        //dest_file = ... /// TODO: Acquire dest_file via either a JService or a JParameter
        dest_dir = dest_file->mkdir("hello"); // Create a subdir inside dest_file for these results
        dest_file->cd();
        h1d_pt_reco = new TH1D("pt_reco", "reco pt", 100,0,10);
        h1d_pt_reco->SetDirectory(dest_dir);
        m_lock->release_lock();
    }

    void Process(const std::shared_ptr<const JEvent>& event) override {

        /// Acquire any results you need for your analysis
        //auto reco_tracks = event->Get<RecoTrack>(m_tracking_alg);

        m_lock->acquire_write_lock();
        /// Inside the global root lock, update histograms
        // for (auto reco_track : reco_tracks) {
        //    h1d_pt_reco->Fill(reco_track->p.Pt());
        // }
        m_lock->release_lock();
    }

    void Finish() override {
        // Close TFile (unless shared)
    }
};
    
extern "C" {
    void InitPlugin(JApplication *app) {
        InitJANAPlugin(app);
        app->Add(new helloProcessor);
    }
}
    
