# -*- coding: utf-8 -*-
import FWCore.ParameterSet.Config as cms

  
from Configuration.Generator.PythiaUESettings_cfi import *

TauolaNoPolar = cms.PSet(
    UseTauolaPolarization = cms.bool(False)
)
TauolaPolar = cms.PSet(
   UseTauolaPolarization = cms.bool(True)
)


from TauAnalysis.MCEmbeddingTools.MCParticleReplacer_cfi import *
newSource.algorithm = "ZTauTau"
newSource.ZTauTau.TauolaOptions.InputCards.mdtau = cms.int32(0)
newSource.ZTauTau.minVisibleTransverseMomentum = cms.untracked.double(0)


source = cms.Source("PoolSource",
        skipEvents = cms.untracked.uint32(0),
        fileNames = cms.untracked.vstring('file:Zmumu_RECO.root')
)

filterEmptyEv = cms.EDFilter("EmptyEventsFilter",
    minEvents = cms.untracked.int32(1),
    target =  cms.untracked.int32(1) 
)

adaptedMuonsFromPFCands = cms.EDProducer("PFMuonAdapter",
    numberOfMuonsToSelect  = cms.untracked.uint32(2),
    pfCands = cms.untracked.InputTag("particleFlow","")
)

dimuonsGlobal = cms.EDProducer('ZmumuPFEmbedder',
    tracks = cms.InputTag("generalTracks"),
    selectedMuons = cms.InputTag("adaptedMuonsFromPFCands","zMusExtracted"),
    keepMuonTrack = cms.bool(False)
)

generator = newSource.clone()
generator.src = cms.InputTag("adaptedMuonsFromPFCands","zMusExtracted")

ProductionFilterSequence = cms.Sequence(adaptedMuonsFromPFCands*dimuonsGlobal*generator*filterEmptyEv)
