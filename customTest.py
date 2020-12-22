import numc as nc
import dumbpy as dp
'''
a = dp.Matrix(3,3,2)
b = nc.Matrix(3,3,2)

###LONG ERROR CHECKS
try:
    a[4] = [5,4,6]
except IndexError:
    print("dumbpy index error seen")

try:
    b[4] = [5,4,6]
except IndexError:
    print("numc index error seen")
except:
    print("numc index error mismatch")

#***********

try:
    a[2] = [[5,4,6]]
except ValueError:
    print("dumbpy value error seen")

try:
    b[2] = [[5,4,6]]
except ValueError:
    print("numc value error seen")
except:
    print("numc value error mismatch")

#***********

try:
    a[2] = 2
except TypeError:
    print("dumbpy type error seen")

try:
    b[2] = 2
except TypeError:
    print("numc type error seen")
except:
    print("numc type error mismatch")

#***********

try:
    a[0] = [5,4,6,1]
except ValueError:
    print("dumbpy value error seen")
except:
    print("dumbpy value error mismatch")

try:
    b[0] = [5,4,6,1]
except ValueError:
    print("numc value error seen")
except:
    print("numc value error mismatch")

###END LONG ERROR CHECKS


print("checking row 1d case: ")
a = dp.Matrix(1,3)
b = nc.Matrix(1,3)

a[0:2] = [3,4]
b[0:2] = [3,4]
print(a == b)

a[0:1] = 89
b[0:1] = 89
print(a == b)

a[0:3] = [8,9,10]
b[0:3] = [8,9,10]
print(a == b)

a[:] = [101, 102, 103]
b[:] = [101, 102, 103]
print(a == b)

print("checking col 1d case: ")
a = dp.Matrix(3, 1)
b = nc.Matrix(3, 1)

a[0:2] = [3,4]
b[0:2] = [3,4]
print(a == b)

a[0:1] = 89
b[0:1] = 89
print(a == b)

a[0:3] = [8,9,10]
b[0:3] = [8,9,10]
print(a == b)

a[:] = [101, 102, 103]
b[:] = [101, 102, 103]
print(a == b)
'''
###LONG ERROR CHECKS
'''
try:
    a[4] = [5,4,6]
except IndexError:
    print("dumbpy index error seen")

try:
    b[4] = [5,4,6]
except IndexError:
    print("numc index error seen")
except:
    print("numc index error mismatch")
'''

#***********
#SLICE SLICE ERROR
a = dp.Matrix(3,3,2)
b = nc.Matrix(3,3,2)

try:
    a[0:1, 0:2] = [5]
except ValueError:
    print("dumbpy value error seen")
try:
    b[0:1, 0:2] = [5]
except ValueError:
    print("TRUE VAL ERROR")
except:
    print("FALSE VAL ERROR")

#~~~~~
try:
    a[0:1, 0:2] = 78
except TypeError:
    print("dumbpy type error seen")
try:
    b[0:1, 0:2] = 78
except TypeError:
    print("TRUE TYPE ERROR")
except:
    print("FALSE TYPE ERROR")

#~~~~~
try:
    a[0:2, 0:2] = [[1,3], [5]]
except ValueError:
    print("dumbpy Val error seen")
try:
    b[0:2, 0:2] = [[1,3], [5]]
except ValueError:
    print("TRUE VAL ERROR")
except:
    print("FALSE VAL ERROR")

#~~~~~
try:
    a[0:2, 0:1] = [5]
except ValueError:
    print("dumbpy val error seen")
try:
    a[0:2, 0:1] = [5]
except ValueError:
    print("TRUE VAL ERROR")
except:
    print("FALSE VAL ERROR")

#~~~~~
try:
    a[0:2, 0:1] = 78
except TypeError:
    print("dumbpy type error seen")
try:
    b[0:2, 0:1] = 78
except TypeError:
    print("TRUE TYPE ERROR")
except:
    print("FALSE TYPE ERROR")

#~~~~~
try:
    a[0:2, 0:2] = [[1,3]]
except ValueError:
    print("dumbpy val error seen")
try:
    b[0:2, 0:2] = [[1,3]]
except ValueError:
    print("TRUE VAL ERROR")
except:
    print("FALSE VAL ERROR")

#~~~~~
try:
    a[0:1, 0:1] = "a"
except TypeError:
    print("dumbpy type error seen")
try:
    b[0:1, 0:1] = "a"
except TypeError:
    print("TRUE TYPE ERROR")
except:
    print("FALSE TYPE ERROR")


a[0:2, 0:2] = [[1,3], [5,6]]
b[0:2, 0:2] = [[1,3], [5,6]]
print("Value Test 1: ", a == b)

a[0:1, 0:2] = [89, 90]
b[0:1, 0:2] = [89, 90]
print("Value Test 2: ", a == b)

a[0:2, 0:1] = [89, 90]
b[0:2, 0:1] = [89, 90]
print("Value Test 3: ", a == b)

a[1:3, 1:3] = [[1917, 1991],[1949, 1989]]
b[1:3, 1:3] = [[1917, 1991],[1949, 1989]]
print("Value Test 4: ", a == b)

#NON-SQUARE SLICE SLICE CASES
a = dp.Matrix(3,5,2)
b = nc.Matrix(3,5,2)

a[1:3, 2:5] = [[1,2,3], [6,7,8]]
b[1:3, 2:5] = [[1,2,3], [6,7,8]]
print("Value Test 5: ", a == b)

a[2:3, 4:5] = 17
b[2:3, 4:5] = 17
print("Value Test 6: ", a == b)

print("DONE SET SUBSCRIPT SLICE SLICE TESTS")

a = dp.Matrix(9,9, 4)
b = nc.Matrix(9,9, 4)
a.set(0,2, 5)
a.set(1,1, 7)
a.set(1,2, 12)
b.set(0,2, 5)
b.set(1,1, 7)
b.set(1,2, 12)

test = (a ** 10) == (b ** 10)
if test:
    print("Pow Test 0: ", (a ** 10) == (b ** 10))
else:
    print("expected: ", (a ** 10))
    print("got: ", (b ** 10))

print("STARTING MUL TEST")

a = dp.Matrix(3,4, 2)
b = nc.Matrix(3,4, 2)
a.set(0,2, 5)
a.set(2,3, 9)
a.set(1,1, 7)
a.set(1,2, 12)
b.set(0,2, 5)
b.set(2,3, 9)
b.set(1,1, 7)
b.set(1,2, 12)
c = dp.Matrix(4, 5, 3)
d = nc.Matrix(4, 5, 3)

test = (a * c) == (b * d)
if test:
    print("Mul Test 1: ", (a * c) == (b * d))
else:
    print("expected: ", (a * c))
    print("got: ", (b * d))

a = dp.Matrix(3,3, 2)
b = nc.Matrix(3,3, 2)
a.set(0,2, 5)
a.set(1,1, 7)
a.set(1,2, 12)
b.set(0,2, 5)
b.set(1,1, 7)
b.set(1,2, 12)
c = dp.Matrix(3, 3, 3)
d = nc.Matrix(3, 3, 3)

test = (a * c) == (b * d)
if test:
    print("Mul Test 2: ", (a * c) == (b * d))
else:
    print("expected: ", (a * c))
    print("got: ", (b * d))

test = (a * a) == (b * b)
if test:
    print("Mul Test 3: ", (a * a) == (b * b))
else:
    print("expected: ", (a * a))
    print("got: ", (b * b))

mat1 = dp.Matrix(9,10, 3)
mat2 = nc.Matrix(9,10, 3)
mat1.set(0,0, 48)
mat1.set(4,8, 12)
mat1.set(3,4, 4)
mat1.set(2,3, 6)
mat1.set(1,5, 96)
mat1.set(5,6, 0)
mat1.set(6,2, 8)
mat1.set(7,1, 24)
mat1.set(8,7, 3)

mat2.set(0,0, 48)
mat2.set(4,8, 12)
mat2.set(3,4, 4)
mat2.set(2,3, 6)
mat2.set(1,5, 96)
mat2.set(5,6, 0)
mat2.set(6,2, 8)
mat2.set(7,1, 24)
mat2.set(8,7, 3)

mat3 = dp.Matrix(10,9, 4)
mat4 = nc.Matrix(10,9, 4)


test = (mat1 * mat3) == (mat2 * mat4)
if test:
    print("Mul Test 4: ", (mat1 * mat3) == (mat2 * mat4))
else:
    print("expected: ", (mat1 * mat3))
    print("got: ", (mat2 * mat4))

a = dp.Matrix(9,9, 2)
b = nc.Matrix(9,9, 2)
a.set(0,2, 5)
a.set(1,1, 7)
a.set(1,2, 12)
b.set(0,2, 5)
b.set(1,1, 7)
b.set(1,2, 12)

test = (a ** 10) == (b ** 10)
if test:
    print("Pow Test 1: ", test)
else:
    print("expected: ", (a ** 10))
    print("got: ", (b ** 10))

# print((a ** 10))
# print((b ** 10))
















