// -*- C++ -*-
//
// Package:    RPCRecHitMixer
// Class:      RPCRecHitMixer
// 
/**\class RPCRecHitMixer RPCRecHitMixer.cc TauAnalysis/RPCRecHitMixer/src/RPCRecHitMixer.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Tomasz Fruboes
//         Created:  Fri Sep 10 14:22:06 CEST 2010
// $Id$
//
//


// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/RPCRecHit/interface/RPCRecHitCollection.h"
#include <DataFormats/MuonReco/interface/Muon.h>

#include <DataFormats/TrackingRecHit/interface/TrackingRecHitFwd.h>
#include "DataFormats/TrackReco/interface/Track.h"
#include <DataFormats/Math/interface/deltaR.h>

using namespace edm;
using namespace std;


//
// class declaration
//

class RPCRecHitMixer : public edm::EDProducer {
   public:
      explicit RPCRecHitMixer(const edm::ParameterSet&);
      ~RPCRecHitMixer();

   private:
      virtual void beginJob() ;
      virtual void produce(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      bool recHitNotFromSelectedMuon(RPCRecHitCollection::const_iterator itRPC);
      InputTag colZmumu_;
      InputTag colTauTau_;
      InputTag selectedMuons_;
      InputTag allMuons_;
      
      // selected muons just a copy of some muons (not exact) may be missing some parts (e.g. rechits reference)
      // matching to muons collection is needed
      Handle< std::vector< reco::Muon > > selectedMuonsHandle;
      Handle< std::vector< reco::Muon > >      allMuonsHandle;

      
      // ----------member data ---------------------------
};

//
// constructors and destructor
//
RPCRecHitMixer::RPCRecHitMixer(const edm::ParameterSet& iConfig):
  colZmumu_(iConfig.getParameter<InputTag>("recHitsZmumu")),
  colTauTau_(iConfig.getParameter<InputTag>("recHitsTauTau")),
  selectedMuons_(iConfig.getParameter<edm::InputTag>("selectedMuons")),
  allMuons_(iConfig.getParameter<edm::InputTag>("allMuons"))
{

  produces<RPCRecHitCollection>();
  
}


RPCRecHitMixer::~RPCRecHitMixer()
{

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
RPCRecHitMixer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   auto_ptr<RPCRecHitCollection> recHitCollection(new RPCRecHitCollection());

    
   // key - rawDetID
   std::map< uint32_t, edm::OwnVector<RPCRecHit> > recHitMap;

   // TODO - check validity of colls 
   Handle<RPCRecHitCollection> rpcHitsZmumu; // need cleaning
   iEvent.getByLabel(colZmumu_,rpcHitsZmumu);

   Handle<RPCRecHitCollection> rpcHitsTauTau; // take as it is
   iEvent.getByLabel(colTauTau_,rpcHitsTauTau);

   //get selected muons, needed for rechit filtering
   iEvent.getByLabel(selectedMuons_, selectedMuonsHandle);
   iEvent.getByLabel(allMuons_, allMuonsHandle);
   
   


   RPCRecHitCollection::const_iterator  itRPC;
   
   for (itRPC = rpcHitsZmumu->begin(); itRPC != rpcHitsZmumu->end() ; ++itRPC) {
     if ( recHitNotFromSelectedMuon(itRPC) ) { 
       RPCRecHit *newHit = new RPCRecHit(*itRPC);
       recHitMap[itRPC->rpcId().rawId()].push_back(newHit);
     }
   }

   for (itRPC = rpcHitsTauTau->begin(); itRPC != rpcHitsTauTau->end() ; ++itRPC) {
     RPCRecHit *newHit = new RPCRecHit(*itRPC);
     recHitMap[itRPC->rpcId().rawId()].push_back(newHit);
   }


 
   std::map<uint32_t, edm::OwnVector<RPCRecHit> >::iterator it = recHitMap.begin();
   std::map<uint32_t, edm::OwnVector<RPCRecHit> >::iterator itE = recHitMap.end();
   for (;it!=itE;++it){
      if (it->second.size()>0) // not really needed
        recHitCollection->put(it->first, it->second.begin(), it->second.end());
   }


   iEvent.put(recHitCollection);

}


// TODO - optimize. Checking rechit by rechit is not good idea (matching should be done once)
bool RPCRecHitMixer::recHitNotFromSelectedMuon(RPCRecHitCollection::const_iterator itRPC){

  double minDR = -1;  
  const std::vector< reco::Muon > * toBeAdded = selectedMuonsHandle.product();
  const std::vector< reco::Muon > * allMu = allMuonsHandle.product();

  std::vector< reco::Muon >::const_iterator closestMu = allMu->end();
  
  std::vector< reco::Muon >::const_iterator itSelectedMu = toBeAdded->begin();
  std::vector< reco::Muon >::const_iterator itSelectedMuE = toBeAdded->end();
  for (; itSelectedMu != itSelectedMuE; ++itSelectedMu ){
    std::vector< reco::Muon >::const_iterator itAllMu = allMu->begin();
    std::vector< reco::Muon >::const_iterator itAllMuE = allMu->end();
    for ( ;itAllMu != itAllMuE; ++itAllMu) {
       double dr = reco::deltaR( *itAllMu, *itSelectedMu);
       if (dr < minDR) {
          minDR = dr;
          closestMu = itAllMu;
       }
    }

    if (minDR < 0.001) {
     // TODO(?):  use nonglobal muons ?
     if (closestMu->isGlobalMuon()) {
       for ( trackingRecHit_iterator  rh = closestMu->globalTrack()->recHitsBegin() ;
                                      rh!= closestMu->globalTrack()->recHitsEnd ();
                                    ++rh )
       {
          const TrackingRecHit *hit = rh->get();
          const  RPCRecHit *  rpcRH = dynamic_cast <const RPCRecHit * > ( hit );
          if ( rpcRH != 0) {
             if ( *rpcRH == *itRPC) {
               return false; // rpc rechit from selected muon
               std::cout << "Skipping rpc rechit from muon\n";
             }
          }

       }
     } else {
        std::cout << "XXX matched muon is not global muon\n";
     }

    } else {
      std::cout << "XXX No muon match\n";

    }
  }

  
  return true;
}

// ------------ method called once each job just before starting event loop  ------------
void 
RPCRecHitMixer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
RPCRecHitMixer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(RPCRecHitMixer);
