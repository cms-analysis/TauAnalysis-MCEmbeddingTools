#include "TauAnalysis/MCEmbeddingTools/interface/ParticleReplacerClass.h"

#ifndef TXGIVE
#define TXGIVE txgive_
extern "C" {
  void TXGIVE(const char*,int length);
}
#endif

#ifndef TXGIVE_INIT
#define TXGIVE_INIT txgive_init_
extern "C" {
  void TXGIVE_INIT();
}
#endif

ParticleReplacerClass::ParticleReplacerClass(const edm::ParameterSet& pset) : tauola_(pset)
{
// 	using namespace reco;
	using namespace edm;
	using namespace std;

	// this module creates a edm::HepMCProduct
	produces<edm::HepMCProduct>();
	
	//HepMC::HEPEVT_Wrapper::set_max_number_entries(4000);
	//HepMC::HEPEVT_Wrapper::set_sizeof_real(8);

	// transformationMode =
	//  0 - no transformation
	//  1 - mumu -> tautau
	transformationMode_ = pset.getUntrackedParameter<int>("transformationMode",1);
	switch (transformationMode_)
	{
		case 0:
		{	
			LogInfo("Replacer") << "won't do any transformation with the given mumu";
			break;
		}
		case 1:
		{
			LogInfo("Replacer") << "will transform mumu into tautau";
			break;
		}
		case 2:
    {
      LogInfo("Replacer") << "will transform mumu into taunu (as coming from a W boson)";
      break;
    }         
	}
	
	// replacementMode =
	//	0 - remove Myons from existing HepMCProduct and implant taus (+decay products)
	//	1 - build new HepMCProduct only with taus (+decay products)
	replacementMode_ = pset.getUntrackedParameter<int>("replacementMode",1);

	// generatorMode =
	//	0 - use Pythia
	//	1 - use Tauola
	generatorMode_ = pset.getUntrackedParameter<int>("generatorMode",0);

	// If one wants to use two instances of this module in one
	// configuration file, there might occur some segmentation
	// faults due to the second initialisation of Tauola. This
	// can be prevented by setting noInitialisation to false.
	//          Caution: This option is not tested!
	noInitialisation_ = pset.getUntrackedParameter<bool>("noInitialisation",false);

	printEvent_ = pset.getUntrackedParameter<bool>("printEvent",false);

	motherParticleID_ = pset.getUntrackedParameter<int>("motherParticleID",23);

	selectedParticles_ = pset.getUntrackedParameter<string>("selectedParticles","selectMuons");

	HepMCSource_ = pset.getUntrackedParameter<string>("HepMCSource","source");

	// requires the visible decay products of a tau to have a sum transverse momentum
	minVisibleTransverseMomentum_ = pset.getUntrackedParameter<double>("minVisibleTransverseMomentum",10.);

	edm::Service<TFileService> fileService_;
	outTree = fileService_->make<TTree>( "event_generation","This tree stores information about the event generation");
	outTree->Branch("attempts",&attempts,"attempts/I");

	edm::LogInfo("Replacer") << "generatorMode = "<< generatorMode_<< "\n";
	edm::LogInfo("Replacer") << "replacementMode = "<< replacementMode_<< "\n";

	return;
}

ParticleReplacerClass::~ParticleReplacerClass()
{
	// do anything here that needs to be done at desctruction time
	// (e.g. close files, deallocate resources etc.)
}

// ------------ method called to produce the data  ------------
void
ParticleReplacerClass::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
	using namespace edm;
	using namespace std;
	using namespace HepMC;

	HepMC::GenEvent * evt=0;

	GenVertex * zvtx = new GenVertex();

	reco::GenParticle * part1=0;
	reco::GenParticle * part2=0;

	/// 1) access the muons to be used
	Handle<std::vector<reco::Muon> > dataHandle;
	if (!iEvent.getByLabel(selectedParticles_,dataHandle))
	{
		edm::LogError("Replacer") << "Stored muons not found:\n"<< selectedParticles_;
		return;
	}
	const std::vector<reco::Muon> muons = *( dataHandle.product() );

	/// 2) transform the muons to the desired particles
	std::vector<reco::Particle> particles;	
	switch (transformationMode_)
	{
		case 0:	// mumu->mumu
		{
			if (muons.size()!=2)
			{
				LogError("Replacer") << "the decay mode Z->mumu requires exactly two muons, aborting processing";
				return;
			}
	
			targetParticleMass_  = 0.105658369;
			targetParticlePdgID_ = 13;
				
			reco::Muon muon1 = muons.at(0);
			reco::Muon muon2 = muons.at(1);
			reco::Particle tau1(muon1.charge(), muon1.p4(), muon1.vertex(), muon1.pdgId(), 0, true);
			reco::Particle tau2(muon2.charge(), muon2.p4(), muon2.vertex(), muon2.pdgId(), 0, true);
			particles.push_back(tau1);
			particles.push_back(tau2);
			break;
		} 
		case 1:	// mumu->tautau
		{
			if (muons.size()!=2)
			{
				LogError("Replacer") << "the decay mode Z->tautau requires exactly two muons, aborting processing";
				return;
			}

			targetParticleMass_  = 1.77690;
			targetParticlePdgID_ = 15;
			
			reco::Muon muon1 = muons.at(0);
			reco::Muon muon2 = muons.at(1);
			reco::Particle tau1(muon1.charge(), muon1.p4(), muon1.vertex(), muon1.pdgId(), 0, true);
			reco::Particle tau2(muon2.charge(), muon2.p4(), muon2.vertex(), muon2.pdgId(), 0, true);
			transformMuMu2TauTau(&tau1, &tau2);
			particles.push_back(tau1);
			particles.push_back(tau2);			
			break;
		}
    case 2: // mumu->taunu (W boson)
    {
      if (muons.size()!=2)
      {
        LogError("Replacer") << "the decay mode Z->tautau requires exactly two muons, aborting processing";
        return;
      }

      targetParticleMass_  = 1.77690;
      targetParticlePdgID_ = 15;
      
      reco::Muon muon1 = muons.at(0);
      reco::Muon muon2 = muons.at(1);
      reco::Particle tau1(muon1.charge(), muon1.p4(), muon1.vertex(), muon1.pdgId(), 0, true);
      reco::Particle tau2(muon2.charge(), muon2.p4(), muon2.vertex(), muon2.pdgId(), 0, true);
      transformMuMu2TauNu(&tau1, &tau2);
      particles.push_back(tau1);
      particles.push_back(tau2);                      
      break;
    }  
	}
	
	if (particles.size()==0)
	{
		LogError("Replacer") << "the creation of the new particles failed somehow";
		return;
	}
	else
	{
		LogInfo("Replacer") << particles.size() << " particles found, continue processing";
	}

	/// 3) prepare the event
	if (replacementMode_==0)
	{
			Handle<edm::HepMCProduct> HepMCHandle;
			iEvent.getByLabel(HepMCSource_,HepMCHandle);
		
			evt = new HepMC::GenEvent(*(HepMCHandle->GetEvent()));
	
			for ( GenEvent::vertex_iterator p = evt->vertices_begin(); p != evt->vertices_end(); p++ ) 
			{
				GenVertex * vtx=(*p);
				
				if ( vtx->particles_out_size()<=0 || vtx->particles_in_size()<=0)
					continue;
				
				bool temp_muon1=false,temp_muon2=false,temp_z=false;
				for (GenVertex::particles_out_const_iterator it = vtx->particles_out_const_begin(); it!=vtx->particles_out_const_end(); it++)
				{
					if ((*it)->pdg_id()==13) temp_muon1=true;
					if ((*it)->pdg_id()==-13) temp_muon2=true;
					if ((*it)->pdg_id()==23) temp_z=true;
				}
	
				int mother_pdg=(*vtx->particles_in_const_begin())->pdg_id();
	
				if ((vtx->particles_out_size()==2 && vtx->particles_in_size()>0 
					&& mother_pdg == 23  
					&& temp_muon1
					&& temp_muon2)
				|| (vtx->particles_out_size()>2 && vtx->particles_in_size()>0 
					&& mother_pdg == 23
					&& temp_muon1 && temp_muon2 && temp_z ))
				{
					zvtx=*p;
				}
			}

			cleanEvent(evt, zvtx);

			// prevent a decay of existing particles
			// this is due to a bug in the PythiaInterface that should be fixed in newer versions
			for (GenEvent::particle_iterator it=evt->particles_begin();it!=evt->particles_end();it++)
				(*it)->set_status(0);

			for (std::vector<reco::Particle>::const_iterator it=particles.begin();it!=particles.end();it++)
			{
				zvtx->add_particle_out(new HepMC::GenParticle((FourVector)it->p4(), it->pdgId(), 1, Flow(), Polarization(0,0)));
			}
	}

	// new product with tau decays
	if (replacementMode_==1)
	{
		reco::Particle::LorentzVector mother_particle_p4;
		for (std::vector<reco::Particle>::const_iterator it=particles.begin();it!=particles.end();it++)
			mother_particle_p4+=it->p4();

		reco::Particle::Point production_point = particles.begin()->vertex();
		GenVertex * decayvtx = new GenVertex(FourVector(production_point.x(),production_point.y(),production_point.z(),0));

		HepMC::GenParticle * mother_particle = new HepMC::GenParticle((FourVector)mother_particle_p4, motherParticleID_, (generatorMode_==0 ? 3 : 2), Flow(), Polarization(0,0));

		decayvtx->add_particle_in(mother_particle);
		
		evt = new HepMC::GenEvent();

		for (std::vector<reco::Particle>::const_iterator it=particles.begin();it!=particles.end();it++)
		{
			decayvtx->add_particle_out(new HepMC::GenParticle((FourVector)it->p4(), it->pdgId(), 1, Flow(), Polarization(0,0)));			
		}
		evt->add_vertex(decayvtx);
	}
	repairBarcodes(evt);
		
	HepMC::GenEvent * retevt = 0;

	/// 3) process the event
	int nr_of_trials=0;
	while(nr_of_trials<1000)
	{
		nr_of_trials++;
		
		if (generatorMode_==0)	// Pythia
		{
			LogError("Replacer") << "Pythia is currently not supported!";
			retevt=evt;
		}

		if (generatorMode_==1)	// TAUOLA
			retevt=tauola_.decay(evt);

		if (retevt==0)
		{
			LogError("Replacer") << "The new event could not be created due to some problems!";
			return;
		}
	
		if (testEvent(retevt))
			break;
	}
	if (nr_of_trials==1000)
	{
		LogError("Replacer") << "failed to create an event where the visible momenta exceed "<< minVisibleTransverseMomentum_ << " GeV ";
		attempts=-1;
		outTree->Fill();
		return;
	}
	attempts=nr_of_trials;
	outTree->Fill();	

	// recover the status codes
	if (replacementMode_==0)
	{
		for (GenEvent::particle_iterator it=retevt->particles_begin();it!=retevt->particles_end();it++)
		{
			if ((*it)->end_vertex())
				(*it)->set_status(2);
			else
				(*it)->set_status(1);
		}
	}

	auto_ptr<HepMCProduct> bare_product(new HepMCProduct()); 
	if (printEvent_)
		retevt->print(std::cout);

	bare_product->addHepMCData(retevt);
	iEvent.put(bare_product);

	delete part1;
	delete part2;
	delete zvtx;
	delete evt;
	return;
}

// ------------ method called once each job just before starting event loop  ------------
void ParticleReplacerClass::beginRun(edm::Run& iRun,const edm::EventSetup& iSetup)
{
	tauola_.init(iSetup);
}

// ------------ method called once each run  ------------
void ParticleReplacerClass::beginJob() { }


// ------------ method called once each job just after ending the event loop  ------------
void 
ParticleReplacerClass::endJob()
{
	tauola_.statistics();
}

bool ParticleReplacerClass::testEvent(HepMC::GenEvent * evt)
{
	using namespace HepMC;
	
	if (minVisibleTransverseMomentum_<=0)
		return true;
		
  for (GenEvent::particle_iterator it=evt->particles_begin();it!=evt->particles_end();it++)
	{
		if (abs((*it)->pdg_id())==15 && (*it)->end_vertex())
    {
    	FourVector vis_mom();
    	math::PtEtaPhiMLorentzVector visible_momentum;
    	std::queue<const GenParticle *> decaying_particles;
			decaying_particles.push(*it);
			int t=0;
			while(!decaying_particles.empty() && (++t < 30))
			{
				const GenParticle * front = decaying_particles.front();
	    	decaying_particles.pop();

				if (!front->end_vertex())
				{
					int pdgId=abs(front->pdg_id());
					if (pdgId>10 && pdgId!=12 && pdgId!=14 && pdgId!=16)
						visible_momentum+=(math::PtEtaPhiMLorentzVector)front->momentum();
				}
				else
				{
					GenVertex * temp_vert = front->end_vertex();
					for (GenVertex::particles_out_const_iterator it2=temp_vert->particles_out_const_begin();it2!=temp_vert->particles_out_const_end();it2++)
						decaying_particles.push((*it2));
				}

    		//delete temp_vert;
    	}
    	if (visible_momentum.pt()<minVisibleTransverseMomentum_)
    	{
	   		LogInfo("Replacer") << "refusing the event as the sum of the visible transverse momenta is too small: " << visible_momentum.pt() << "\n";
    		return false;
    	}
		}
	}
	return true;
}

void ParticleReplacerClass::cleanEvent(HepMC::GenEvent * evt, HepMC::GenVertex * vtx)
{
	using namespace HepMC;
	using namespace std;
	using namespace edm;
	using namespace reco;

	stack<HepMC::GenParticle *> deleteParticle;
	
	stack<GenVertex *> deleteVertex;
	stack<GenVertex *> queueVertex;

	if (vtx->particles_out_size()>0)
	{
		for (GenVertex::particles_out_const_iterator it=vtx->particles_out_const_begin();it!=vtx->particles_out_const_end();it++)
		{
			deleteParticle.push(*it);
			if ((*it)->end_vertex())
				queueVertex.push((*it)->end_vertex());
		}
	}

	while (!queueVertex.empty())
	{
		GenVertex * temp_vtx=queueVertex.top();
		if (temp_vtx->particles_out_size()>0)
		{
			for (GenVertex::particles_out_const_iterator it=temp_vtx->particles_out_const_begin();it!=temp_vtx->particles_out_const_end();it++)
			{
				if ((*it)->end_vertex())
					queueVertex.push((*it)->end_vertex());
			}
			delete temp_vtx;
		}
		deleteVertex.push(queueVertex.top());
		queueVertex.pop();
	}
	
	while (!deleteVertex.empty())
	{
		evt->remove_vertex(deleteVertex.top());
		deleteVertex.pop();
	}

	while (!deleteParticle.empty())
	{
		delete vtx->remove_particle(deleteParticle.top());
		deleteParticle.pop();
	}

	while (!deleteVertex.empty())
		deleteVertex.pop();
	while (!queueVertex.empty())
		queueVertex.pop();

	repairBarcodes(evt);
}

void ParticleReplacerClass::repairBarcodes(HepMC::GenEvent * evt)
{
	using namespace HepMC;

	// repair the barcodes
	int max_barc=0;
	for (GenEvent::vertex_iterator it=evt->vertices_begin();it!=evt->vertices_end();it++)
		while (!(*it)->suggest_barcode(-1*(++max_barc)))
			;

	max_barc=0;
	for (GenEvent::particle_iterator it=evt->particles_begin();it!=evt->particles_end();it++)
		while (!(*it)->suggest_barcode(++max_barc))
			;
}

///	transform a muon pair into a tau pair
void ParticleReplacerClass::transformMuMu2TauTau(reco::Particle * muon1, reco::Particle * muon2)
{
	using namespace edm;
	using namespace reco;
	using namespace std;
	
	reco::Particle::LorentzVector muon1_momentum = muon1->p4();
	reco::Particle::LorentzVector muon2_momentum =  muon2->p4();
	reco::Particle::LorentzVector z_momentum = muon1_momentum + muon2_momentum;

	ROOT::Math::Boost booster(z_momentum.BoostToCM());
	ROOT::Math::Boost invbooster(booster.Inverse());
	
	reco::Particle::LorentzVector Zb = booster(z_momentum);

	reco::Particle::LorentzVector muon1b = booster(muon1_momentum);
	reco::Particle::LorentzVector muon2b = booster(muon2_momentum);
	
	double tau_mass2 = targetParticleMass_*targetParticleMass_;

	double muonxb_mom2 = muon1b.x()*muon1b.x() + muon1b.y()*muon1b.y() + muon1b.z() * muon1b.z();
	double tauxb_mom2 = 0.25 * Zb.t() * Zb.t() - tau_mass2;

	float scaling1 = sqrt(tauxb_mom2 / muonxb_mom2);
	float scaling2 = scaling1;

	float tauEnergy= Zb.t() / 2.;

	if (tauEnergy*tauEnergy<tau_mass2)
		return;
	
	reco::Particle::LorentzVector tau1b_mom = reco::Particle::LorentzVector(scaling1*muon1b.x(),scaling1*muon1b.y(),scaling1*muon1b.z(),tauEnergy);
	reco::Particle::LorentzVector tau2b_mom = reco::Particle::LorentzVector(scaling2*muon2b.x(),scaling2*muon2b.y(),scaling2*muon2b.z(),tauEnergy);

	// some checks
	// the following test guarantees a deviation
	// of less than 0.1% for phi and theta for the
	// original muons and the placed taus
	// (in the centre-of-mass system of the z boson)
	assert((muon1b.phi()-tau1b_mom.phi())/muon1b.phi()<0.001);
	assert((muon2b.phi()-tau2b_mom.phi())/muon2b.phi()<0.001);
	assert((muon1b.theta()-tau1b_mom.theta())/muon1b.theta()<0.001);
	assert((muon2b.theta()-tau2b_mom.theta())/muon2b.theta()<0.001);	

	reco::Particle::LorentzVector tau1_mom = (invbooster(tau1b_mom));
	reco::Particle::LorentzVector tau2_mom = (invbooster(tau2b_mom));
	
	// some additional checks
	// the following tests guarantee a deviation of less
	// than 0.1% for the following values of the original
	// muons and the placed taus
	//	invariant mass
	//	transverse momentum
	assert(((muon1_momentum+muon1_momentum).mass()-(tau1_mom+tau2_mom).mass())/(muon1_momentum+muon1_momentum).mass()<0.001);
	assert(((muon1_momentum+muon2_momentum).pt()-(tau1_mom+tau2_mom).pt())/(muon1_momentum+muon1_momentum).pt()<0.001);

	muon1->setP4(tau1_mom);
	muon2->setP4(tau2_mom);

	muon1->setPdgId(targetParticlePdgID_*muon1->pdgId()/abs(muon1->pdgId()));
	muon2->setPdgId(targetParticlePdgID_*muon2->pdgId()/abs(muon2->pdgId()));

	muon1->setStatus(1);
	muon2->setStatus(1);

	return;
}
///     transform a muon pair into tau nu (as coming from a W boson)
void ParticleReplacerClass::transformMuMu2TauNu(reco::Particle * part1, reco::Particle * part2)
{
	using namespace edm;
	using namespace reco;
	using namespace std;

	reco::Particle::LorentzVector muon1_momentum = part1->p4();
	reco::Particle::LorentzVector muon2_momentum =  part2->p4();
	reco::Particle::LorentzVector z_momentum = muon1_momentum + muon2_momentum;

	ROOT::Math::Boost booster(z_momentum.BoostToCM());
	ROOT::Math::Boost invbooster(booster.Inverse());

	reco::Particle::LorentzVector Zb = booster(z_momentum);

	const double breitWignerWidth_Z = 2.4952;
	const double breitWignerWidth_W = 2.141;
	const double knownMass_W = 80.398;
	const double knownMass_Z = 91.1876;
		      
	double Wb_mass = ( Zb.mass() - knownMass_Z ) * ( breitWignerWidth_W / breitWignerWidth_Z ) + knownMass_W;
	std::cout << "Wb_mass: " << Wb_mass << "\n";

	reco::Particle::LorentzVector muon1b = booster(muon1_momentum);
	reco::Particle::LorentzVector muon2b = booster(muon2_momentum);

	double tau_mass2 = targetParticleMass_*targetParticleMass_;

	double muonxb_mom2 = muon1b.x()*muon1b.x() + muon1b.y()*muon1b.y() + muon1b.z() * muon1b.z();
	double tauxb_mom2 = 0.25 * Zb.t() * Zb.t() - tau_mass2;

	float scaling1 = sqrt(tauxb_mom2 / muonxb_mom2) * Wb_mass/Zb.mass();
	float scaling2 = scaling1;

	float tauEnergy= Zb.t() / 2.;

	if (tauEnergy*tauEnergy<tau_mass2)
		      return;

	reco::Particle::LorentzVector tau1b_mom = reco::Particle::LorentzVector(scaling1*muon1b.x(),scaling1*muon1b.y(),scaling1*muon1b.z(),tauEnergy* Wb_mass/Zb.mass());
	reco::Particle::LorentzVector tau2b_mom = reco::Particle::LorentzVector(scaling2*muon2b.x(),scaling2*muon2b.y(),scaling2*muon2b.z(),tauEnergy* Wb_mass/Zb.mass());

	std::cout << "muon1b_momentum: " << muon1b << "\n";
	std::cout << "muon2b_momentum: " << muon2b << "\n";

	std::cout << "tau1b_momentum: " << tau1b_mom << "\n";
	std::cout << "tau2b_momentum: " << tau2b_mom << "\n";

	std::cout << "zb_momentum: " << Zb << "\n";
	std::cout << "wb_momentum: " << (tau1b_mom+tau2b_mom) << "\n";
		              
	// some checks
	// the following test guarantees a deviation
	// of less than 0.1% for phi and theta for the
	// original muons and the placed taus
	// (in the centre-of-mass system of the z boson)
	assert((muon1b.phi()-tau1b_mom.phi())/muon1b.phi()<0.001);
	assert((muon2b.phi()-tau2b_mom.phi())/muon2b.phi()<0.001);
	assert((muon1b.theta()-tau1b_mom.theta())/muon1b.theta()<0.001);
	assert((muon2b.theta()-tau2b_mom.theta())/muon2b.theta()<0.001);        

	reco::Particle::LorentzVector tau1_mom = (invbooster(tau1b_mom));
	reco::Particle::LorentzVector tau2_mom = (invbooster(tau2b_mom));

	// some additional checks
	// the following tests guarantee a deviation of less
	// than 0.1% for the following values of the original
	// muons and the placed taus
	//      invariant mass
	//      transverse momentum
	//assert(((muon1_momentum+muon1_momentum).mass()-(tau1_mom+tau2_mom).mass())/(muon1_momentum+muon1_momentum).mass()<0.001);
	//assert(((muon1_momentum+muon2_momentum).pt()-(tau1_mom+tau2_mom).pt())/(muon1_momentum+muon1_momentum).pt()<0.001);

	part1->setP4(tau1_mom);
	part2->setP4(tau2_mom);

	part1->setPdgId(15*part1->pdgId()/abs(part1->pdgId()));
	part2->setPdgId(16*part2->pdgId()/abs(part2->pdgId()));

	part1->setStatus(1);
	part2->setStatus(1);

	return;
}


//define this as a plug-in
//DEFINE_FWK_MODULE(Replacer);
