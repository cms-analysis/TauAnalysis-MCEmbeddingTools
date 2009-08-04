// -*- C++ -*-
//
// Package:    Replacer
// Class:      Replacer
//
/**\class Replacer Replacer.cc TauAnalysis/MCEmbeddingTools/src/Replacer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Manuel Zeise
//
//


// system include files
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include <stack>
#include <queue>

#include "Math/LorentzVector.h"
#include "Math/VectorUtil.h"
#include "DataFormats/Math/interface/deltaR.h"

#include "DataFormats/Candidate/interface/Particle.h"
#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"

/*
#include "HepMC/IO_HEPEVT.h"
#include "HepMC/GenParticle.h"
#include "HepMC/GenVertex.h"
#include "HepMC/PythiaWrapper.h"
*/

#include "GeneratorInterface/ExternalDecays/interface/TauolaInterface.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "HepPDT/ParticleDataTable.hh"

#include "DataFormats/MuonReco/interface/Muon.h"

#include "PhysicsTools/UtilAlgos/interface/TFileService.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "TTree.h"

using namespace std;
using namespace edm;

class ParticleReplacerClass : public edm::EDProducer
{
public:
	explicit ParticleReplacerClass(const edm::ParameterSet&);
	~ParticleReplacerClass();

	virtual void produce(edm::Event& iEvent, const edm::EventSetup& iSetup);
	virtual void beginRun(edm::Run& iRun,const edm::EventSetup& iSetup);
	virtual void beginJob();
	virtual void endJob();

private:
	void transformMuMu2TauTau(reco::Particle * muon1, reco::Particle * muon2);
	void transformMuMu2TauNu(reco::Particle * muon1, reco::Particle * muon2);

	HepMC::GenEvent * processEventWithTauola(HepMC::GenEvent * evt);
	HepMC::GenEvent * processEventWithPythia(HepMC::GenEvent * evt);
	
	bool testEvent(HepMC::GenEvent * evt);	

	void cleanEvent(HepMC::GenEvent * evt, HepMC::GenVertex * vtx);
	void repairBarcodes(HepMC::GenEvent * evt);

	/// replace mode:
	//	0	fullEvent	(incl. all other particles)
	//	1	taus only	(create an event only with the two decaying taus)
	unsigned int replacementMode_;
	unsigned int generatorMode_;
	bool noInitialisation_;

	// this variable defines the type of decay to simulate
	// 0 - mumu->mumu (i.e. no transformation)
	// 1 - mumu->tautau (default value)
	unsigned int transformationMode_;
	
	string HepMCSource_;
	string selectedParticles_;

	int motherParticleID_;
	bool useExternalGenerators_ ;
	bool useTauola_ ;
	bool useTauolaPolarization_ ;
	
	gen::TauolaInterface tauola_;

	bool printEvent_;

	double minVisibleTransverseMomentum_;
	
	double targetParticleMass_;
	int targetParticlePdgID_;
	
	TTree * outTree;
	int attempts;
};


