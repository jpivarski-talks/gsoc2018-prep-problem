

# pip install oamap (to view the columnar data as objects)
import oamap.source.root

# pip install uproot (to read the data files)
import uproot

# in this package (to make Python lists and OAMap ListProxies functional)
import functional

[1, 2, 3, 4, 5].map(lambda x: x * 10)

[1, 2, 3, 4, 5].reduce(lambda x, y: x + y)

[1, 2, 3, 4, 5].size

[[1, 2], [3, 4, 5]].flatten

["a", "b", "c", "d", "e"].enumerate

[1, 2, 3, 4, 5].pairs(lambda x, y: [x, y])

([1, 2, 3, 4, 5], ["a", "b", "c", "d", "e"]).table(lambda x, y: [x, y])

g1 = [1, 2, 3, 4, 5].lazy
g1

g2 = g1.map(lambda x: x * 10)
g2

g2.collect

def makeinfinite():
    i = 0
    while True:
        yield i
        i += 1

infinite = makeinfinite()
infinite

infinite.take(10)

infinite.map(lambda x: x * 10).take(10)

infinite.size
infinite.reduce(lambda x, y: x + y)

# get the data (lazily, one column at a time)
events = uproot.open("http://scikit-hep.org/uproot/examples/HZZ.root")["events"].oamap()

# replace some names, to be more natural in a functional context
events.schema.content.rename("NElectron", "electrons")
events.schema.content["electrons"].content.rename("Electron_Px", "px")
events.schema.content["electrons"].content.rename("Electron_Py", "py")
events.schema.content["electrons"].content.rename("Electron_Pz", "pz")
events.schema.content["electrons"].content.rename("Electron_E", "energy")
events.schema.content["electrons"].content.rename("Electron_Iso", "isolation")
events.schema.content["electrons"].content.rename("Electron_Charge", "charge")
events.schema.content.rename("NMuon", "muons")
events.schema.content["muons"].content.rename("Muon_Px", "px")
events.schema.content["muons"].content.rename("Muon_Py", "py")
events.schema.content["muons"].content.rename("Muon_Pz", "pz")
events.schema.content["muons"].content.rename("Muon_E", "energy")
events.schema.content["muons"].content.rename("Muon_Iso", "isolation")
events.schema.content["muons"].content.rename("Muon_Charge", "charge")
events.schema.content.rename("NPhoton", "photons")
events.schema.content["photons"].content.rename("Photon_Px", "px")
events.schema.content["photons"].content.rename("Photon_Py", "py")
events.schema.content["photons"].content.rename("Photon_Pz", "pz")
events.schema.content["photons"].content.rename("Photon_E", "energy")
events.schema.content["photons"].content.rename("Photon_Iso", "isolation")
events.schema.content.rename("NJet", "jets")
events.schema.content["jets"].content.rename("Jet_Px", "px")
events.schema.content["jets"].content.rename("Jet_Py", "py")
events.schema.content["jets"].content.rename("Jet_Pz", "pz")
events.schema.content["jets"].content.rename("Jet_E", "energy")
events.schema.content["jets"].content.rename("Jet_ID", "id")
events.schema.content["jets"].content.rename("Jet_btag", "btag")
events.regenerate()

from math import *

def mass(*particles):
    px = particles.map(lambda particle: particle.px).sum
    py = particles.map(lambda particle: particle.py).sum
    pz = particles.map(lambda particle: particle.pz).sum
    energy = particles.map(lambda particle: particle.energy).sum
    return sqrt(energy**2 - px**2 - py**2 - pz**2)

masses = (events
          .lazy
          .filter(lambda event: event.muons.size >= 2)
          .map(lambda event: event.muons.pairs(mass))
          .flatten)

masses.take(100)

(events
 .lazy
 .filter(lambda event: event.electrons.size == 2 and event.muons.size == 2)
 .map(lambda event: (mass(*event.electrons), mass(*event.muons)))
 ).take(10)

