#include "numc.h"
#include <structmember.h>


PyTypeObject Matrix61cType;

/* Helper functions for initalization of matrices and vectors */

/*
 * Return a tuple given rows and cols
 */
PyObject *get_shape(int rows, int cols) {
  if (rows == 1 || cols == 1) {
    return PyTuple_Pack(1, PyLong_FromLong(rows * cols));
  } else {
    return PyTuple_Pack(2, PyLong_FromLong(rows), PyLong_FromLong(cols));
  }
}
/*
 * Matrix(rows, cols, low, high). Fill a matrix random double values
 */
int init_rand(PyObject *self, int rows, int cols, unsigned int seed, double low,
              double high) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    rand_matrix(new_mat, seed, low, high);
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(rows, cols, val). Fill a matrix of dimension rows * cols with val
 */
int init_fill(PyObject *self, int rows, int cols, double val) {
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed)
        return alloc_failed;
    else {
        fill_matrix(new_mat, val);
        ((Matrix61c *)self)->mat = new_mat;
        ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    }
    return 0;
}

/*
 * Matrix(rows, cols, 1d_list). Fill a matrix with dimension rows * cols with 1d_list values
 */
int init_1d(PyObject *self, int rows, int cols, PyObject *lst) {
    if (rows * cols != PyList_Size(lst)) {
        PyErr_SetString(PyExc_ValueError, "Incorrect number of elements in list");
        return -1;
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    int count = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j, PyFloat_AsDouble(PyList_GetItem(lst, count)));
            count++;
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * Matrix(2d_list). Fill a matrix with dimension len(2d_list) * len(2d_list[0])
 */
int init_2d(PyObject *self, PyObject *lst) {
    int rows = PyList_Size(lst);
    if (rows == 0) {
        PyErr_SetString(PyExc_ValueError,
                        "Cannot initialize numc.Matrix with an empty list");
        return -1;
    }
    int cols;
    if (!PyList_Check(PyList_GetItem(lst, 0))) {
        PyErr_SetString(PyExc_ValueError, "List values not valid");
        return -1;
    } else {
        cols = PyList_Size(PyList_GetItem(lst, 0));
    }
    for (int i = 0; i < rows; i++) {
        if (!PyList_Check(PyList_GetItem(lst, i)) ||
                PyList_Size(PyList_GetItem(lst, i)) != cols) {
            PyErr_SetString(PyExc_ValueError, "List values not valid");
            return -1;
        }
    }
    matrix *new_mat;
    int alloc_failed = allocate_matrix(&new_mat, rows, cols);
    if (alloc_failed) return alloc_failed;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            set(new_mat, i, j,
                PyFloat_AsDouble(PyList_GetItem(PyList_GetItem(lst, i), j)));
        }
    }
    ((Matrix61c *)self)->mat = new_mat;
    ((Matrix61c *)self)->shape = get_shape(new_mat->rows, new_mat->cols);
    return 0;
}

/*
 * This deallocation function is called when reference count is 0
 */
void Matrix61c_dealloc(Matrix61c *self) {
    deallocate_matrix(self->mat);
    Py_TYPE(self)->tp_free(self);
}

/* For immutable types all initializations should take place in tp_new */
PyObject *Matrix61c_new(PyTypeObject *type, PyObject *args,
                        PyObject *kwds) {
    /* size of allocated memory is tp_basicsize + nitems*tp_itemsize*/
    Matrix61c *self = (Matrix61c *)type->tp_alloc(type, 0);
    return (PyObject *)self;
}

/*
 * This matrix61c type is mutable, so needs init function. Return 0 on success otherwise -1
 */
int Matrix61c_init(PyObject *self, PyObject *args, PyObject *kwds) {
    /* Generate random matrices */
    if (kwds != NULL) {
        PyObject *rand = PyDict_GetItemString(kwds, "rand");
        if (!rand) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (!PyBool_Check(rand)) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
        if (rand != Py_True) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        PyObject *low = PyDict_GetItemString(kwds, "low");
        PyObject *high = PyDict_GetItemString(kwds, "high");
        PyObject *seed = PyDict_GetItemString(kwds, "seed");
        double double_low = 0;
        double double_high = 1;
        unsigned int unsigned_seed = 0;

        if (low) {
            if (PyFloat_Check(low)) {
                double_low = PyFloat_AsDouble(low);
            } else if (PyLong_Check(low)) {
                double_low = PyLong_AsLong(low);
            }
        }

        if (high) {
            if (PyFloat_Check(high)) {
                double_high = PyFloat_AsDouble(high);
            } else if (PyLong_Check(high)) {
                double_high = PyLong_AsLong(high);
            }
        }

        if (double_low >= double_high) {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }

        // Set seed if argument exists
        if (seed) {
            if (PyLong_Check(seed)) {
                unsigned_seed = PyLong_AsUnsignedLong(seed);
            }
        }

        PyObject *rows = NULL;
        PyObject *cols = NULL;
        if (PyArg_UnpackTuple(args, "args", 2, 2, &rows, &cols)) {
            if (rows && cols && PyLong_Check(rows) && PyLong_Check(cols)) {
                return init_rand(self, PyLong_AsLong(rows), PyLong_AsLong(cols), unsigned_seed, double_low,
                                 double_high);
            }
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    }
    PyObject *arg1 = NULL;
    PyObject *arg2 = NULL;
    PyObject *arg3 = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 3, &arg1, &arg2, &arg3)) {
        /* arguments are (rows, cols, val) */
        if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && (PyLong_Check(arg3)
                || PyFloat_Check(arg3))) {
            if (PyLong_Check(arg3)) {
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyLong_AsLong(arg3));
            } else
                return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), PyFloat_AsDouble(arg3));
        } else if (arg1 && arg2 && arg3 && PyLong_Check(arg1) && PyLong_Check(arg2) && PyList_Check(arg3)) {
            /* Matrix(rows, cols, 1D list) */
            return init_1d(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), arg3);
        } else if (arg1 && PyList_Check(arg1) && arg2 == NULL && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_2d(self, arg1);
        } else if (arg1 && arg2 && PyLong_Check(arg1) && PyLong_Check(arg2) && arg3 == NULL) {
            /* Matrix(rows, cols, 1D list) */
            return init_fill(self, PyLong_AsLong(arg1), PyLong_AsLong(arg2), 0);
        } else {
            PyErr_SetString(PyExc_TypeError, "Invalid arguments");
            return -1;
        }
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return -1;
    }
}

/*
 * List of lists representations for matrices
 */
PyObject *Matrix61c_to_list(Matrix61c *self) {
    int rows = self->mat->rows;
    int cols = self->mat->cols;
    PyObject *py_lst = NULL;
    if (self->mat->is_1d) {  // If 1D matrix, print as a single list
        py_lst = PyList_New(rows * cols);
        int count = 0;
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(py_lst, count, PyFloat_FromDouble(get(self->mat, i, j)));
                count++;
            }
        }
    } else {  // if 2D, print as nested list
        py_lst = PyList_New(rows);
        for (int i = 0; i < rows; i++) {
            PyList_SetItem(py_lst, i, PyList_New(cols));
            PyObject *curr_row = PyList_GetItem(py_lst, i);
            for (int j = 0; j < cols; j++) {
                PyList_SetItem(curr_row, j, PyFloat_FromDouble(get(self->mat, i, j)));
            }
        }
    }
    return py_lst;
}

PyObject *Matrix61c_class_to_list(Matrix61c *self, PyObject *args) {
    PyObject *mat = NULL;
    if (PyArg_UnpackTuple(args, "args", 1, 1, &mat)) {
        if (!PyObject_TypeCheck(mat, &Matrix61cType)) {
            PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
            return NULL;
        }
        Matrix61c* mat61c = (Matrix61c*)mat;
        return Matrix61c_to_list(mat61c);
    } else {
        PyErr_SetString(PyExc_TypeError, "Invalid arguments");
        return NULL;
    }
}

/*
 * Add class methods
 */
PyMethodDef Matrix61c_class_methods[] = {
    {"to_list", (PyCFunction)Matrix61c_class_to_list, METH_VARARGS, "Returns a list representation of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/*
 * Matrix61c string representation. For printing purposes.
 */
PyObject *Matrix61c_repr(PyObject *self) {
    PyObject *py_lst = Matrix61c_to_list((Matrix61c *)self);
    return PyObject_Repr(py_lst);
}

/* NUMBER METHODS */

/*
 * Add the second numc.Matrix (Matrix61c) object to the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_add(Matrix61c* self, PyObject* args) {
   if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
        return NULL;
    }
    int argRows = ((Matrix61c*)args)->mat->rows;
    int argCols = ((Matrix61c*)args)->mat->cols;
    if(self->mat->rows != argRows || self->mat->cols != argCols) {
        PyErr_SetString(PyExc_ValueError, "Dimensions of matricies don't match!");
        return NULL;
    }
    Matrix61c *temp = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *newMat;
    matrix **newTest = &newMat;
    allocate_matrix(newTest, argRows, argCols);
    add_matrix(*newTest, self->mat, ((Matrix61c*)args)->mat);
    temp->mat = *newTest;
    temp->shape = get_shape(argRows, argCols);
    return temp;
}

/*
 * Substract the second numc.Matrix (Matrix61c) object from the first one. The first operand is
 * self, and the second operand can be obtained by casting `args`.
 */
PyObject *Matrix61c_sub(Matrix61c* self, PyObject* args) {
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
        return NULL;
    }
    int argRows = ((Matrix61c*)args)->mat->rows;
    int argCols = ((Matrix61c*)args)->mat->cols;
    if(self->mat->rows != argRows || self->mat->cols != argCols) {
        PyErr_SetString(PyExc_ValueError, "Dimensions of matricies don't match!");
        return NULL;
    }
    Matrix61c *temp = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *newMat;
    matrix **newTest = &newMat;
    allocate_matrix(newTest, argRows, argCols);
    sub_matrix(*newTest, self->mat, ((Matrix61c*)args)->mat);
    temp->mat = *newTest;
    temp->shape = get_shape(argRows, argCols);
    return temp;
}

/*
 * NOT element-wise multiplication. The first operand is self, and the second operand
 * can be obtained by casting `args`.
 */
PyObject *Matrix61c_multiply(Matrix61c* self, PyObject *args) {
    if (!PyObject_TypeCheck(args, &Matrix61cType)) {
        PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
        return NULL;
    }
    int argRows = ((Matrix61c*)args)->mat->rows;
    int argCols = ((Matrix61c*)args)->mat->cols;
    if(self->mat->cols != argRows) {
        PyErr_SetString(PyExc_ValueError, "Dimensions of matricies don't match!");
        return NULL;
    }
    Matrix61c *temp = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *newMat;
    matrix **newTest = &newMat;
    allocate_matrix(newTest, self->mat->rows, argCols);
    mul_matrix(*newTest, self->mat, ((Matrix61c*)args)->mat);
    temp->mat = *newTest;
    temp->shape = get_shape(self->mat->rows, argCols);
    return temp;
}

/*
 * Negates the given numc.Matrix.
 */
PyObject *Matrix61c_neg(Matrix61c* self) {
    Matrix61c *temp = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *newMat;
    matrix **newTest = &newMat;
    allocate_matrix(newTest, self->mat->rows, self->mat->cols);
    neg_matrix(*newTest, self->mat);
    temp->mat = *newTest;
    temp->shape = get_shape(self->mat->rows, self->mat->cols);
    return temp;
}

/*
 * Take the element-wise absolute value of this numc.Matrix.
 */
PyObject *Matrix61c_abs(Matrix61c *self) {
    Matrix61c *temp = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *newMat;
    matrix **newTest = &newMat;
    allocate_matrix(newTest, self->mat->rows, self->mat->cols);
    abs_matrix(*newTest, self->mat);
    temp->mat = *newTest;
    temp->shape = get_shape(self->mat->rows, self->mat->cols);
    return temp;
}

/*
 * Raise numc.Matrix (Matrix61c) to the `pow`th power. You can ignore the argument `optional`.
 */
PyObject *Matrix61c_pow(Matrix61c *self, PyObject *pow, PyObject *optional) {
    if (!PyObject_TypeCheck(self, &Matrix61cType) || !PyLong_Check(pow)) {
        PyErr_SetString(PyExc_TypeError, "Argument must of type numc.Matrix!");
        return NULL;
    }
    if(self->mat->cols != self->mat->rows) {
        PyErr_SetString(PyExc_ValueError, "Dimensions of matricies don't match!");
        return NULL;
    }
    Matrix61c *temp = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *newMat;
    matrix **newTest = &newMat;
    allocate_matrix(newTest, self->mat->rows, self->mat->cols);
    pow_matrix(*newTest, self->mat, PyLong_AsLong(pow));
    temp->mat = *newTest;
    temp->shape = get_shape(self->mat->rows, self->mat->cols);
    return temp;
}

/*
 * Create a PyNumberMethods struct for overloading operators with all the number methods you have
 * define. You might find this link helpful: https://docs.python.org/3.6/c-api/typeobj.html
 */
PyNumberMethods Matrix61c_as_number = {
    .nb_add = Matrix61c_add, 
    .nb_subtract = Matrix61c_sub,
    .nb_multiply = Matrix61c_multiply,
    .nb_remainder = 0,
    .nb_divmod = 0,
    .nb_power = Matrix61c_pow,
    .nb_negative = Matrix61c_neg,
    .nb_positive = 0,
    .nb_absolute = Matrix61c_abs,
    .nb_bool = 0,
    .nb_invert = 0,
    .nb_lshift = 0,
    .nb_rshift = 0,
    .nb_and = 0,
    .nb_xor = 0,
    .nb_or = 0,
    .nb_int = 0,
    .nb_reserved = 0,
    .nb_float = 0,
    .nb_inplace_add = 0,
    .nb_inplace_subtract = 0,
    .nb_inplace_multiply = 0,
    .nb_inplace_remainder = 0,
    .nb_inplace_power = 0,
    .nb_inplace_lshift = 0,
    .nb_inplace_rshift = 0,
    .nb_inplace_and = 0,
    .nb_inplace_xor = 0,
    .nb_inplace_or = 0,
    .nb_floor_divide = 0,
    .nb_true_divide = 0,
    .nb_inplace_floor_divide = 0,
    .nb_inplace_true_divide = 0,
    .nb_index = 0,
    .nb_matrix_multiply = 0,
    .nb_inplace_matrix_multiply = 0,

};


/* INSTANCE METHODS */

/*
 * Given a numc.Matrix self, parse `args` to (int) row, (int) col, and (double/int) val.
 * Return None in Python (this is different from returning null).
 */
PyObject *Matrix61c_set_value(Matrix61c *self, PyObject* args) {
    if ((int) PyTuple_Size(args) != 3) {
        PyErr_SetString(PyExc_TypeError, "Argument must have 3 entries!");
        return NULL;
    }
    if (!PyObject_TypeCheck(PyTuple_GetItem(args, 0), &PyLong_Type)) {
        PyErr_SetString(PyExc_TypeError, "row is not proper type");
        return NULL;
    }
    if (!PyObject_TypeCheck(PyTuple_GetItem(args, 1), &PyLong_Type)) {
        PyErr_SetString(PyExc_TypeError, "col is not proper type");
        return NULL;
    }
    if (!PyObject_TypeCheck(PyTuple_GetItem(args, 2), &PyLong_Type) && !PyObject_TypeCheck(PyTuple_GetItem(args, 2), &PyFloat_Type)) {
        if (!PyObject_TypeCheck(PyTuple_GET_ITEM(args, 2), &PyLong_Type)) {
            PyErr_SetString(PyExc_TypeError, "val is not int type");
            return NULL;
        } else {
            PyErr_SetString(PyExc_TypeError, "val is not double type");
            return NULL;
        }
    }
    if (PyLong_AsLong(PyTuple_GetItem(args, 0)) > self->mat->rows - 1 || PyLong_AsLong(PyTuple_GetItem(args, 1)) > self->mat->cols - 1) {
        PyErr_SetString(PyExc_IndexError, "Row or col is out of bounds");
        return NULL;
    }
    //printf("pre-Rows\n");
    int rows = (int) PyLong_AsLong(PyTuple_GET_ITEM(args, 0));
    int cols = (int) PyLong_AsLong(PyTuple_GET_ITEM(args, 1));
    set(self->mat, rows, cols, PyFloat_AsDouble(PyTuple_GET_ITEM(args, 2)));
    return Py_None;
}

/*
 * Given a numc.Matrix `self`, parse `args` to (int) row and (int) col.
 * Return the value at the `row`th row and `col`th column, which is a Python
 * float/int.
 */
PyObject *Matrix61c_get_value(Matrix61c *self, PyObject* args) {
    /* TODO: YOUR CODE HERE */
    if ((int) PyTuple_Size(args) != 2) {
        PyErr_SetString(PyExc_TypeError, "Argument must have 2 entries!");
        return NULL;
    }
    if (!PyObject_TypeCheck(PyTuple_GetItem(args, 0), &PyLong_Type)) {
        PyErr_SetString(PyExc_TypeError, "row is not proper type");
        return NULL;
    }
    if (!PyObject_TypeCheck(PyTuple_GetItem(args, 1), &PyLong_Type)) {
        PyErr_SetString(PyExc_TypeError, "col is not proper type");
        return NULL;
    }
    if (PyLong_AsLong(PyTuple_GetItem(args, 0)) > self->mat->rows - 1 || PyLong_AsLong(PyTuple_GetItem(args, 1)) > self->mat->cols - 1) {
        PyErr_SetString(PyExc_IndexError, "Row or col is out of bounds");
        return NULL;
    } //*(*(mat->data + row) + col)
    int rows = (int) PyLong_AsLong(PyTuple_GET_ITEM(args, 0));
    int cols = (int) PyLong_AsLong(PyTuple_GET_ITEM(args, 1));
    return PyFloat_FromDouble(self->mat->data[rows][cols]);
}

/*
 * Create an array of PyMethodDef structs to hold the instance methods.
 * Name the python function corresponding to Matrix61c_get_value as "get" and Matrix61c_set_value
 * as "set"
 * You might find this link helpful: https://docs.python.org/3.6/c-api/structures.html
 */
PyMethodDef Matrix61c_methods[] = { // Matrix61c_set_value
    /* TODO: YOUR CODE HERE */
    {"set", (PyCFunction)Matrix61c_set_value, METH_VARARGS, "sets value of numc.Matrix"}, 
    {"get", (PyCFunction)Matrix61c_get_value, METH_VARARGS, "gets value of numc.Matrix"},
    {NULL, NULL, 0, NULL}
};

/* INDEXING */

/*
 * Given a numc.Matrix `self`, index into it with `key`. Return the indexed result.
 */
PyObject *Matrix61c_subscript(Matrix61c* self, PyObject* key) {
    int rowDim = self->mat->rows;
    int colDim = self->mat->cols;
    int rowOffset = 0;
    int colOffset = 0;
    int rows = 0;
    int cols = 0;
    Matrix61c *temp = (Matrix61c *) Matrix61c_new(&Matrix61cType, NULL, NULL);
    matrix *newMat;
    matrix **newTest = &newMat;

    //1-D CASE (also handles a[x][x] case)
    if (self->mat->rows == 1) {
        if (PyObject_TypeCheck(key, &PyLong_Type)) {
            if ((int) PyLong_AsLong(key) >= colDim || (int) PyLong_AsLong(key) < 0) {
                PyErr_SetString(PyExc_IndexError, "Out of Bounds Error");
                return NULL;
            }
            return PyFloat_FromDouble(get(self->mat, 0, (int) PyLong_AsLong(key)));
        }
        if (PyObject_TypeCheck(key, &PySlice_Type)){
            PyObject* slice = key;
            Py_ssize_t length = colDim;
            Py_ssize_t begin = 0; Py_ssize_t stop = 0; Py_ssize_t step = 0; Py_ssize_t sliceLength = 0;
            PySlice_GetIndicesEx(slice, length, &begin, &stop, &step, &sliceLength);
            if (sliceLength == 0 || step != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return NULL;
            }
            if (sliceLength == 1) {
                return PyFloat_FromDouble(get(self->mat, 0, (int) begin));
            }
            rows = 1;
            cols = sliceLength;
            temp->shape = get_shape(rows, cols);
            allocate_matrix_ref(newTest, self->mat, 0, (int) begin, (int) rows, cols);
            temp->mat = *newTest;
            return temp;
        }
        PyErr_SetString(PyExc_TypeError, "matrix is 1d but you are not using only splice or index");
        return NULL;
    }
    //LONG ONLY
    if (PyObject_TypeCheck(key, &PyLong_Type)) {
        if (PyLong_AsLong(key) < 0 || PyLong_AsLong(key) >= rowDim) {
            PyErr_SetString(PyExc_IndexError, "Row is out of bounds");
            return NULL;
        }
        if (colDim == 1) {
            return PyFloat_FromDouble(get(self->mat, PyLong_AsLong(key), 0));
        }
        rowOffset = PyLong_AsLong(key);
        rows = 1;
        cols = colDim;
        temp->shape = get_shape(rows, cols);
        allocate_matrix_ref(newTest, self->mat, rowOffset, colOffset, rows, cols);
        temp->mat = *newTest;
        return temp;
    } 
    //SLICE ONLY
    if (PyObject_TypeCheck(key, &PySlice_Type)) {
        PyObject* slice = key;
        Py_ssize_t length = rowDim;
        Py_ssize_t begin = 0; Py_ssize_t stop = 0; Py_ssize_t step = 0; Py_ssize_t sliceLength = 0;
        int ret = PySlice_GetIndicesEx(slice, length, &begin, &stop, &step, &sliceLength);
        if (ret == -1) {
            PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into slice\n");
            return NULL;
        }
        
        if (sliceLength == 0 || step != 1) {
            PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds, or step size not 1\n");
            return NULL;
        }
        if (sliceLength == 1 && colDim == 1) {
            return PyFloat_FromDouble(get(self->mat, (int) begin, 0));
        }
        //printf("did stuff\n");
        
        rows = sliceLength;
        cols = colDim;
        temp->shape = get_shape(rows, cols);
        allocate_matrix_ref(newTest, self->mat, (int) begin, 0, (int) rows, cols);
        printf("%d\n",self->mat->ref_cnt);
        temp->mat = *newTest;
        return temp;

    }
    //TUPLE TYPES
    if (PyObject_TypeCheck(key, &PyTuple_Type)) {
        if (self->mat->cols == 1) {
            PyErr_SetString(PyExc_TypeError, "matrix has 1 column, 2d access not allowed.\n");
            return NULL;
        }
        //LONG LONG
        if (PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PyLong_Type) && PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PyLong_Type)) {
            int rowOffset = (int) PyLong_AsLong(PyTuple_GET_ITEM(key, 0));
            int colOffset = (int) PyLong_AsLong(PyTuple_GET_ITEM(key, 1));
            if (rowOffset >= self->mat->rows || colOffset >= self->mat->cols || rowOffset < 0 || colOffset < 0) {
                PyErr_SetString(PyExc_IndexError, "row or col is out of bounds \n");
                return NULL;
            }
            return PyFloat_FromDouble(get(self->mat, rowOffset, colOffset));
        }   
        //SLICE SLICE
        if (PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PySlice_Type) && PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PySlice_Type)) {
            PyObject* slice0 = PyTuple_GET_ITEM(key, 0);
            Py_ssize_t length0 = rowDim;
            Py_ssize_t begin0 = 0; Py_ssize_t stop0 = 0; Py_ssize_t step0 = 0; Py_ssize_t sliceLength0 = 0;
            int ret = PySlice_GetIndicesEx(slice0, length0, &begin0, &stop0, &step0, &sliceLength0);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into first slice\n");
                return NULL;
            }

            if (sliceLength0 == 0 || step0 != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return NULL;
            }

            PyObject* slice1 = PyTuple_GET_ITEM(key, 1);
            Py_ssize_t length1 = colDim; //TODO: CONFIRM
            Py_ssize_t begin1 = 0; Py_ssize_t stop1 = 0; Py_ssize_t step1 = 0; Py_ssize_t sliceLength1 = 0;
            ret = PySlice_GetIndicesEx(slice1, length1, &begin1, &stop1, &step1, &sliceLength1);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into second slice\n");
                return NULL;
            }

            if (sliceLength1 == 0 || step1 != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return NULL;
            }

            rows = sliceLength0;
            cols = sliceLength1;
            //Case for a single integer being returned
            if (sliceLength0 == 1 && sliceLength1 == 1) {
                return PyFloat_FromDouble(get(self->mat, (int) begin0, (int) begin1));
            }

            temp->shape = get_shape(rows, cols);
            allocate_matrix_ref(newTest, self->mat, (int) begin0, (int) begin1, (int) rows, cols);
            temp->mat = *newTest;
            return temp;
            
        }
        //SLICE LONG
        if (PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PySlice_Type) && PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PyLong_Type)) {
            //TODO
            PyObject* slice0 = PyTuple_GET_ITEM(key, 0);
            int colNum = PyLong_AsLong(PyTuple_GET_ITEM(key, 1));
            Py_ssize_t length0 = rowDim;
            Py_ssize_t begin0 = 0; Py_ssize_t stop0 = 0; Py_ssize_t step0 = 0; Py_ssize_t sliceLength0 = 0;
            int ret = PySlice_GetIndicesEx(slice0, length0, &begin0, &stop0, &step0, &sliceLength0);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into slice\n");
                return NULL;
            }

            if (sliceLength0 == 0 || step0 != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return NULL;
            }

            rows = sliceLength0;
            cols = 1;
            if (colNum >= self->mat->cols || colNum < 0) {
                PyErr_SetString(PyExc_IndexError, "col is out of bounds \n");
                return NULL;
            }
            //Case for a single integer being returned
            if (rows == 1) {
                return PyFloat_FromDouble(get(self->mat, (int) begin0, (int) colNum));
            }
            
            temp->shape = get_shape(rows, cols);
            allocate_matrix_ref(newTest, self->mat, (int) begin0, (int) colNum, (int) rows, cols);
            temp->mat = *newTest;
            return temp;
        }
        //LONG SLICE
        if (PyObject_TypeCheck(PyTuple_GetItem(key, 1), &PySlice_Type) && PyObject_TypeCheck(PyTuple_GetItem(key, 0), &PyLong_Type)) {
            PyObject* slice0 = PyTuple_GetItem(key, 1);
            int rowNum = PyLong_AsLong(PyTuple_GetItem(key, 0));
            if(rowNum > rowDim) {
                PyErr_SetString(PyExc_IndexError, "Row out of bounds\n");
                return NULL;
            }
            
            Py_ssize_t length0 = colDim;
            Py_ssize_t begin0 = 0; Py_ssize_t stop0 = 0; Py_ssize_t step0 = 0; Py_ssize_t sliceLength0 = 0;
            int ret = PySlice_GetIndicesEx(slice0, length0, &begin0, &stop0, &step0, &sliceLength0);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into slice\n");
                return NULL;
            }

            if (sliceLength0 == 0 || step0 != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return NULL;
            }

            rows = 1;
            cols = sliceLength0;
            if (rowNum >= self->mat->rows || rowNum < 0) {
                PyErr_SetString(PyExc_IndexError, "row is out of bounds \n");
                return NULL;
            }
            //Case for a single integer being returned
            if (cols == 1) {
                return PyFloat_FromDouble(get(self->mat, (int) rowNum, (int) begin0));
            }
            
            temp->shape = get_shape(rows, cols);
            allocate_matrix_ref(newTest, self->mat, (int) rowNum, (int) begin0, rows, (int) cols);
            temp->mat = *newTest;
            return temp;
        }

    }
    PyErr_SetString(PyExc_TypeError, "You've Done messed up\n");
    return NULL;
}

/*
 * Given a numc.Matrix `self`, index into it with `key`, and set the indexed result to `v`.
 */
int Matrix61c_set_subscript(Matrix61c* self, PyObject *key, PyObject *v) {
    int rowDim = self->mat->rows;
    int colDim = self->mat->cols;
    
    //LONG
    if (PyObject_TypeCheck(key, &PyLong_Type)) {
        //1D CASES ONLY
        if (rowDim == 1) {
            if (!PyObject_TypeCheck(v, &PyFloat_Type) && !PyObject_TypeCheck(v, &PyLong_Type)) {
                PyErr_SetString(PyExc_TypeError, "Value is not valid\n");
                return -1;
            } 
            int colAdd = (int) PyLong_AsLong(key);
            double value = PyFloat_AsDouble(v);
            if (colAdd < 0 || colAdd >= colDim) {
                PyErr_SetString(PyExc_IndexError, "Value out of bounds\n");
                return -1;
            }
            set(self->mat, 0, colAdd, value);
            return 0;
        }
        if (colDim == 1) {
            if (!PyObject_TypeCheck(v, &PyFloat_Type) && !PyObject_TypeCheck(v, &PyLong_Type)) {
                PyErr_SetString(PyExc_TypeError, "Value is not valid");
                return -1;
            } 
            int rowAdd = (int) PyLong_AsLong(key);
            double value = PyFloat_AsDouble(v);
            if (rowAdd < 0 || rowAdd >= rowDim) {
                PyErr_SetString(PyExc_IndexError, "Value out of bounds");
                return -1;
            }
            set(self->mat, rowAdd, 0, value);
            return 0;
        }
        if (PyLong_AsLong(key) >= rowDim || PyLong_AsLong(key) < 0) {
            PyErr_SetString(PyExc_IndexError, "Value out of bounds");
            return -1;
        }
        if (!PyObject_TypeCheck(v, &PyList_Type)) {
            PyErr_SetString(PyExc_TypeError, "Type is invalid");
            return -1;
        }
        if (PyObject_TypeCheck(PyList_GetItem(v, 0), &PyList_Type)) {
            PyErr_SetString(PyExc_ValueError, "Value is invalid");
            return -1;
        }
        //printf("%d %d\n",PyList_GET_SIZE(v), colDim );
        if (PyList_GET_SIZE(v) != colDim) {
            PyErr_SetString(PyExc_ValueError, "Mismatched col values");
            return -1;
        }
        double val = 0;
        int rowVal = (int) PyLong_AsLong(key);
        for (int j = 0; j < colDim; j++) {
            val = PyFloat_AsDouble(PyList_GetItem(v, j));
            set(self->mat, rowVal, j, val);
        }
        return 0;
    }
    //SLICE
    if (PyObject_TypeCheck(key, &PySlice_Type)) {
        //1D CASES ONLY
        if (rowDim == 1) {
            PyObject* slice = key;
            Py_ssize_t length = colDim;
            Py_ssize_t begin = 0; Py_ssize_t stop = 0; Py_ssize_t step = 0; Py_ssize_t sliceLength = 0;
            int ret = PySlice_GetIndicesEx(slice, length, &begin, &stop, &step, &sliceLength);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into first slice\n");
                return -1;
            }
            if (sliceLength == 0 || step != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return -1;
            }
            if (sliceLength == 1) {
                if (!PyObject_TypeCheck(v, &PyFloat_Type) && !PyObject_TypeCheck(v, &PyLong_Type)) {
                    PyErr_SetString(PyExc_TypeError, "Invalid inputs to matrix value\n");
                    return -1;
                } 
                double value = PyFloat_AsDouble(v);
                set(self->mat, 0, begin, value);
                return 0;
            }
            if (!PyObject_TypeCheck(v, &PyList_Type)) {
                PyErr_SetString(PyExc_TypeError, "Value is not valid\n");
                return -1;
            }
            if (PyList_GET_SIZE(v) != sliceLength) {
                PyErr_SetString(PyExc_ValueError, "Size is not valid\n");
                return -1;
            }
            double val = 0;
            int count = 0;
            for (int j = begin; j < stop; j++) {
                val = PyFloat_AsDouble(PyList_GetItem(v, count));
                set(self->mat, 0, j, val);
                count += 1;
            }
            return 0;
            
        }
        if (colDim == 1) {
            PyObject* slice = key;
            Py_ssize_t length = rowDim;
            Py_ssize_t begin = 0; Py_ssize_t stop = 0; Py_ssize_t step = 0; Py_ssize_t sliceLength = 0;
            int ret = PySlice_GetIndicesEx(slice, length, &begin, &stop, &step, &sliceLength);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into first slice\n");
                return -1;
            }
            if (sliceLength == 0 || step != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return -1;
            }
            if (sliceLength == 1) {
                if (!PyObject_TypeCheck(v, &PyFloat_Type) && !PyObject_TypeCheck(v, &PyLong_Type)) {
                    PyErr_SetString(PyExc_TypeError, "Invalid inputs to matrix value\n");
                    return -1;
                } 
                double value = PyFloat_AsDouble(v);
                set(self->mat, 0, begin, value);
                return 0;
            }
            if (!PyObject_TypeCheck(v, &PyList_Type)) {
                PyErr_SetString(PyExc_TypeError, "Value is not valid\n");
                return -1;
            }
            if (PyList_GET_SIZE(v) != sliceLength) {
                PyErr_SetString(PyExc_ValueError, "Size is not valid\n");
                return -1;
            }
            double val = 0;
            int count = 0;
            for (int i = begin; i < stop; i++) {
                val = PyFloat_AsDouble(PyList_GetItem(v, count));
                set(self->mat, i, 0, val);
                count += 1;
            }
            return 0;
        }
        PyObject* slice = key;
        Py_ssize_t length = rowDim;
        Py_ssize_t begin = 0; Py_ssize_t stop = 0; Py_ssize_t step = 0; Py_ssize_t sliceLength = 0;
        int ret = PySlice_GetIndicesEx(slice, length, &begin, &stop, &step, &sliceLength);
        if (ret == -1) {
            PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into first slice\n");
            return -1;
        }
        if (sliceLength == 0 || step != 1) {
            PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
            return -1;
        }
        if (sliceLength == 1){
            if (PyList_GET_SIZE(v) != colDim) {
                PyErr_SetString(PyExc_ValueError, "Dimension of input is not valid\n");
                return -1;
            }
            for(int j = 0; j < colDim; j++) {
                PyObject* item = PyList_GetItem(v, j);
                if (!PyObject_TypeCheck(item, &PyFloat_Type) && !PyObject_TypeCheck(item, &PyLong_Type)) {
                    PyErr_SetString(PyExc_ValueError, "Value is not valid\n");
                    return -1;
                }
                double value = PyFloat_AsDouble(PyList_GetItem(v, j));
                set(self->mat, begin, j, value);
            }
            return 0;

        }

        if (PyList_GET_SIZE(v) != sliceLength) {
            PyErr_SetString(PyExc_ValueError, "Dimension of input is not valid\n");
            return -1;
        }

        //error check pass
        PyObject* tempList = NULL;
        for(int i = 0; i < sliceLength; i++) {
            tempList = PyList_GetItem(v, i);
            if (PyList_GET_SIZE(tempList) != colDim) {
                PyErr_SetString(PyExc_ValueError, "Dimension of cols is not valid\n");
                return -1;
            }
        }
        //setting values in matrix
        int count = 0;
        for(int i = begin; i < stop; i++) {
            PyObject* currList = PyList_GetItem(v, count);
            for(int j = 0; j < colDim; j++) {
                double value = PyFloat_AsDouble(PyList_GetItem(currList, j));
                set(self->mat, i, j, value);
            }
            count += 1;
        }
        return 0;
    }
    //TUPLE
    if (PyObject_TypeCheck(key, &PyTuple_Type)) {
        if (!PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PyLong_Type) && !PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PySlice_Type)) {
            PyErr_SetString(PyExc_ValueError, "First Entry is not a slice or integer");
            return -1;
        }
        if (!PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PyLong_Type) && !PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PySlice_Type)) {
            PyErr_SetString(PyExc_ValueError, "Second Entry is not a slice or integer");
            return -1;
        }
        if (rowDim == 1 || colDim == 1) {
            PyErr_SetString(PyExc_TypeError, "1D matrices only support single slice!\n");
            return -1;
        }
        //LONG LONG
        if (PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PyLong_Type) && PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PyLong_Type)){
            if (rowDim == 1 || colDim == 1) {
                PyErr_SetString(PyExc_TypeError, "1D matrices only support single slice!\n");
                return -1;
            }
            int givenRow = PyFloat_AsDouble(PyTuple_GET_ITEM(key, 0));
            int givenCol = PyFloat_AsDouble(PyTuple_GET_ITEM(key, 1));
            if (givenRow >= rowDim || givenCol >= colDim || givenRow < 0 || givenCol < 0) {
                PyErr_SetString(PyExc_IndexError, "Index out of range\n");
                return -1;
            }
            if (!PyObject_TypeCheck(v, &PyFloat_Type) && !PyObject_TypeCheck(v, &PyLong_Type)) {
                PyErr_SetString(PyExc_TypeError, "Value is not valid\n");
                return -1;
            } 
            set(self->mat, givenRow, givenCol, PyFloat_AsDouble(v));
            return 0;

        }
        //SLICE SLICE
        if (PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PySlice_Type) && PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PySlice_Type)){
            PyObject* slice0 = PyTuple_GET_ITEM(key, 0);
            Py_ssize_t length0 = rowDim;
            Py_ssize_t begin0 = 0; Py_ssize_t stop0 = 0; Py_ssize_t step0 = 0; Py_ssize_t sliceLength0 = 0;
            int ret = PySlice_GetIndicesEx(slice0, length0, &begin0, &stop0, &step0, &sliceLength0);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into first slice\n");
                return -1;
            }

            if (sliceLength0 == 0 || step0 != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return -1;
            }

            PyObject* slice1 = PyTuple_GET_ITEM(key, 1);
            Py_ssize_t length1 = colDim;
            Py_ssize_t begin1 = 0; Py_ssize_t stop1 = 0; Py_ssize_t step1 = 0; Py_ssize_t sliceLength1 = 0;
            ret = PySlice_GetIndicesEx(slice1, length1, &begin1, &stop1, &step1, &sliceLength1);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into second slice\n");
                return -1;
            }

            if (sliceLength1 == 0 || step1 != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return -1;
            }

            if (sliceLength0 == 1 && sliceLength1 == 1) {
                if (!PyObject_TypeCheck(v, &PyFloat_Type) && !PyObject_TypeCheck(v, &PyLong_Type)) {
                    PyErr_SetString(PyExc_TypeError, "Value type is not valid\n");
                    return -1;
                }
                set(self->mat, begin0, begin1, PyFloat_AsDouble(v));
                return 0;
            }
            if (!PyObject_TypeCheck(v, &PyList_Type)) {
                PyErr_SetString(PyExc_TypeError, "Type is not valid\n");
                return -1;
            }
            if (sliceLength0 == 1) {
                if (PyList_GET_SIZE(v) != sliceLength1) {
                    PyErr_SetString(PyExc_ValueError, "Dimension of input is not valid\n");
                    return -1;
                }
                for(int j = begin1; j < stop1; j++) {
                    PyObject* item = PyList_GET_ITEM(v, j);
                    if (!PyObject_TypeCheck(item, &PyFloat_Type) && !PyObject_TypeCheck(item, &PyLong_Type)) {
                        PyErr_SetString(PyExc_ValueError, "Value is not valid\n");
                        return -1;
                    }
                    double value = PyFloat_AsDouble(PyList_GET_ITEM(v, j));
                    set(self->mat, begin0, j, value);
                }
                return 0;
            }

            if (sliceLength1 == 1) {
                if (PyList_GET_SIZE(v) != sliceLength0) {
                    PyErr_SetString(PyExc_ValueError, "Dimension of input is not valid\n");
                    return -1;
                }
                for(int j = begin0; j < stop0; j++) {
                    PyObject* item = PyList_GetItem(v, j);
                    if (!PyObject_TypeCheck(item, &PyFloat_Type) && !PyObject_TypeCheck(item, &PyLong_Type)) {
                        PyErr_SetString(PyExc_ValueError, "Value is not valid\n");
                        return -1;
                    }
                    double value = PyFloat_AsDouble(PyList_GetItem(v, j));
                    set(self->mat,j, begin1, value);
                }
                return 0;
            }

            if (PyList_GET_SIZE(v) != sliceLength0) {
                PyErr_SetString(PyExc_ValueError, "Value dimensions are not valid\n");
                return -1;
            }
            int count = 0;
            for(int i = begin0; i < stop0; i++){
                PyObject* currList = PyList_GetItem(v, count);
                if (PyList_GET_SIZE(currList) != sliceLength1) {
                    PyErr_SetString(PyExc_ValueError, "Value dimensions are not valid\n");
                    return -1;
                }
                int internalCount = 0;
                for (int j = begin1; j < stop1; j++){
                    double value = PyFloat_AsDouble(PyList_GetItem(currList, internalCount));
                    set(self->mat, i, j, value);
                    internalCount += 1;
                }
                count += 1;
            }
            return 0;

        }
        //LONG SLICE
        if (PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PyLong_Type) && PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PySlice_Type)){
            int index = (int) PyLong_AsLong(PyTuple_GET_ITEM(key, 0));
            if(index < 0 || index >= rowDim) {
                PyErr_SetString(PyExc_IndexError, "Index out of Range!");
                return -1;
            }
            PyObject* slice = PyTuple_GET_ITEM(key, 1);
            Py_ssize_t length = colDim;
            Py_ssize_t begin = 0; Py_ssize_t stop = 0; Py_ssize_t step = 0; Py_ssize_t sliceLength = 0;
            int ret = PySlice_GetIndicesEx(slice, length, &begin, &stop, &step, &sliceLength);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into slice");
                return -1;
            }

            if (sliceLength == 0 || step != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return -1;
            }
            if (sliceLength == 1) {
                if (!PyObject_TypeCheck(v, &PyFloat_Type) && !PyObject_TypeCheck(v, &PyLong_Type)) {
                    PyErr_SetString(PyExc_TypeError, "this indexing combo requires a number not a list");
                    return -1;
                }
                set(self->mat,index, begin, (double) PyFloat_AsDouble(v));
                return 0;
            }
            if(!PyObject_TypeCheck(v, &PyList_Type)) {
                PyErr_SetString(PyExc_TypeError, "Value should be list");
                return -1;
            }
            if (PyList_GET_SIZE(v) != sliceLength) {
                PyErr_SetString(PyExc_ValueError, "Value dimensions are not valid");
                return -1;
            }
            int count = 0;
            //printf("%d %d\n", begin, stop);
            for(int j = begin; j < stop; j++) {
                PyObject* item = PyList_GetItem(v, count);
                if (!PyObject_TypeCheck(item, &PyFloat_Type) && !PyObject_TypeCheck(item, &PyLong_Type)) {
                    PyErr_SetString(PyExc_ValueError, "Value is not valid");
                    return -1;
                }
                double value = PyFloat_AsDouble(PyList_GetItem(v, count));
                set(self->mat,index, j, value);
                count++;
            }
            return 0;
        }
        //SLICE LONG
        if (PyObject_TypeCheck(PyTuple_GET_ITEM(key, 0), &PySlice_Type) && PyObject_TypeCheck(PyTuple_GET_ITEM(key, 1), &PyLong_Type)){
            int index = (int) PyLong_AsLong(PyTuple_GET_ITEM(key, 1));
            if(index < 0 || index >= colDim) {
                PyErr_SetString(PyExc_IndexError, "Index out of Range!");
                return -1;
            }
            PyObject* slice = PyTuple_GET_ITEM(key,0);
            Py_ssize_t length = rowDim;
            Py_ssize_t begin = 0; Py_ssize_t stop = 0; Py_ssize_t step = 0; Py_ssize_t sliceLength = 0;
            int ret = PySlice_GetIndicesEx(slice, length, &begin, &stop, &step, &sliceLength);
            if (ret == -1) {
                PyErr_SetString(PyExc_TypeError, "Incorrect type of stuff entered into slice");
                return -1;
            }

            if (sliceLength == 0 || step != 1) {
                PyErr_SetString(PyExc_ValueError, "Incorrect Slicing format or bounds or step size not 1\n");
                return -1;
            }
            if (sliceLength == 1) {
                if (!PyObject_TypeCheck(v, &PyFloat_Type) && !PyObject_TypeCheck(v, &PyLong_Type)) {
                    PyErr_SetString(PyExc_TypeError, "this indexing combo requires a number not a list");
                    return -1;
                }
                set(self->mat,begin, index, (double) PyFloat_AsDouble(v));
                return 0;
            }
            if(!PyObject_TypeCheck(v, &PyList_Type)) {
                PyErr_SetString(PyExc_TypeError, "Value should be list");
                return -1;
            }
            if (PyList_GET_SIZE(v) != sliceLength) {
                PyErr_SetString(PyExc_ValueError, "Value dimensions are not valid");
                return -1;
            }
            int count = 0;
            //printf("%d %d\n", begin, stop);
            for(int j = begin; j < stop; j++) {
                PyObject* item = PyList_GetItem(v, count);
                if (!PyObject_TypeCheck(item, &PyFloat_Type) && !PyObject_TypeCheck(item, &PyLong_Type)) {
                    PyErr_SetString(PyExc_ValueError, "Value is not valid");
                    return -1;
                }
                double value = PyFloat_AsDouble(PyList_GetItem(v, count));
                set(self->mat,j, index, value);
                count++;
            }
            return 0;
        }
    }
    return -1;

}

PyMappingMethods Matrix61c_mapping = {
    NULL,
    (binaryfunc) Matrix61c_subscript,
    (objobjargproc) Matrix61c_set_subscript,
};

/* INSTANCE ATTRIBUTES*/
PyMemberDef Matrix61c_members[] = {
    {
        "shape", T_OBJECT_EX, offsetof(Matrix61c, shape), 0,
        "(rows, cols)"
    },
    {NULL}  /* Sentinel */
};

PyTypeObject Matrix61cType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "numc.Matrix",
    .tp_basicsize = sizeof(Matrix61c),
    .tp_dealloc = (destructor)Matrix61c_dealloc,
    .tp_repr = (reprfunc)Matrix61c_repr,
    .tp_as_number = &Matrix61c_as_number,
    .tp_flags = Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,
    .tp_doc = "numc.Matrix objects",
    .tp_methods = Matrix61c_methods,
    .tp_members = Matrix61c_members,
    .tp_as_mapping = &Matrix61c_mapping,
    .tp_init = (initproc)Matrix61c_init,
    .tp_new = Matrix61c_new
};


struct PyModuleDef numcmodule = {
    PyModuleDef_HEAD_INIT,
    "numc",
    "Numc matrix operations",
    -1,
    Matrix61c_class_methods
};

/* Initialize the numc module */
PyMODINIT_FUNC PyInit_numc(void) {
    PyObject* m;

    if (PyType_Ready(&Matrix61cType) < 0)
        return NULL;

    m = PyModule_Create(&numcmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&Matrix61cType);
    PyModule_AddObject(m, "Matrix", (PyObject *)&Matrix61cType);
    printf("NumC Module Imported\n");
    fflush(stdout);
    return m;
}