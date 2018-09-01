import numpy

tasksize = 10000000   # 100000000

for averagesize in 0.3, 0.5, 1.0, 2.0, 3.0, 5.0, 10.0, 20.0, 30.0, 50.0, 100.0, 200.0, 300.0, 500.0, 1000.0:
    print(averagesize)

    # counts to offsets to starts/stops
    print("offsets")
    counts = numpy.random.poisson(averagesize, int(numpy.ceil(tasksize / averagesize)))
    offsets = numpy.empty(len(counts) + 1, dtype=numpy.int32)
    offsets[0] = 0
    numpy.cumsum(counts, out=offsets[1:])
    del counts
    with open("DATA/offsets-{}".format(averagesize), "wb") as f:
        offsets.tofile(f)

    # content
    print("content")
    content = numpy.random.normal(1, 0.01, offsets[-1]).astype(numpy.float32)
    with open("DATA/content-{}".format(averagesize), "wb") as f:
        content.tofile(f)

    # parents
    print("parents")
    parents = numpy.zeros(len(content), dtype=numpy.int32)
    numpy.add.at(parents, offsets[offsets != offsets[-1]][1:], 1)
    numpy.cumsum(parents, out=parents)
    with open("DATA/parents-{}".format(averagesize), "wb") as f:
        parents.tofile(f)

    del offsets
    del content
    del parents
