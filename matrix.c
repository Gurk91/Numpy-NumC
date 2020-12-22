#include "matrix.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

// Include SSE intrinsics
#if defined(_MSC_VER)
#include <intrin.h>
#elif defined(__GNUC__) && (defined(__x86_64__) || defined(__i386__))
#include <immintrin.h>
#include <x86intrin.h>
#endif

/* Below are some intel intrinsics that might be useful
 * void _mm256_storeu_pd (double * mem_addr, __m256d a)
 * __m256d _mm256_set1_pd (double a)
 * __m256d _mm256_set_pd (double e3, double e2, double e1, double e0)
 * __m256d _mm256_loadu_pd (double const * mem_addr)
 * __m256d _mm256_add_pd (__m256d a, __m256d b)
 * __m256d _mm256_sub_pd (__m256d a, __m256d b)
 * __m256d _mm256_fmadd_pd (__m256d a, __m256d b, __m256d c)
 * __m256d _mm256_mul_pd (__m256d a, __m256d b)
 * __m256d _mm256_cmp_pd (__m256d a, __m256d b, const int imm8)
 * __m256d _mm256_and_pd (__m256d a, __m256d b)
 * __m256d _mm256_max_pd (__m256d a, __m256d b)
*/

/*
 * Generates a random double between `low` and `high`.
 */
double rand_double(double low, double high) {
    double range = (high - low);
    double div = RAND_MAX / range;
    return low + (rand() / div);
}

/*
 * Generates a random matrix with `seed`.
 */
void rand_matrix(matrix *result, unsigned int seed, double low, double high) {
    srand(seed);
    for (int i = 0; i < result->rows; i++) {
        for (int j = 0; j < result->cols; j++) {
            set(result, i, j, rand_double(low, high));
        }
    }
}

/*
 * Allocate space for a matrix struct pointed to by the double pointer mat with
 * `rows` rows and `cols` columns. You should also allocate memory for the data array
 * and initialize all entries to be zeros. Remember to set all fieds of the matrix struct.
 * `parent` should be set to NULL to indicate that this matrix is not a slice.
 * You should return -1 if either `rows` or `cols` or both have invalid values, or if any
 * call to allocate memory in this function fails. If you don't set python error messages here upon
 * failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
 int allocate_matrix(matrix **mat, int rows, int cols) {
    if (rows <= 0 || cols <= 0) {
        PyErr_SetString(PyExc_ValueError, "Matrix row or col value received invalid input");
        return -1;
    } 
    *(mat) = (matrix*) malloc(sizeof(matrix));
    if (*(mat) ==  NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc of *(mat) failed");
        return -1;
    }
    (*(mat))->data = malloc(rows * sizeof(double*));
    if ((*(mat))->data ==  NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc of *(mat)->data failed");
        return -1;
    }
    *((*(mat))->data) = (double *) calloc(1, rows * cols * sizeof(double));
    if (*((*(mat))->data) == NULL) {
        free((*(mat))->data);
        free((*(mat)));
        PyErr_SetString(PyExc_RuntimeError, "Malloc of *(mat)->data cols failed");
        return -1;
    }
    for (int i = 0; i < rows; i++){
        *((*(mat))->data + i) = (*((*(mat))->data) + cols * i);
    }

    (*(mat))->rows = rows;
    (*(mat))->cols = cols;
    if(rows == 1 || cols == 1) {
        (*(mat))->is_1d = 1;
    } else {
        (*(mat))->is_1d = 0;
    } 
    (*(mat))->ref_cnt = 1;
    (*(mat))->parent = NULL;
    return 0;
}

/*
 * Allocate space for a matrix struct pointed to by `mat` with `rows` rows and `cols` columns.
 * This is equivalent to setting the new matrix to be
 * from[row_offset:row_offset + rows, col_offset:col_offset + cols]
 * If you don't set python error messages here upon failure, then remember to set it in numc.c.
 * Return 0 upon success and non-zero upon failure.
 */
int allocate_matrix_ref(matrix **mat, matrix *from, int row_offset, int col_offset, int rows, int cols) {
    if (row_offset + rows > from->rows || col_offset + cols > from->cols) {
        PyErr_SetString(PyExc_RuntimeError, "Matrix Range Out of Bounds.");
        return -1;
    }
    *(mat) = (matrix *) malloc(sizeof(matrix));
    if (*(mat) ==  NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc of *(mat) failed");
        return -1;
    }
    (*(mat))->data = malloc(rows * sizeof(double*));
    if ((*(mat))->data ==  NULL) {
        PyErr_SetString(PyExc_RuntimeError, "Malloc of *(mat)->data failed");
        return -1;
    }
    //int fromCol = from->cols;
    for(int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            *((*(mat))->data + i) = (*(from->data + i + row_offset) + col_offset);
            //printf("%f\n", *(*(from->data + i + row_offset) + col_offset));
        }
    }
    (*(mat))->rows = rows;
    (*(mat))->cols = cols;
    if(rows == 1 || cols == 1) {
        (*(mat))->is_1d = 1;
    } else {
        (*(mat))->is_1d = 0;
    } 
    (*(mat))->ref_cnt = 1;
    (*(mat))->parent = from;
    from->ref_cnt += 1;
    return 0;

}

/*
 * This function will be called automatically by Python when a numc matrix loses all of its
 * reference pointers.
 * You need to make sure that you only free `mat->data` if no other existing matrices are also
 * referring this data array.
 * See the spec for more information.
 */
void deallocate_matrix(matrix *mat) {
    if (mat == NULL) {
        return;
    }
    if (mat->ref_cnt > 1 && mat->parent == NULL) {
        mat->ref_cnt--;
        return;
    } 
    if (mat->parent != NULL) {
        mat->parent->ref_cnt -= 1;
        free(mat->data);
        free(mat);
        return;
    }
    if (mat->parent == NULL && mat->ref_cnt == 1) {
        free(*((mat)->data));
        free((mat)->data );
        free(mat);
    }
}

/*
 * Return the double value of the matrix at the given row and column.
 * You may assume `row` and `col` are valid.
 */
/*double get(matrix *mat, int row, int col) {
    return (*(mat->data))[(col * row) + col];
}*/
double get(matrix *mat, int row, int col) {
    return (mat->data)[row][col];
}

/*
 * Set the value at the given row and column to val. You may assume `row` and
 * `col` are valid
 */
/*void set(matrix *mat, int row, int col, double val) {
    //*(*(mat->data + row) + col) = val;
    int cols = mat->cols;
    (*(mat->data))[(cols * row)+ col] = val;
}*/
void set(matrix *mat, int row, int col, double val) {
    (mat->data)[row][col] = val;
}

/*
 * Set all entries in mat to val
 */
void fill_matrix(matrix *mat, double val) {
    int cols = mat->cols;
    int rows = mat->rows;

    __m256d vec = _mm256_set1_pd (val);

    //#pragma omp parallel for
    for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
        _mm256_storeu_pd((*(mat->data) + i), vec);
    }
    
    if ((rows * cols) % 4 != 0) {
        for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
            mat->data[0][i] = val;
        }
    }
}

/*
 * Store the result of adding mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int add_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
        PyErr_SetString(PyExc_ValueError, "Dimensions of matricies don't match!");
        return -1;
    }
    int rows = mat1->rows;
    int cols = mat1->cols;

    __m256d mat1Res;
    __m256d mat2Res;
    __m256d resRes;
    
    #pragma omp parallel for private(mat1Res, mat2Res, resRes)
    for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
        mat1Res = _mm256_loadu_pd((*(mat1->data) + i));
        mat2Res = _mm256_loadu_pd((*(mat2->data) + i));
        resRes = _mm256_add_pd(mat1Res, mat2Res);
        _mm256_storeu_pd((*(result->data) + i), resRes);
    }
    if ((rows * cols) % 4 != 0) {
        for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
	        result->data[0][i] = mat1->data[0][i] + mat2->data[0][i];
        }
    }

    return 0;
}

/*
 * Store the result of subtracting mat2 from mat1 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int sub_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    if (mat1->rows != mat2->rows || mat1->cols != mat2->cols) {
        return -1;
    }

    int rows = mat1->rows;
    int cols = mat1->cols;

    __m256d mat1Res;
    __m256d mat2Res;
    __m256d resRes;
    #pragma omp parallel for private(mat1Res, mat2Res, resRes)
    for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
        mat1Res = _mm256_loadu_pd((*(mat1->data) + i));
        mat2Res = _mm256_loadu_pd((*(mat2->data) + i));
        resRes = _mm256_sub_pd(mat1Res, mat2Res);
         _mm256_storeu_pd((*(result->data) + i), resRes);

    }
    if ((rows * cols) % 4 != 0) {
        for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
	        result->data[0][i] = mat1->data[0][i] - mat2->data[0][i];
        }
    }

    return 0;
}

/*
 * Store the result of multiplying mat1 and mat2 to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 * Remember that matrix multiplication is not the same as multiplying individual elements.
 */
 int mul_matrix(matrix *result, matrix *mat1, matrix *mat2) {
    int i,j,k;
    int mat1rows = mat1->rows;
    int mat2cols = mat2->cols;

    if ((mat1rows < 8) && (mat2cols < 8)) {
        for(i = 0; i < mat1->rows; i++) {
            for(k = 0; k < mat2->rows; k++) {
                for(j = 0; j < mat2->cols; j++) {
                    result->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
                }
            }
        }
        return 0;
    }

    #pragma omp parallel for
    for(i = 0; i < mat1->rows; i++) {
        for(k = 0; k < ((mat2->rows) / 4) * 4; k+= 4) {
            for(j = 0; j < mat2->cols; j++) {
                #pragma critical
                result->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
                result->data[i][j] += mat1->data[i][k + 1] * mat2->data[k + 1][j];
                result->data[i][j] += mat1->data[i][k + 2] * mat2->data[k + 2][j];
                result->data[i][j] += mat1->data[i][k + 3] * mat2->data[k + 3][j];
            }
        }
        for (k = ((mat2->rows) / 4) * 4; k < mat2->rows; k++){
            for(j = 0; j < mat2->cols; j++){
                #pragma critical
                result->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
            }
        }
    }
    return 0;
}

int mul_pow(matrix *temp, matrix *result, matrix *mat1, matrix *mat2){
    int i,j,k;
    /* This is jki loop order. */
    int rows = mat1->rows;
    int cols = mat1->cols;
    if (rows < 8){
        for(i = 0; i < rows; i++) {
            for(k = 0; k < rows; k++) {
                for(j = 0; j < cols; j++) {
                    temp->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
                }
            }
        }
        for (int i = 0; i < rows * cols; i++){
            result->data[0][i] = temp->data[0][i];
        } 
        return 0;
    }

    #pragma omp parallel for
    for(i = 0; i < rows; i++) {
        for(k = 0; k < (rows / 4) * 4; k+= 4) {
            for(j = 0; j < cols; j++) {
                #pragma critical
                temp->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
                temp->data[i][j] += mat1->data[i][k + 1] * mat2->data[k + 1][j];
                temp->data[i][j] += mat1->data[i][k + 2] * mat2->data[k + 2][j];
                temp->data[i][j] += mat1->data[i][k + 3] * mat2->data[k + 3][j];
            }
        }
        for (k = (rows/ 4) * 4; k < mat2->rows; k++){
            for(j = 0; j < cols; j++){
                #pragma critical
                temp->data[i][j] += mat1->data[i][k] * mat2->data[k][j];
            }
        }
    }

    #pragma omp parallel for
    for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
        _mm256_storeu_pd((*(result->data) + i), _mm256_loadu_pd((*(temp->data) + i)));
    }
        
    if ((rows * cols) % 4 != 0) {
        for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
            result->data[0][i] = temp->data[0][i];
        }
    } 

    //start 0 out
    return 0;
}



int pow_matrix(matrix *result, matrix *mat, int pow) {
    //printf("pow: %d\n", pow);
    if (pow == 0) {
      for (int i = 0; i < mat->rows; i++){
        result->data[i][i] = 1;
      }
      return 0;
    }
    if (pow == 2) {
        mul_matrix(result, mat, mat);
        return 0;
    }

    int rows = result->rows;
    int cols = result->cols;

    if (pow == 1) {
        #pragma omp parallel for
        for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
            _mm256_storeu_pd((*(result->data) + i), _mm256_loadu_pd((*(mat->data) + i)));
        }
        
        if ((rows * cols) % 4 != 0) {
            for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
                result->data[0][i] = mat->data[0][i];
            }
        } 

        return 0;
    }

    matrix *toot;
    matrix **temp = &toot;
    allocate_matrix(temp, rows, cols);

    matrix *good;
    matrix **squared = &good;
    allocate_matrix(squared, rows, cols);
    for (int i = 0; i < rows * cols; i++){
        (*squared)->data[0][i] = mat->data[0][i];
    }

    int lastBit = pow & 1;
    pow = pow >> 1;
    int currBit = pow & 1;
    if (lastBit == 0){
        #pragma omp parallel for
        for (int i = 0; i < rows * cols; i++){
            result->data[0][i] = 0;
        } 
        #pragma omp parallel for
        for (int i = 0; i < rows; i++){
            result->data[i][i] = 1;
        }

    } else {
        #pragma omp parallel for
        for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
            _mm256_storeu_pd((*(result->data) + i), _mm256_loadu_pd((*(mat->data) + i)));
        }
        
        if ((rows * cols) % 4 != 0) {
            for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
                result->data[0][i] = mat->data[0][i];
            }
        } 
 
    }
    __m256d vec = _mm256_set1_pd(0);
    while (pow != 0 || currBit != 0) {
        mul_pow((*temp), (*squared), (*squared), (*squared));
        //fill_matrix((*temp), 0);
        //-----------------------fill start-------------------------------------
        #pragma omp parallel for
        for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
            _mm256_storeu_pd((*((*temp)->data) + i), vec);
        }
        
        if ((rows * cols) % 4 != 0) {
            for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
                (*temp)->data[0][i] = 0;
            }
        }
        //-----------------------fill end----------------------------------
        if (currBit == 1) {
            mul_pow((*temp), result, result, (*squared));
            //fill_matrix((*temp), 0);
            #pragma omp parallel for
            for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
                _mm256_storeu_pd((*((*temp)->data) + i), vec);
            }
            
            if ((rows * cols) % 4 != 0) {
                for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
                    (*temp)->data[0][i] = 0;
                }
            }
        }//end currbit 1 case
        pow = pow >> 1;
        currBit = pow & 1;
        
    }
    deallocate_matrix((*squared));
    deallocate_matrix((*temp));

    return 0;
}

/*
 * Store the result of element-wise negating mat's entries to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int neg_matrix(matrix *result, matrix *mat) {
    if (result->rows != mat->rows || result->cols != mat->cols) {
        return -1;
    }
    int rows = mat->rows;
    int cols = mat->cols;
    if (rows < 8 && cols < 8){
        for (int i = 0; i < rows * cols; i++){
            result->data[0][i] = mat->data[0][i] * -1;
        }
        return 0;
    }

    __m256d negative =_mm256_set1_pd (-1);
    
    #pragma omp parallel for
    for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
        _mm256_storeu_pd((*(result->data) + i), _mm256_mul_pd(_mm256_loadu_pd((*(mat->data) + i)), negative));
    }
    
    if ((rows * cols) % 4 != 0) {
        for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
            result->data[0][i] = mat->data[0][i] * -1;
        }
    } 
 
    return 0;
}

/*
 * Store the result of taking the absolute value element-wise to `result`.
 * Return 0 upon success and a nonzero value upon failure.
 */
int abs_matrix(matrix *result, matrix *mat) {
    if (result->rows != mat->rows || result->cols != mat->cols) {
        return -1;
    }
    int rows = mat->rows;
    int cols = mat->cols;

    if (rows < 8 && cols < 8){
        for (int i = 0; i < rows*cols; i++){
            if (mat->data[0][i] >= 0) {
                result->data[0][i] = mat->data[0][i];
            } else {
                result->data[0][i] = -1 * mat->data[0][i];
            }
        }
        return 0;
    }
    
    __m256d zeros =_mm256_set1_pd (-0.);
    __m256d values;

    #pragma omp parallel for
    for (int i = 0; i < ((rows * cols) / 4) * 4; i += 4) {
        values = _mm256_loadu_pd((*(mat->data) + i));
        _mm256_storeu_pd((*(result->data) + i), _mm256_max_pd(_mm256_sub_pd(zeros,values), values));
    }
    if ((rows * cols) % 4 != 0) {
        for (int i = ((rows * cols) / 4) * 4; i < (rows * cols); i++) {
            if (mat->data[0][i] >= 0) {
                result->data[0][i] = mat->data[0][i];
            }
            else {
                result->data[0][i] = -1 * mat->data[0][i];
            }
        }
    }
    return 0;
    /*
    for(int i = 0; i < mat->rows; i++) {
        for (int j = 0; j < mat->cols; j++) {
            if (*(*(mat->data + i) + j) >= 0) {
                *(*(result->data + i) + j) = *(*(mat->data + i) + j);
            } else {
                *(*(result->data + i) + j) = *(*(mat->data + i) + j) * -1;
            }
        }
    }
    */
}