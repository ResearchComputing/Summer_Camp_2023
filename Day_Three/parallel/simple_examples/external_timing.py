import numpy
def dotprod(a,b):
    c = 0
    for i in range(len(a)):
        c+=a[i]*b[i]
    return c

def absdiff(a,b):
    c = numpy.abs(a-b)
    return c

def maxabs(a,b):
    c = numpy.max(numpy.abs(a-b))
    return c

def main():
    ntests = 1000
    n = 16384
    a = numpy.zeros(n,dtype='float64')
    b = numpy.zeros(n,dtype='float64')
    for i in range(n):
        imod = i % 2
        a[i] = i*i
        b[i] = a[i]*i*(-1)**imod
    for i in range(ntests):
        dotprod(a,b)
        absdiff(a,b)
        maxabs(a,b)

main()
