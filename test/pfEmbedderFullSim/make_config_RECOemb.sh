#!/bin/bash
#CONDITIONS=FrontierConditions_GlobalTag,DESIGN_38_V10::All , Problematic
CONDITIONS=FrontierConditions_GlobalTag,START38_V10::All




if [ 1 -eq 2 ]; then

cmsDriver.py ZMM_7TeV_cfi \
       -s GEN:ProductionFilterSequence,SIM,DIGI,L1,DIGI2RAW,HLT \
       --no_exec \
       -n 10 \
       --conditions=${CONDITIONS} \
       --fileout=Zmumu_HLT.root  \
       --python_filename=Zmumu_HLT.py \
       --eventcontent FEVTDEBUGHLT


cmsDriver.py \
       -s RAW2DIGI,RECO \
       --no_exec \
       -n -1 \
       --conditions=${CONDITIONS} \
       --fileout=Zmumu_RECO.root \
       --filein=file:Zmumu_HLT.root \
       --python_filename=Zmumu_RECO.py \
       --eventcontent RECO


fi
 
cmsDriver.py TauAnalysis/MCEmbeddingTools/python/RECOEmbeddingSource_cff.py \
       -s GEN:ProductionFilterSequence,SIM,DIGI,L1,DIGI2RAW,HLT \
       --no_exec \
       -n -1 \
       --conditions=${CONDITIONS} \
       --filein=file:Zmumu_RECO.root \
       --fileout=embedded_HLT.root  \
       --python_filename=embedReco_HLT.py \
       --customise=TauAnalysis/MCEmbeddingTools/reco_00_customizeHLT.py



cmsDriver.py \
       -s RAW2DIGI,RECO \
       --no_exec \
       -n -1 \
       --conditions=${CONDITIONS} \
       --fileout=embedded_RECO.root \
       --filein=file:embedded_HLT.root \
       --python_filename=embedReco_RECO.py \
       --customise=TauAnalysis/MCEmbeddingTools/reco_01_customizeRECO.py

