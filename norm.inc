/*
  This file is part of Open Dose.

  Copyright (C) 2019 Elliot Nunn

  This program is free software: you can redistribute it and/or  modify
  it under the terms of the GNU Affero General Public License, version 3,
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Affero General Public License for more details.

  You should have received a copy of the GNU Affero General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "prng.inc"

/* Draw a single number from the standard normal distribution */
/* Marsaglia algorithm */
double norm(struct prng64_state *prng)
{
    double x, y, rsq, f;
    do {
        x = 2.0 * prng64(prng) / (double)UINT64_MAX - 1.0;
        y = 2.0 * prng64(prng) / (double)UINT64_MAX - 1.0;
        rsq = x * x + y * y;
    } while(rsq >= 1. || rsq == 0.);
    f = sqrt(-2.0 * log(rsq) / rsq);
    return x * f;
    //discard y * f;
}

/* The Cholesky decomposition of the covariance matrix is applied to impose correlation on a vector */
/* Lower triangular */
void cholesky(const double *A, double *L, size_t n)
{
    memset(L, 0, n * n * sizeof(*L));
 
    for (size_t i = 0; i < n; i++)
        for (size_t j = 0; j < (i+1); j++) {
            double s = 0;

            for (size_t k = 0; k < j; k++) {
                s += L[i * n + k] * L[j * n + k];
            }

            L[i * n + j] = (i == j) ?
                sqrt(A[i * n + i] - s) :
                (1.0 / L[j * n + j] * (A[i * n + j] - s));
        }
}

void mvnorm(struct prng64_state *prng, const double *omega, double *ary, size_t dim)
{
    /* Initially, a vector of samples from the standard normal */
    for (size_t i = 0; i < dim; i++) {
        ary[i] = norm(prng);
    }

    /* Take the Cholesky transform of omega (please compiler please partial) */
    double chol[dim * dim];
    cholesky(omega, chol, dim);

    /* Dirty work of matrix multiplication */
    double ary2[dim];

    for (size_t row = 0; row < dim; row++) {
        ary2[row] = 0.0;
        for (size_t col = 0; col < dim; col++) {
            ary2[row] += ary[col] * chol[dim * row + col];
        }
    }

    memcpy(ary, ary2, dim * sizeof(*ary));
}
