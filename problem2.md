# Problem 2: vectorized HEP

## Install dependencies

Start by installing [meta](https://github.com/srossross/Meta), used internally by vectorized.py.

```bash
$ pip install meta --user
```

(If you prefer to use virtualenv or conda, go ahead and do that.)

## The problem

Real HEP datasets are large, and therefore must be processed quickly to keep physicists productive. This is aided in part by the fact that HEP datasets are stored in columns:

```python
>>> import uproot
>>> columnar_events = uproot.open("http://scikit-hep.org/uproot/examples/HZZ.root")["events"]
>>> columns = columnar_events.arrays(["*Muon*"])
>>> columns["Muon_Px"].content
array([-52.899456  ,  37.73778   ,  -0.81645936, ..., -29.756786  ,
         1.1418698 ,  23.913206  ], dtype=float32)
>>> columns["NMuon"]
array([2, 1, 2, ..., 1, 1, 1], dtype=int32)
```

Each particle attribute is contiguous in memory and therefore suffers fewer page misses when passed through the CPU cache.

This format could also allow for vectorized processing, in which groups of neighboring elements of an array are processed together, in lock-step ("SIMD"). For instance, if we wanted to compute the total momentum (`px**2 + py**2 + pz**2`) of all muons, we could either do it in a non-vectorized way:

```python
>>> import numpy
>>> Muon_Px = columns["Muon_Px"].content
>>> Muon_Py = columns["Muon_Py"].content
>>> Muon_Pz = columns["Muon_Pz"].content

>>> Muon_P = numpy.empty(len(Muon_Px))
>>> for i in range(len(Muon_Px)):
...     Muon_P[i] = numpy.sqrt(Muon_Px[i]**2 + Muon_Py[i]**2 + Muon_Pz[i]**2)
...
>>> Muon_P
array([ 54.77939728,  39.40155414,  31.69026934, ...,  62.39507159,
       174.20860614,  69.55613492])
```

or in a vectorized way:

```python
Muon_P = numpy.sqrt(Muon_Px**2 + Muon_Py**2 + Muon_Pz**2)
```

The lack of indexes tells Numpy to perform `Muon_Px[i]**2` for all `i` before moving on to `Muon_Py**2`, etc. This is faster than the non-vectorized Python for loop because the loop over many items is applied to a uniform type in compiled code. It can also be faster than a non-vectorized loop in a compiled language like C++ because identical operations on neighboring elements in memory can take advantage of special instructions in the microprocessor that perform 4 multiplications side by side in one clock tick, rather than just one. In the extreme case, it could be loaded into a GPU and 1024 multiplications can be performed side by side.

The cost, however, is conceptual: it's easier to think in non-vectorized terms than in vectorized terms. This summer's project is about implementing common HEP analysis tasks in a vectorized way and hiding that complexity in a functional interface.

## Testing vectorized intuition

To help us think and talk about vectorized algorithms, I've implemented another toy library, [vectorized.py](vectorized.py). The `vectorize` function takes a function, such as `totalp` below, and runs many copies of that function, one statement at a time. The function must take an `index` as its first argument (to know which element to operate on) and arrays for input (`Muon_Px, Muon_Py, Muon_Pz`) and output (`Muon_P`). (If you've ever written a CUDA kernel to program a GPU, this would be familiar. If not, here's how it works!)

```python
>>> from vectorized import vectorize
>>> Muon_P = numpy.empty(len(Muon_Px))
>>> def totalp(index, Muon_Px, Muon_Py, Muon_Pz, Muon_P):
...     px2 = Muon_Px[index]**2
...     py2 = Muon_Py[index]**2
...     pz2 = Muon_Pz[index]**2
...     Muon_P[index] = numpy.sqrt(px2 + py2 + pz2)
... 
>>> vectorize(totalp, len(Muon_Px), Muon_Px, Muon_Py, Muon_Pz, Muon_P)
leading step 0 (100.0% at leading): 
    px2 = (Muon_Px[index] ** 2)
    ...advancing 1

leading step 1 (100.0% at leading): 
    py2 = (Muon_Py[index] ** 2)
    ...advancing 2

leading step 2 (100.0% at leading): 
    pz2 = (Muon_Pz[index] ** 2)
    ...advancing 3

leading step 3 (100.0% at leading): 
    Muon_P[index] = numpy.sqrt(((px2 + py2) + pz2))
    ...advancing 4

5
```

Since `len(Muon_Px)` is 3825, it starts 3825 instances of `totalp`, runs the first line for all of them, the second line for all of them, etc., printing out each line as it goes. Then it returns the number of vectorized steps. Because `totalp` is a simple function (an easily vectorizable one), the number of vectorized steps is just the number of statements.

The trouble comes when we want to do something that involves if statements (branching) or for/while blocks (looping). Suppose the problem is to find the maximum muon momentum of each event. Now there are two scales: (1) the number of events and (2) the number of muons.

To begin with, we need a lookup table to associate event indexes with muon indexes. We have this table in the `.starts` and `.stops` of the muon columns.

```python
>>> starts = columns["Muon_Px"].starts
>>> stops = columns["Muon_Px"].stops
>>> starts
array([   0,    2,    3, ..., 3822, 3823, 3824])
>>> stops
array([   2,    3,    5, ..., 3823, 3824, 3825])
```

(The first event has two muons, the second has one, the third has two, etc.) We have a choice in whether we index the vectorization on the number of events or the number of muons; let's do the number of events.

```python
>>> highest_by_event = numpy.empty(len(starts))

>>> def maxp(index, starts, stops, Muon_P, highest_by_event):
...     highest = float("nan")
...     for i in range(starts[index], stops[index]):
...         if numpy.isnan(highest) or Muon_P[i] > highest:
...             highest = Muon_P[i]
...     highest_by_event[index] = highest
... 
>>> vectorize(maxp, len(starts), starts, stops, Muon_P, highest_by_event)
leading step 0 (100.0% at leading): 
    highest = float('nan')
    ...advancing 1

leading step 1 (100.0% at leading): 
    for i in range(starts[index], stops[index]):
        if (numpy.isnan(highest) or (Muon_P[i] > highest)):    
            highest = Muon_P[i]
    ...advancing 2

leading step 4 (2.44% at leading): 
    highest_by_event[index] = highest
    ...catching up 3 (2.44% at leading)
    ...catching up 4 (2.44% at leading)
    ...catching up 5 (41.64% at leading)
    ...catching up 6 (86.66% at leading)
    ...catching up 7 (99.13% at leading)
    ...catching up 8 (99.75% at leading)
    ...catching up 9 (99.92% at leading)
    ...catching up 10 (99.96% at leading)
    ...advancing 11

12
```

There are 2421 events and 2.44% of them have zero muons. The events with zero muons are the first to get to step 4 (the last step) and the rest catch up by going through the loop. Keep in mind that because this is vectorized, whenever any threads are still going through the loop, the others cannot proceed. It takes as long as the longest thread, and that's bad: one event with many muons can spoil the whole calculation.

This is basically how CUDA functions for GPUs work, except that this library illustrates the process, showing where algorithms slow down and why. (Technically, this `vectorize` function emulates one big "warp.")

## Problem to consider: Vectorizing mass calculations efficiently

The problem for you to solve is the following: perform Z mass calculations in the fewest possible vectorized steps. There are three scales in this problem: (1) the number of events, (2) the number of muons per event, and (3) the number of muon _pairs_ per event. There will be multiple Z candidates in each event, not simply because the Higgs decays into two of them, but also because the same list of muons can be combined in multiple ways.

Bonus for also reducing the multiple Z candidates per event to the single best Z candidate per event (closest to 91 GeV). Double bonus for optimizing Higgs candidates. Triple bonus for hiding the vectorized function under a functional interface. These things are what the summer project is about (though we'll have more problems than just computing masses).

As with problem 1, we're much more interested in your thought process than strictly minimizing the numerical output of the `vectorize` function. You'll probably notice that writing the same expression on two lines counts as two statements as writing it on one line, but if it doesn't involve any branching or looping, we don't care.

As with problem 1, please e-mail your work to Jim Pivarski <pivarski@princeton.edu> and David Lange <david.lange@cern.ch>. Don't submit it as a pull request to this repository, because we don't want another applicant to be influenced by your ideas. Use the subject line "GSoC: DIANA-HEP/analysisfunctions" so that we don't lose track of your submission.
