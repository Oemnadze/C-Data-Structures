#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int maxInt(int a, int b) {
	if (a > b)
		return a;
	return b;
}

void grow(vector* v) {
	v->elems = realloc(v->elems, v->allocLen * 2 * v->elemSz);
	assert(v->elems != NULL);
	v->allocLen *= 2;
}

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
	assert(elemSize > 0);
	assert(initialAllocation >= 0);
	v->elemSz = elemSize;
	v->allocLen = initialAllocation;
	if (initialAllocation == 0)
		v->allocLen = 4;
	v->logLen = 0;
	v->memoryCnt = 0;
	v->freeFunc = freeFn;
	v->elems = malloc(elemSize * v->allocLen);
	assert(v->elems != NULL);
}

void VectorDispose(vector *v)
{
	if (v->freeFunc != NULL) {
		for (int i = 0; i < v->memoryCnt; i++) {
			v->freeFunc((char*)v->elems + i * v->elemSz);
		}
	}
	free(v->elems);
}

int VectorLength(const vector *v)
{ 
	return v->logLen; 
}

void *VectorNth(const vector *v, int position)
{ 
	assert(position < v->logLen);
	assert(position >= 0);
	return (char*)v->elems + position * v->elemSz; 
}

void VectorReplace(vector *v, const void *elemAddr, int position)
{
	assert(position < v->logLen);
	assert(position >= 0);
	void* cur = VectorNth(v, position);
	memcpy(cur, elemAddr, v->elemSz);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
	assert(position <= v->logLen);
	assert(position >= 0);
	if (position == v->logLen) {
		VectorAppend(v, elemAddr);
		return;
	}
	if (v->allocLen == v->logLen) {
		grow(v);
	}
	memmove((void*)((char*)v->elems + (position + 1) * v->elemSz), (void*)((char*)v->elems + position * v->elemSz), (v->logLen - position) * v->elemSz);
	memcpy((void*)((char*)v->elems + position * v->elemSz), elemAddr, v->elemSz);
	v->logLen++;
	v->memoryCnt = maxInt(v->memoryCnt, v->logLen);
}

void VectorAppend(vector *v, const void *elemAddr)
{
	if (v->allocLen == v->logLen) {
		grow(v);
	}
	memcpy((char*)v->elems + v->logLen * v->elemSz, elemAddr, v->elemSz);
	v->logLen++;
	v->memoryCnt = maxInt(v->memoryCnt, v->logLen);
}

void VectorDelete(vector *v, int position)
{
	assert(v->logLen > 0);
	assert(position < v->logLen);
	assert(position >= 0);
	void* cur = VectorNth(v, position);
	if (v->freeFunc != NULL)
		v->freeFunc(cur);
	memmove(cur, (void*)((char*)cur + v->elemSz), (v->logLen - position - 1) * v->elemSz);
	v->logLen--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
	assert(compare != NULL);
	qsort(v->elems, v->logLen, v->elemSz, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
	assert(mapFn != NULL);
	for (int i = 0; i < v->logLen; i++)
		mapFn((char*)v->elems + i * v->elemSz, auxData);
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
	assert(startIndex >= 0);
	assert(startIndex < v->logLen);
	assert(key != NULL);
	assert(searchFn != NULL);
	if (isSorted) {
		char* ptr = bsearch(key, (char*)v->elems + startIndex * v->elemSz, v->logLen - startIndex - 1, v->elemSz, searchFn);
		if (ptr == NULL)
			return kNotFound;
		return (ptr - (char*)v->elems) / v->elemSz;
	}
	for (int i = startIndex; i < v->logLen; i++) {
		if (searchFn((char*)v->elems + i * v->elemSz, key) == 0) 
			return i;
	}
	return kNotFound;
} 
