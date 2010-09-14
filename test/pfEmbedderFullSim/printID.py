import FWCore.ParameterSet.Config as cms

process = cms.Process('TTT')


process.source = cms.Source("PoolSource",
   # fileNames = cms.untracked.vstring('file:embedded_HLT.root')
    fileNames = cms.untracked.vstring('file:Zmumu_RECO_allProds.root')
)



process.pr = cms.EDFilter("PrintProductID",
  process = cms.untracked.int32(2),
  product = cms.untracked.int32(229)
)


process.t = cms.Path(process.pr)
