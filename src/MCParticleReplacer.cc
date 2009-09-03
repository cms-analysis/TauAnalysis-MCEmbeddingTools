#include "TauAnalysis/MCEmbeddingTools/interface/MCParticleReplacer.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

MCParticleReplacer::MCParticleReplacer(const edm::ParameterSet& pset):
  src_(pset.getParameter<edm::InputTag>("selectedParticles")),
  srcHepMC_(pset.getParameter<edm::InputTag>("HepMCSource")),
  replacer_(ParticleReplacerBase::factory(pset.getUntrackedParameter<int>("desiredReplacerClass", 1), pset)) {

  produces<edm::HepMCProduct>();
}

MCParticleReplacer::~MCParticleReplacer()
{}

// ------------ method called to produce the data  ------------
void
MCParticleReplacer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  edm::Handle<reco::MuonCollection> muons;
  iEvent.getByLabel(src_, muons);

  std::auto_ptr<HepMC::GenEvent> evt = replacer_->produce(*muons);

  if(evt.get() != 0) {
    std::auto_ptr<edm::HepMCProduct> bare_product(new edm::HepMCProduct());  
    bare_product->addHepMCData(evt.release()); // transfer ownership of the HepMC:GenEvent to bare_product

    iEvent.put(bare_product);
  }
}

void MCParticleReplacer::beginRun(edm::Run& iRun,const edm::EventSetup& iSetup)
{
  replacer_->beginRun(iRun, iSetup);
}

void MCParticleReplacer::endRun()
{
  replacer_->endRun();
}

// ------------ method called once each job just before starting event loop  ------------
void 
MCParticleReplacer::beginJob()
{
  replacer_->beginJob();
}

// ------------ method called once each job just after ending the event loop  ------------
void 
MCParticleReplacer::endJob()
{
  replacer_->endJob();
}

//define this as a plug-in
DEFINE_FWK_MODULE(MCParticleReplacer);
