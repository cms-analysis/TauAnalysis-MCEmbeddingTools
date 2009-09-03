#include "TauAnalysis/MCEmbeddingTools/interface/ParticleReplacerFactory.h"
#include "TauAnalysis/MCEmbeddingTools/interface/ParticleReplacerClass.h"

boost::shared_ptr<ParticleReplacerBase> ParticleReplacerFactory::create(int algo, const edm::ParameterSet& iConfig) {
  if(algo == 1)
    return boost::shared_ptr<ParticleReplacerBase>(new ParticleReplacerClass(iConfig));
  else 
    throw cms::Exception("Configuration") << "Unknown particle replacer algorithm " << algo
                                          << ". Supported algorithms: 1 for ParticleReplacerClass" << std::endl;
}
