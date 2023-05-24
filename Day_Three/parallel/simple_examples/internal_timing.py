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
    import time
    ntests = 1000
    n = 16384
    
    #initialize
    t1 = time.time()
    a = numpy.zeros(n,dtype='float64')
    b = numpy.zeros(n,dtype='float64')
    for i in range(n):
        imod = i % 2
        a[i] = i*i
        b[i] = a[i]*i*(-1)**imod
    t2 = time.time()
    dt = t2-t1
    print('Init time (s): ', dt)
    
    #math tests
    t1 = time.time()
    for i in range(ntests):
        dotprod(a,b)
        absdiff(a,b)
        maxabs(a,b)
    t2 = time.time()
    dt = t2-t1
    print('math test time (s): ', dt)

main()
