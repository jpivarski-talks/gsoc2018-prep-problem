import numpy

tasksize = 10000000

for averagesize in 0.1, 0.5, 1.0, 2.0, 5.0, 10.0, 20.0, 100.0, 1000.0:
    print(averagesize)

    # counts to offsets to starts/stops
    counts = numpy.random.poisson(averagesize, int(numpy.ceil(tasksize / averagesize)))
    offsets = numpy.empty(len(counts) + 1, dtype=numpy.int32)
    offsets[0] = 0
    numpy.cumsum(counts, out=offsets[1:])

    # content
    content = numpy.random.normal(1, 0.01, offsets[-1]).astype(numpy.float32)

    # parents
    parents = numpy.zeros(len(content), dtype=numpy.int32)
    numpy.add.at(parents, offsets[offsets != offsets[-1]][1:], 1)
    numpy.cumsum(parents, out=parents)

    offsets.tofile(open("DATA/offsets-{}".format(averagesize), "wb"))
    parents.tofile(open("DATA/parents-{}".format(averagesize), "wb"))
    content.tofile(open("DATA/content-{}".format(averagesize), "wb"))
