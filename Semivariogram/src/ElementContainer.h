/* 
 * File:   ElementContainer.h
 * Author: ricepig
 *
 * Created on 2013年12月8日, 上午3:13
 */

#ifndef ELEMENTCONTAINER_H
#define	ELEMENTCONTAINER_H

#define INIT_SIZE 1024

#ifdef	__cplusplus
extern "C" {
#endif

struct Element {
    float X;
    float Y;
    float Value;
};

struct ElementContainer {
    struct Element* Head;
    size_t Size;
    size_t Length;
};

void ECInit(struct ElementContainer* ec)
{
    ec->Head = (struct Element *)malloc(sizeof(Element) * INIT_SIZE);
    ec->Size = INIT_SIZE;
    ec->Length = 0;
}

void ECInitWithSize(struct ElementContainer* ec, size_t size)
{
    ec->Head = (struct Element *)malloc(sizeof(Element) * size);
    ec->Size = size;
    ec->Length = 0;
}

static void ECRealloc(struct ElementContainer* ec)
{
    struct Element* dest = (struct Element*)malloc(sizeof(struct Element) * ec->Size * 2);
    memcpy(dest, ec->Head, sizeof(struct Element) * ec->Size);
    free(ec->Head);
    ec->Head = dest;
    ec->Size *= 2;
}

void ECAdd(struct ElementContainer* ec, float x, float y, float value)
{
    if(ec->Size==ec->Length)
        ECRealloc(ec);

    struct Element* tmp = ec->Head + ec->Length;
    tmp->X = x;
    tmp->Y = y;
    tmp->Value = value;
    ec->Length++;
}

void ECDestory(struct ElementContainer* ec)
{
    if(ec->Head != NULL){
        free(ec->Head);
        ec->Head = NULL;
        ec->Size = 0;
        ec->Length = 0;
    }
}

#ifdef	__cplusplus
}
#endif

#endif	/* ELEMENTCONTAINER_H */

