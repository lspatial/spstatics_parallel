/* 
 * File:   ParallelInversion.h
 * Author: ricepig
 *
 * Created on 2013年12月9日, 上午2:38
 */

#ifndef PARALLELINVERSION_H
#define	PARALLELINVERSION_H

#ifdef	__cplusplus
extern "C" {
#endif

void invert(double* matrix, int order, int group_size, int my_rank);


#ifdef	__cplusplus
}
#endif

#endif	/* PARALLELINVERSION_H */

