/* 
 * File:   GuassianElimination.h
 * Author: ricepig
 *
 */

#ifndef EPS
#define EPS 0.00000001
#endif

#ifndef GUASSIANELIMINATION_H
#define	GUASSIANELIMINATION_H

int forward_substitution(double** a, int n) 
{
    int i, j, k, max;
    double t,s;
    for (i = 0; i < n; ++i) {
        // find the row whose i'th element is max in the column
        max = i;
        for (j = i + 1; j < n; ++j)
            if (a[j][i] > a[max][i])
                max = j;
        // swap the max row and the first row;
        if(max != i){
            for (j = 0; j < n + 1; ++j) {
                t = a[max][j];
                a[max][j] = a[i][j];
                a[i][j] = t;
            }
        }

        s = a[i][i];
        if(s<EPS && s>-EPS) 
            return 1;

        // substitute
        for (j = n; j >= i; --j)
            for (k = i + 1; k < n; ++k)
                a[k][j] -= a[k][i]/s * a[i][j];
    }
    return 0;
}

void reverse_elimination(double** a, int n) 
{
    int i, j;
    for (i = n - 1; i >= 0; --i) {
        a[i][n] = a[i][n] / a[i][i];
        a[i][i] = 1;

        for (j = i - 1; j >= 0; --j) {
            a[j][n] -= a[j][i] * a[i][n];
            a[j][i] = 0;
        }
    }
}

int guass_eliminator(double** a, int n)
{
    if(forward_substitution(a, n) == 1) return 1;
    reverse_elimination(a, n);
    return 0;
}


#endif	/* GUASSIANELIMINATION_H */

