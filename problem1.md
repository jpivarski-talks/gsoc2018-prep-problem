# Problem 1: functional HEP

## Install dependencies

Start by installing [uproot](https://github.com/scikit-hep/uproot/) (to read HEP data files) and [oamap](https://github.com/diana-hep/oamap) (to interpret columnar data as objects).

```bash
$ pip install uproot --user
$ pip install oamap --user
```

(If you prefer to use virtualenv or conda, go ahead and do that.)

## Load them into Python

```python
>>> import oamap.source.root
>>> import uproot
```

## Look at functional.py

I've implemented a toy library that makes Python work as a functional language. It modifies Python's built-in list and tuple classes to have functional methods like `map` and `reduce`, which is not a safe thing to do in production, but it makes the examples look clear and simple.

Think about the following examples and explain to yourself what they're doing.

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

However, we can never compute anything that applies to all of the elements of an infinite list because that would literally take forever. Thus, the same restriction applies to all lazy generators, because we never know whether a lazy generator is finite or infinite.

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

The file [`HZZ.root`](http://scikit-hep.org/uproot/examples/HZZ.root) is a very small sample of high energy physics collisions, not real but realistic (simulated by computer). The collisions create Higgs bosons (one per event) that each decay into two Z bosons, and each of those decay into two electrons or two muons.

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

So if we compute mass from the total energy and total momentum of a set of particles, that mass will be approximately single-valued if the set of particles came from a particle of a given mass. If you can make a histogram, it would show up as a peak.

Here's a functional calculation of mass:

```python
from math import *
def mass(*particles):
    energy = particles.map(lambda particle: particle.energy).sum
    px = particles.map(lambda particle: particle.px).sum
    py = particles.map(lambda particle: particle.py).sum
    pz = particles.map(lambda particle: particle.pz).sum
    return sqrt(energy**2 - px**2 - py**2 - pz**2)
```

We can verify that many of these muons come from Z bosons by computing the mass of pairs of muons. Many of them are close to the Z boson mass of 91 GeV.

```python
>>> masses = (events
...           .lazy
...           .filter(lambda event: event.muons.size >= 2)
...           .map(lambda event: event.muons.pairs(mass))
...           .flatten)
... 
>>> masses.take(100)
[90.22779776988638, 74.74654928040661, 89.75736375665977, 94.85521728415152, 92.11672152709482,
 53.36427793158392, 89.84969494859244, 96.03694205062722, 86.80058490641416, 87.29730141854886,
 94.1401344991302, 99.82829548826258, 89.35045512899516, 93.01657608591354, 87.97456406526169,
 128.58445366515267, 62.48648393717207, 94.41155889760824, 93.16613921673573, 89.05260696344838,
 95.24223125872672, 91.1353537369359, 131.6289837509788, 84.15809133930381, 121.70734373587467,
 93.39859514897753, 22.996373270352922, 62.53167765812187, 42.72818072650716, 44.548414733706565,
 29.48155781970828, 85.6893720090954, 94.39039849164844, 92.17629058864074, 90.83335188133685,
 84.97899749086838, 73.10379762338002, 98.58303266123528, 94.21043301661783, 87.82125627289902,
 83.74543604301972, 88.24547163560324, 89.78230306443301, 90.82769327825987, 104.83045241665121,
 92.2173745787749, 83.74228864027768, 90.47529505648548, 87.21292251216, 85.96696855012343,
 92.74518919627343, 107.91682234922389, 52.46662314216714, 81.94645775909692, 15.467054936981627,
 34.31345895598342, 91.44368657064676, 90.03258576594307, 91.52615869443355, 94.07814544534953,
 91.62112701759015, 91.9124195359188, 90.76310384269505, 94.51892685552717, 38.901375489119076,
 92.0439494505834, 97.91948543522014, 89.42322123165098, 90.51819594401387, 107.26384921023508,
 90.26441951733524, 74.50993309312625, 93.36452236005488, 93.33739808516253, 71.12071045606393,
 31.846838381585496, 100.84634047323613, 90.4660621047501, 158.82395539632836, 92.98899149112879,
 93.17262498832066, 86.25237133936999, 88.25145119721869, 86.31652219026287, 92.6243030421588,
 89.79725149987402, 91.6714882788389, 91.75678242515797, 89.61860213130518, 94.23567073224159,
 89.29089921880713, 92.79282709730712, 92.94636949784277, 89.88178496299547, 20.303541497459513,
 85.33690283114224, 95.70057903381662, 92.48604417648964, 93.09659904899873, 86.17823185774537]
```

The masses that are far from 91 GeV are pairs of muons that are _not_ from decays of Z bosons.

## The problem to solve

Some pairs of electrons come from Z bosons, some pairs of muons come from Z bosons, and some pairs of Z bosons come from Higgs bosons. Write a functional analysis chain that identifies Higgs bosons.

Write this up as a short paper mixing human-readable paragraphs and executable code (like this problem statement; use Markdown syntax). The important thing we want to see is your thought process, not the final result, and that's why we don't specify a grading scale. We want to see you weigh the pros and cons of various techniques to give us a sense of how you'll think about more complex problems this summer.

Please e-mail your work to Jim Pivarski <pivarski@princeton.edu> and David Lange <david.lange@cern.ch>. Don't submit it as a pull request to this repository, because we don't want another applicant to be influenced by your ideas. Use the subject line "GSoC: DIANA-HEP/analysisfunctions" so that we don't lose track of your submission.

Don't get hung up if you find that the Higgs mass is not 125 GeV (its true value). I think these events were simulated before the Higgs was discovered.
