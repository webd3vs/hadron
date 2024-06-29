#include "array.h"

static void initArray(Array *, int);

Array *newArray(int s) {
  Array *p = malloc(sizeof(Array));
  initArray(p, s);
  return p;
}

void initArray(Array *a, int s) {
  int size  = s < 2 ? 2 : s;
  a->array  = malloc(sizeof(void *) * size);
  a->length = 0;
  a->size   = size;
}

void trimArray(Array *a) {
  a->array = realloc(a->array, sizeof(void *) * a->length);
  a->size  = a->length;
}

void pushArray(Array *a, void *el) {
  if (a->size == a->length) {
    a->size *= 1.5;
    a->array = realloc(a->array, sizeof(void *) * a->size);
  };
  a->array[a->length++] = el;
}

void removeArray(Array *a, int i) {
  if (i < 0 && i >= a->length) return;
  a->array[i] = NULL;
}

Array *clearArray(Array *a) {
  Array *r = newArray(a->size);
  for (int i = 0; i < a->length; i++) {
    void *x = a->array[i];
    if (x != NULL) pushArray(r, x);
  };
  return r;
}

// @warning `array` will be unusable after this point
void freeArray(Array *a) {
  for (int i = 0; i < a->length; i++)
    free(a->array[i]);
  free(a->array);
  free(a);
}

void *popArray(Array *a) {
  return a->length > 0 ? a->array[--a->length] : NULL;
}

void *lastArray(Array *a) {
  return a->length > 0 ? a->array[a->length - 1] : NULL;
}

void *getArray(Array *a, int i) {
  return i >= 0 && i < a->length ? a->array[i] : NULL;
}
