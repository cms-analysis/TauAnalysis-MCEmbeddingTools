// -*- C++ -*-
//
// Package:    RecoTracksMixer
// Class:      RecoTracksMixer
// 
/**\class RecoTracksMixer RecoTracksMixer.cc TauAnalysis/RecoTracksMixer/src/RecoTracksMixer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Tomasz Maciej Frueboes
//         Created:  Fri Apr  9 12:15:56 CEST 2010
// $Id: RecoTracksMixer.cc,v 1.2.2.1 2010/09/13 10:03:33 fruboes Exp $
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
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "TrackingTools/PatternTools/interface/Trajectory.h"
#include "TrackingTools/PatternTools/interface/TrajTrackAssociation.h"

#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
 

//
// class decleration
//

class RecoTracksMixer : public edm::EDProducer {
   public:
      explicit RecoTracksMixer(const edm::ParameterSet&);
      ~RecoTracksMixer();

   private:
      virtual void beginJob() ;
      virtual void produce(edm::Event&, const edm::EventSetup&);
      virtual void endJob() ;
      std::auto_ptr<reco::TrackExtraCollection> outputTrkExtras;
      reco::TrackExtraRefProd refTrkExtras;
      
      std::auto_ptr< std::vector<Trajectory> > outputTrajs;
      std::auto_ptr< TrajTrackAssociationCollection >  outputTTAss;
      
      std::auto_ptr< TrajectorySeedCollection > outputSeeds;
      edm::RefProd< TrajectorySeedCollection > refTrajSeeds;


      
//       std::auto_ptr< TrackingRecHitCollection>  outputTrkHits;
//       TrackingRecHitRefProd refTrkHits;


      
      edm::InputTag _tracks1;
      edm::InputTag _tracks2;
      bool _doExtra;
      
      // ----------member data ---------------------------
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
RecoTracksMixer::RecoTracksMixer(const edm::ParameterSet& iConfig) :
  _tracks1(iConfig.getParameter< edm::InputTag > ("trackCol1")),
  _tracks2(iConfig.getParameter< edm::InputTag > ("trackCol2")),
  _doExtra(iConfig.getUntrackedParameter< bool > ("doExtra", false) )
{
  
   produces<reco::TrackCollection>();
   
   if(_doExtra){
     produces<reco::TrackExtraCollection>();
     
     produces< TrajTrackAssociationCollection >();
     produces< std::vector<Trajectory> >();
     
     produces<TrajectorySeedCollection>();

     
     
//      produces<TrackingRecHitCollection>();
   }
}


RecoTracksMixer::~RecoTracksMixer()
{
 
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
RecoTracksMixer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
   using namespace edm;
   
   std::vector< edm::Handle<reco::TrackCollection> > cols;
   edm::Handle<reco::TrackCollection> tks1;
   iEvent.getByLabel( _tracks1, tks1);

   edm::Handle<reco::TrackCollection> tks2;
   iEvent.getByLabel( _tracks2, tks2);

   cols.push_back(tks1);
   cols.push_back(tks2);
 
   std::auto_ptr< reco::TrackCollection  > newCol(new reco::TrackCollection );
   
   if(_doExtra){
     outputTrkExtras = std::auto_ptr<reco::TrackExtraCollection>(new reco::TrackExtraCollection);
     refTrkExtras    = iEvent.getRefBeforePut<reco::TrackExtraCollection>();
     
     outputTrajs = std::auto_ptr< std::vector<Trajectory> >(new std::vector<Trajectory>()); 
     outputTTAss = std::auto_ptr< TrajTrackAssociationCollection >(new TrajTrackAssociationCollection());

     outputSeeds = std::auto_ptr<TrajectorySeedCollection>(new TrajectorySeedCollection);
     refTrajSeeds = iEvent.getRefBeforePut<TrajectorySeedCollection>();

     
//      outputTrkHits   = std::auto_ptr<TrackingRecHitCollection>(new TrackingRecHitCollection);
//      refTrkHits      = e.getRefBeforePut<TrackingRecHitCollection>();


   }

   //std::cout << "##########################################\n";
   std::vector< edm::Handle<reco::TrackCollection> >::iterator it = cols.begin();
   for(;it != cols.end(); ++it) 
   {
     //std::cout << " col " << i++ << std::endl;
     for ( reco::TrackCollection::const_iterator itT = (*it)->begin() ; itT != (*it)->end(); ++itT)
     {
       /*
       std::cout << " " << itT->vx()
           << " " << itT->vy()
           << " " << itT->vz()
           << " " << itT->pt()
           << std::endl;*/

       newCol->push_back(*itT);
       
       if(_doExtra){
          edm::RefToBase<TrajectorySeed> origSeedRef = itT->seedRef();
          outputSeeds->push_back( TrajectorySeed(*origSeedRef));
          
          edm::Ref<TrajectorySeedCollection> pureRef(refTrajSeeds, outputSeeds->size()-1);
          origSeedRef=edm::RefToBase<TrajectorySeed>( pureRef);
          
          //outputTrkExtras->push_back(*itT->extra());
          const reco::Track & theTrack = newCol->back();
          outputTrkExtras->push_back( reco::TrackExtra( 
                              theTrack.outerPosition(), theTrack.outerMomentum(), theTrack.outerOk(),
                              theTrack.innerPosition(), theTrack.innerMomentum(), theTrack.innerOk(),
                              theTrack.outerStateCovariance(), theTrack.outerDetId(),
                              theTrack.innerStateCovariance(), theTrack.innerDetId(),
                              theTrack.seedDirection(), origSeedRef ) );
    
          
          
          newCol->back().setExtra( reco::TrackExtraRef( refTrkExtras, outputTrkExtras->size() - 1) );
       }
       
  


       /*
       reco::TrackExtra & tx = outputTrkExtras->back();
       for (int i = 0; i < tx->recHitsSize(); ++i){
         
       
     } */

       
     }

   }
   if(_doExtra){
     iEvent.put(outputTrkExtras);
     iEvent.put(outputTrajs);
     iEvent.put(outputTTAss);
     iEvent.put(outputSeeds);
   }
   iEvent.put(newCol);



   

}

// ------------ method called once each job just before starting event loop  ------------
void 
RecoTracksMixer::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void 
RecoTracksMixer::endJob() {
}

//define this as a plug-in
DEFINE_FWK_MODULE(RecoTracksMixer);
