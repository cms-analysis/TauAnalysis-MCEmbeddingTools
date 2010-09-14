// -*- C++ -*-
//
// Package:    PrintProductID
// Class:      PrintProductID
// 
/**\class PrintProductID PrintProductID.cc MyAna/PrintProductID/src/PrintProductID.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Tomasz Maciej Frueboes
//         Created:  Fri Dec 18 14:29:14 CET 2009
// $Id: PrintProductID.cc,v 1.1 2010/03/17 16:14:10 fruboes Exp $
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include <DataFormats/ParticleFlowCandidate/interface/PFCandidate.h>
#include <DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h>
//
// class declaration
//

class PrintProductID : public edm::EDFilter {
   public:
      explicit PrintProductID(const edm::ParameterSet&);
      ~PrintProductID();

   private:
      virtual void beginJob() ;
      virtual bool filter(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      
      // ----------member data ---------------------------
      int _procInd;
      int _prodInd;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
PrintProductID::PrintProductID(const edm::ParameterSet& iConfig):
  _procInd(iConfig.getUntrackedParameter<int>("process")),
  _prodInd(iConfig.getUntrackedParameter<int>("product"))
{

}


PrintProductID::~PrintProductID()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

// ------------ method called on each new Event  ------------
bool
PrintProductID::filter(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;

  ProductID pID(_procInd,_prodInd);

  edm::Provenance prov = iEvent.getProvenance(pID);

  edm::InputTag tag(prov.moduleLabel(),  	 
                    prov.productInstanceName(),  	 
                    prov.processName());

  std::cout << "PrintProductID " << tag << std::endl;


  return true;
}

// ------------ method called once each job just before starting event loop  ------------
void 
PrintProductID::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
PrintProductID::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(PrintProductID);
