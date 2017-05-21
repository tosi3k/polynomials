/** @file
   Implementacja wektora wektorów jednomianów

   @author Antoni Zawodny <az337756@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-22
*/

#include <assert.h>
#include <stdio.h>
#include "poly.h"

/**
 * Definicja wektora jednomianów.
 * Struktura Vector przechowuje dynamicznie alokowaną tablicę jednomianów,
 * która zmniejsza się i zwiększa w zamortyzowanym czasie stałym (tak, jak
 * vector w C++).
 */
typedef struct Vector {
    Mono* monos; ///< tablica jednomianów
    unsigned size; ///< liczba jednomianów w tablicy monos
    unsigned maxSize; ///< faktyczny rozmiar tablicy monos
} Vector;

/**
 * Zwraca pusty wektor jednomianów.
 * @return pusty wektor
 */
Vector VectorCreate() {
    return (Vector) {.monos = NULL, .size = 0, .maxSize = 1};
}

/**
 * Sprawdza czy wektor @p v jest pusty.
 * @param[in] v : wektor
 * @return czy wektor @p v jest pusty?
 */
inline bool VectorIsEmpty(const Vector *v) {
    return (v->size == 0);
}

/**
 * Dodaje jednomian @p m na koniec wektora @p v.
 * Wektor @p v przejmuje na własność jednomian @p m.
 * @param[in,out] v : wektor
 * @param[in] m : jednomian
 */
void VectorAddMono(Vector *v, Mono m) {
    if (VectorIsEmpty(v)) {
        v->monos = malloc(sizeof(Mono));
        assert(v->monos != NULL);

        v->monos[0] = m;
    } else if (v->size == v->maxSize) {
        v->maxSize *= 2;
        Mono *newMonos = malloc(sizeof(Mono) * v->maxSize);
        assert(newMonos != NULL);

        for (unsigned j = 0; j < v->size; ++j) {
            newMonos[j] = v->monos[j];
        }

        newMonos[v->size] = m;
        free(v->monos);
        v->monos = newMonos;
    } else {
        v->monos[v->size] = m;
    }

    v->size++;
}

/**
 * Czyści wektor @p v z pamięci przezeń posiadaną.
 * @param[in,out] v : wektor
 */
void ClearVector(Vector *v) {
    if (!VectorIsEmpty(v)) {
        for (unsigned j = 0; j < v->size; ++j) {
            MonoDestroy(&v->monos[j]);
        }

        free(v->monos);
    }
}

/**
 * Definicja wektora wektorów jednomianów.
 * De facto jest to stos stosów pomagający uniknąć rekurencji w parsowaniu
 * wielomianów. Każdy z elementów wektora wektorów będziemy nazywali dalej
 * w dokumentacji warstwą.
 */
typedef struct ParseVector {
    Vector *vectors; ///< tablica wektorów (warstw)
    unsigned size; ///< liczba wektorów w tablicy `vectors`
    unsigned maxSize; ///< faktyczny rozmiar tablicy `vectors`
} ParseVector;

/**
 * Wektor wektorów jednomianów, na którym odbywaja się wszystkie obliczenia
 * w module `vector`.
 */
ParseVector PV;

void ParseVectorInit() {
    PV = (ParseVector) {.vectors = malloc(sizeof(Vector)), .size = 1, .maxSize = 1};
    assert(PV.vectors != NULL);
    PV.vectors[0] = VectorCreate();
}

void ParseVectorNewLayer() {
    if (PV.size == PV.maxSize) {
        PV.maxSize *= 2;
        Vector *newVectors = malloc(sizeof(Vector) * PV.maxSize);
        assert(newVectors != NULL);

        for (unsigned j = 0; j < PV.size; ++j) {
            newVectors[j] = PV.vectors[j];
        }

        newVectors[PV.size] = VectorCreate();
        free(PV.vectors);
        PV.vectors = newVectors;
    } else {
        PV.vectors[PV.size] = VectorCreate();
    }

    PV.size++;
}

void ParseVectorAddMono(Mono m) {
    VectorAddMono(&PV.vectors[PV.size - 1], m);
}

void ParseVectorPolyAddMonos(poly_coeff_t coeff, poly_exp_t e) {
    Mono m;

    if (VectorIsEmpty(&PV.vectors[PV.size - 1])) {
        Poly p = PolyFromCoeff(coeff);
        m = MonoFromPoly(&p, e);
    } else {
        Poly p = PolyAddMonos(PV.vectors[PV.size - 1].size, PV.vectors[PV.size - 1].monos);
        free(PV.vectors[PV.size - 1].monos);

        m = MonoFromPoly(&p, e);
    }

    PV.size--;

    ParseVectorAddMono(m);

    if (PV.size <= PV.maxSize / 4) {
        Vector *newVectors = malloc(sizeof(Vector) * (PV.maxSize / 2));
        assert(newVectors != NULL);

        for (unsigned j = 0; j < PV.size; ++j) {
            newVectors[j] = PV.vectors[j];
        }

        free(PV.vectors);
        PV.vectors = newVectors;
        PV.maxSize /= 2;
    }
}

void ParseVectorClear() {
    if (PV.size > 0) {
        for (unsigned j = 0; j < PV.size; ++j) {
            ClearVector(&PV.vectors[j]);
        }

        free(PV.vectors);
    }
}

Poly ParseVectorResult() {
    assert(PV.size == 1);

    Poly p = PolyAddMonos(PV.vectors[0].size, PV.vectors[0].monos);

    free(PV.vectors[0].monos);
    free(PV.vectors);

    PV.size = 0;

    return p;
}