# -*- coding: utf-8 -*-

from FWCore.ParameterSet.Modules import _Module


"replaces target with modOrgRenamed*target"
def placeMixer(process, target, modOrgRenamed):
   temp = target.clone()
   # for some reason siPixelClustersRenamed gets placed several times in same sequence if this is done in one loop
   for s in process.sequences:
     seq = getattr(process,s)
     seq.replace(target, modOrgRenamed*temp)
   for s in process.sequences:
     seq = getattr(process,s)
     seq.replace(temp, target)


def customise(process):
  
  process._Process__name="RECO2"
  
  process.TFileService = cms.Service("TFileService",  fileName = cms.string("histo.root")          )


  """
  "For the moment track mixer is incomplete"
  "Place RecoTracksMixer in the sequences" 
  process.generalTracksRenamed = process.generalTracks.clone()
  process.generalTracks = cms.EDProducer("RecoTracksMixer",
      trackCol1 = cms.InputTag("dimuonsGlobal"),
      trackCol2 = cms.InputTag("generalTracksRenamed","","RECO2"),
      doExtra = cms.untracked.bool(True)
  )  
  placeMixer(process, process.generalTracks,  process.generalTracksRenamed)
  """
  
  process.rpcRecHitsRenamed = process.rpcRecHits.clone()
  process.rpcRecHits = cms.EDProducer("RPCRecHitMixer",
      recHitsZmumu  = cms.InputTag("rpcRecHits","","RECO"),
      recHitsTauTau = cms.InputTag("rpcRecHitsRenamed"),
      selectedMuons = cms.InputTag("adaptedMuonsFromPFCands","zMusExtracted"),
      allMuons = cms.InputTag("muons", "", "RECO")
  )
  
  placeMixer(process, process.rpcRecHits,  process.rpcRecHitsRenamed)
  
  """
  print "Removing doAlldEdXEstimators sequence"
  for s in process.sequences:
    seq =  getattr(process,s)
    seq.remove(process.doAlldEdXEstimators)
  """

  
  print "Changing eventcontent to AODSIM + misc "
  process.output.outputCommands = process.AODSIMEventContent.outputCommands
  keepMC = cms.untracked.vstring("keep *_*_zMusExtracted_*",
                                 "keep *_dimuonsGlobal_*_*",
                                 'keep *_generator_*_*'
  )
  process.output.outputCommands.extend(keepMC)


  return(process)
