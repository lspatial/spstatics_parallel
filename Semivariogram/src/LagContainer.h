/* 
 * File:   LagContainer.h
 * Author: ricepig
 *
 * Created on 2013年12月8日, 下午11:31
 */

#ifndef LAGCONTAINER_H
#define	LAGCONTAINER_H

#ifdef	__cplusplus
extern "C" {
#endif

struct LagContainer
{
    double* sums;
    int* counts;
    size_t size;
};

void LCInit(struct LagContainer * lc, size_t size)
{
    lc->sums = (double*)malloc(sizeof(double)*size);
    memset(lc->sums, 0, sizeof(double)*size);
    lc->counts = (int*)malloc(sizeof(int)*size);
    memset(lc->counts, 0, sizeof(int)*size);
    lc->size = size;
}

void LCDestory(struct LagContainer *lc)
{
    if(lc->sums != NULL){
        free(lc->sums);
        lc->sums = NULL;
    }
    
    if(lc->counts != NULL){
        free(lc->counts);
        lc->counts = NULL;
    }
    lc->size = 0;
}

void LCCalcAverage(struct LagContainer *lc)
{
    double* s = lc->sums;
    int* c = lc->counts;
    for(size_t i=0;i<lc->size;i++,s++,c++)
        *s /= *c;
}

#ifdef	__cplusplus
}
#endif

#endif	/* LAGCONTAINER_H */

