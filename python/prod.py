#!/usr/bin/env python


import os
import sys

ver="20120912EmbTF"

decModes = { 115:"elec1_17had1_17"
             116:"mu1_13had1_17",
             121:"elec1_17elec2_8",
             122:"mu1_18mu2_8",
             123:"tau1_18tau2_8",
             132:"tau1_17had2_17",
           }


#

datasets={ "/DoubleMu/aburgmei-DoubleMu_Run2012A_13Jul2012_v1_Zmumu_skim_v1-f456bdbb960236e5c696adfe9b04eaae/USER"
                        : "BeamSpotObjects_2009_LumiBased_SigmaZ_v27_offline", 
           "/DoubleMu/aburgmei-DoubleMu_Run2012B_13Jul2012_v4_Zmumu_skim_v1-f456bdbb960236e5c696adfe9b04eaae/USER"
                        :  "BeamSpotObjects_2009_LumiBased_SigmaZ_v27_offline" ,
           "/DoubleMu/aburgmei-DoubleMu_Run2012C_PromptReco_v1_Zmumu_skim_v1-f456bdbb960236e5c696adfe9b04eaae/USER"
                        : "BeamSpotObjects_PCL_byLumi_v0_prompt" ,
           "/DoubleMu/aburgmei-DoubleMu_Run2012C_PromptReco_v2_Zmumu_skim_v1-f456bdbb960236e5c696adfe9b04eaae/USER"
                        : "BeamSpotObjects_PCL_byLumi_v0_prompt"
         }

def getFile(ds):
     print "#"*30
     print "#"*30
     print "#"*30
     print "Getting", ds
     command="~/util/filemoverv1.csh "+ ds
     #os.system(command)
     sys.stdout.flush()


def createJob(mode,minPT,ds):
      name=ver+"_embedded_trans1_tau"+str(mode)+"_" + minPT
      print name, ds
      command="crab -USER.ui_working_dir="+name
      command+=" -create "
      #command+=" -submit "
      command+=" -USER.publish_data_name="+name
      command+=" -CMSSW.datasetpath="+ds


      print "XXXXXXX setting", str(mode), str(minPT)
      sys.stdout.flush()

      os.environ["mdtau"]=str(mode)
      os.environ["minpt"]=str(minPT)
      os.environ["beamspot"]=str(datasets[ds])
      #os.system(command)
      #sys.stdout.flush()

      c1 = "python embed.py"
      os.system(c1)


def main():

  #for ds in datasets:
  #   getFile(ds)

# '''
  for ds in datasets:
    for mode in decModes:
       createJob(mode, decModes[mode], ds)
#'''





if __name__ == "__main__":
     main()

