#!/usr/bin/env python

import uproot

class Particle:
    def __init__(self, px, py, pz, energy):
        self.px = px
        self.py = py
        self.pz = pz
        self.energy = energy

class ChargedParticle(Particle):
    def __init__(self, px, py, pz, energy, charge):
        super(ChargedParticle, self).__init__(px, py, pz, energy)
        self.charge = charge

class Electron(ChargedParticle):
    def __init__(self, px, py, pz, energy, charge, isolation):
        super(Electron, self).__init__(px, py, pz, energy, charge)
        self.isolation = isolation

class Muon(ChargedParticle):
    def __init__(self, px, py, pz, energy, charge, isolation):
        super(Muon, self).__init__(px, py, pz, energy, charge)
        self.isolation = isolation

class Photon(Particle):
    def __init__(self, px, py, pz, energy, isolation):
        super(Photon, self).__init__(px, py, pz, energy)
        self.isolation = isolation

class Jet(Particle):
    def __init__(self, px, py, pz, energy, ID, btag):
        super(Jet, self).__init__(px, py, pz, energy)
        self.ID = ID
        self.btag = btag

class MissingEnergy:
    def __init__(self, x, y):
        self.x = x
        self.y = y

class Event:
    def __init__(self, missingEnergy, electrons, muons, photons, jets):
        


columnarEvents = uproot.open("http://scikit-hep.org/uproot/examples/HZZ.root")["events"].arrays()

events = []
for i in range(columnarEvents.numentries):
    event = 
    electrons = []
    for j in range(columnarEvents["NElectron"][i]):
        electrons.append(Electron(columnarEvents["Electron_Px"][i][j], columnarEvents["Electron_Py"][i][j], columnarEvents["Electron_Pz"][i][j], columnarEvents["Electron_E"][i][j], columnarEvents["Electron_Charge"][i][j], columnarEvents["Electron_Iso"][i][j]))
    
