# Numpy_With_C

## Overview

Numpy_With_C (or NumC) is Python's Numpy module implemented in C with a Python interface. NumC takes advantage of all of C's inherent methods for speedup, including but not limited to Thread Level Parallelism, and SIMD-X86 intrinsic instructions.

## Installation

You'll need to start by creating a virtual environment in Python 3
```
$ python3 -m venv .venv
```
Activate the virtual environment before usage
```
$ source .venv/bin/activate
```
Then run:
```
$ pip3 install -r requirements.txt
```
Once you're done, remember to deacitvate your virtual environment:
```
$ deactivate
```
You can install the main NumC module by running the following command, in the repo you have cloned this one into:
```
$ make
```

## Usage

While all regular Numpy functionalty that uses [] should be available, here are a few ways that you can use to initialize a new NumC matrix:
```
>>> import numc as nc
NumC Module Imported
>>> nc.Matrix(3, 3) 					# This creates a 3 * 3 matrix with entries all zeros
[[0.0, 0.0, 0.0], [0.0, 0.0, 0.0], [0.0, 0.0, 0.0]]
>>> nc.Matrix(3, 3, 1) 					# This creates a 3 * 3 matrix with entries all ones
[[1.0, 1.0, 1.0], [1.0, 1.0, 1.0], [1.0, 1.0, 1.0]]
>>> nc.Matrix([[1, 2, 3], [4, 5, 6]]) 	# This creates a 2 * 3 matrix with first row 1, 2, 3, second row 4, 5, 6
[[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]
>>> nc.Matrix(1, 2, [4, 5]) 			# This creates a 1 * 2 matrix with entries 4, 5
[4.0, 5.0]
``` 

## Credit

Created during a class at UC Berkeley, by Gurkaran S Goindi and Rohit Deshpande

#### Disclaimer

No part of the code of this project can be used in a submission in any academic institution, in any part of the US, and especially not at UC Berkeley. This code is on Github as a showcase of my programming experience and not to enable misconduct and dishonesty. Any attempts to copy the code in this repository will result in several academic penalties.
