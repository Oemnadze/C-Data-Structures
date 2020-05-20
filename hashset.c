#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0);
	assert(numBuckets > 0);
	assert(hashfn != NULL);
	assert(comparefn != NULL);
	h->elemSz = elemSize;
	h->bucketsCnt = numBuckets;
	h->hashFunc = hashfn;
	h->compareFunc = comparefn;
	h->freeFunc = freefn;
	h->logLen = 0;
	h->elems = malloc(sizeof(void*) * numBuckets);
	for (int i = 0; i < numBuckets; i++) {
		vector* vc = malloc(sizeof(vector));
		VectorNew(vc, elemSize, freefn, 0);
		h->elems[i] = vc;
	}
}

void HashSetDispose(hashset *h)
{
	for (int i = 0; i < h->bucketsCnt; i++) {
		VectorDispose(h->elems[i]);
		free(h->elems[i]);
	}
	free(h->elems);
}

int HashSetCount(const hashset *h)
{ 
	return h->logLen; 
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != NULL);
	for (int i = 0; i < h->bucketsCnt; i++)
		VectorMap(h->elems[i], mapfn, auxData);
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL);
	int bucketIt = h->hashFunc(elemAddr, h->bucketsCnt);
	assert(bucketIt >= 0 && bucketIt < h->bucketsCnt);
	if (VectorLength(h->elems[bucketIt]) != 0) {
		int pos = VectorSearch(h->elems[bucketIt], elemAddr, h->compareFunc, 0, 1);
		if (pos == -1) {
			VectorAppend(h->elems[bucketIt], elemAddr);
			h->logLen++;
		}
		else {
			VectorReplace(h->elems[bucketIt], elemAddr, pos);
		}
		VectorSort(h->elems[bucketIt], h->compareFunc);
	}
	else {
		VectorAppend(h->elems[bucketIt], elemAddr);
		h->logLen++;
	}
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ 
	assert(elemAddr != NULL);
	int bucketIt = h->hashFunc(elemAddr, h->bucketsCnt);
	assert(bucketIt >= 0 && bucketIt < h->bucketsCnt);
	if (VectorLength(h->elems[bucketIt]) == 0) {
		return NULL;
	}
	int pos = VectorSearch(h->elems[bucketIt], elemAddr, h->compareFunc, 0, 1);
	if (pos == -1) {
		return NULL;
	}
	return VectorNth(h->elems[bucketIt], pos);
}
