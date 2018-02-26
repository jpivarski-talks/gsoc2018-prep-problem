# Problem 1: functional HEP

## Install dependencies

Start by installing uproot (to read HEP data files) and oamap (to interpret columnar data as objects).

```bash
pip install uproot --user
pip install oamap --user
```

(If you prefer to use virtualenv or conda, go ahead and do that.)

## Load them into Python

```python
>>> import oamap.source.root
>>> import uproot
```

## Look at functional.py

This is a toy example of making Python functional. It modifies Python's built-in list and tuple classes to have functional methods like `map` and `reduce`, which is not a safe thing to do in production, but it makes the examples look clear and simple.

Think about the following examples and explain what they're doing.

```python
>>> import functional
>>> [1, 2, 3, 4, 5].map(lambda x: x * 10)
[10, 20, 30, 40, 50]
>>> [1, 2, 3, 4, 5].reduce(lambda x, y: x + y)
15
```

This library has a suite of useful functionals; here are just a couple. For more depth, look at the [functional.py code](functional.py).

```python
>>> [1, 2, 3, 4, 5].size
5
>>> [[1, 2], [3, 4, 5]].flatten
[1, 2, 3, 4, 5]
>>> ["a", "b", "c", "d", "e"].enumerate
[(0, 'a'), (1, 'b'), (2, 'c'), (3, 'd'), (4, 'e')]
>>> [1, 2, 3, 4, 5].pairs(lambda x, y: [x, y])
[[1, 2], [1, 3], [1, 4], [1, 5], [2, 3], [2, 4], [2, 5], [3, 4], [3, 5], [4, 5]]
>>> ([1, 2, 3, 4, 5], ["a", "b", "c", "d", "e"]).table(lambda x, y: [x, y])
[[1, 'a'], [1, 'b'], [1, 'c'], [1, 'd'], [1, 'e'], [2, 'a'], [2, 'b'], [2, 'c'], [2, 'd'], [2, 'e'],
 [3, 'a'], [3, 'b'], [3, 'c'], [3, 'd'], [3, 'e'], [4, 'a'], [4, 'b'], [4, 'c'], [4, 'd'], [4, 'e'],
 [5, 'a'], [5, 'b'], [5, 'c'], [5, 'd'], [5, 'e']]
```

The same applies to generators, which are lazily evaluated lists in Python.

```python
>>> g1 = [1, 2, 3, 4, 5].lazy
>>> g1
<generator object <genexpr> at 0x72ef60e54410>
```

You can add operations to generators without anything being executed right away.

```python
>>> g2 = g1.map(lambda x: x * 10)
>>> g2
<generator object <genexpr> at 0x72ef60e543c0>
```

The opposite of `lazy` is `collect`, which evaluates a generator, turning it into a list.

```python
>>> g2.collect
[10, 20, 30, 40, 50]
```

Lazy generators let us work with infinite lists.

```python
>>> def makeinfinite():
...     i = 0
...     while True:
...         yield i
...         i += 1
...
>>> infinite = makeinfinite()
>>> infinite
<generator object makeinfinite at 0x72ef60e54280>
```

We can't `collect` an infinite list, but we can `take` the first few elements.

```python
>>> infinite.take(10)
[10, 11, 12, 13, 14, 15, 16, 17, 18, 19]
```

And we can apply operations to infinite lists.

```python
>>> infinite.map(lambda x: x * 10).take(10)
[200, 210, 220, 230, 240, 250, 260, 270, 280, 290]
```

But we can never compute anything that applies to all of the elements of an infinite list because that would literally take forever. Thus, the same restriction applies to all lazy generators, because we never know whether a lazy generator is finite or infinite.

```python
>>> infinite.size
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 'generator' object has no attribute 'size'
>>> infinite.reduce(lambda x, y: x + y)
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
AttributeError: 'generator' object has no attribute 'reduce'
```

In experimental physics, we don't actually have any infinite datasets. We do, however, have some very large datasets that we'd like to describe operations on without actually evaluating them. This is the benefit that functional programming would provide: the ability to describe, examine, and manipulate operations that have not yet been performed.

## Get a realistic dataset

The file `HZZ.root` is a very small sample of high energy physics collisions, not real but realistic (simulated by computer). They simulate Higgs bosons (one per event) that each decay into two Z bosons, and each of those decay into two electrons or two muons.

Here's how you can load it:

```python
>>> events = uproot.open("http://scikit-hep.org/uproot/examples/HZZ.root")["events"].oamap()
```

Unfortunately, the names weren't defined in such a way that the objects look natural, so use the following to rename them. In this exercise, we'll assume the new names.

```python
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
```

## Discovering the Higgs

Higgs bosons and Z bosons decay so quickly that they're gone before they ever reach a detector. All we can detect is the electrons and/or muons, and sometimes not even that, if an electron or muon flies past the detector without entering it. How can we tell which electrons or muons came from which Z boson or Higgs?

One powerful technique relies on relativistic mass: Higgs bosons have a (fairly) well-defined mass and Z bosons have a (fairly) well-defined mass (with some variation due to quantum effects, some due to measurement error). When a particle decays, its decay products have the same total energy as the original particle (a scalar number), as well as the same total momentum (a vector: px, py, pz). The equation that relates mass, energy, and momentum is

![](https://wikimedia.org/api/rest_v1/media/math/render/svg/baf9362dce85e812de86351f54ba2727bd2dad82)



# DONE

from math import *

def mass(*particles):
    energy = particles.map(lambda particle: particle.energy).sum
    px = particles.map(lambda particle: particle.px).sum
    py = particles.map(lambda particle: particle.py).sum
    pz = particles.map(lambda particle: particle.pz).sum
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

