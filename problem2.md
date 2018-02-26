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

The lack of indexes tells Numpy to perform `Muon_Px[i]**2` for all `i` before moving on to `Muon_Py**2`, etc. This is faster than the non-vectorized Python for loop because the loop over many items is applied to a uniform type in compiled code. It can also be faster than a non-vectorized loop in a compiled language like C++ because identical operations on neighboring elements in memory can take advantage of special instructions in the microprocessor that perform four multiplications side by side in one clock tick, rather than just one. In the extreme case, it could be loaded into a GPU and 1024 multiplications can be performed side by side.





