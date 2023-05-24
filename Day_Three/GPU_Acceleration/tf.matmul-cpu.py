import os
import numpy as np
import tensorflow as tf
import time

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
os.environ['CUDA_VISIBLE_DEVICES'] = '-1'
if tf.test.gpu_device_name():
        print("GPU found")
else:
        print("Tensorflow is using only CPUs.")

numpy_start = time.time()
a = np.random.rand(10000, 70000)
print("Numpy generated a random 10000x70000 matrix, matrix a.")
b = np.random.rand(70000, 10000)
numpy_end = time.time()
print("Numpy generated a random 70000x10000 matrix, matrix b.")
print("It took numpy", numpy_end-numpy_start, "seconds to generate matrices a and b.")
print("Tensorflow is beginning matrix multiplication.")
tf_start = time.time()
c = tf.matmul(a,b)
tf_end  = time.time()
print("Tensorflow multiplied matrices a and b in", tf_end-tf_start, "seconds.")
