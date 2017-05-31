/** @file
   Implementacja klasy stosu wielomianów

   @author Antoni Zawodny <az337756@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-22
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "polystack.h"
#include "utils.h"

typedef struct Node Node;

/**
 * Definicja typu reprezentującego element stosu.
 * Element stosu opakowuje wielomian i trzyma wskaźnik na następny element.
 */
typedef struct Node {
    Poly p; ///< wielomian
    Node *next; ///< wskaźnik na następny element stosu
} Node;

/**
 * Definicja typu reprezentującego stos wielomianów.
 * Stos przechowuje wskaźnik na szczyt i swój rozmiar.
 */
typedef struct Stack {
    Node *head; ///< wierzchołek stosu
    unsigned size; ///< rozmiar stosu
} Stack;

/**
 * Stos na którym odbywają się wszystkie operacje w module `polystack`.
 */
Stack s;

void StackInit() {
    s.head = NULL;
    s.size = 0;
}

void StackPush(Poly p) {
    Node *n = malloc(sizeof(Node));
    assert(n != NULL);
    n->p = p;
    n->next = s.head;

    s.head = n;
    s.size++;
}

Poly StackPop() {
    if (s.size) {
        Node *n = s.head;
        s.head = n->next;
        s.size--;

        Poly result = n->p;
        free(n);

        return result;
    } else {
        return PolyZero();
    }
}

unsigned StackSize() {
    return s.size;
}

bool StackIsEmpty() {
    return !StackSize();
}

void StackClear() {
    Poly p;

    while (s.size) {
        p = StackPop();
        PolyDestroy(&p);
    }
}

void StackZero() {
    StackPush(PolyZero());
}

bool StackIsCoeff() {
    return PolyIsCoeff(&s.head->p);
}

bool StackIsZero() {
    return PolyIsZero(&s.head->p);
}

void StackClone() {
    StackPush(PolyClone(&s.head->p));
}

void StackAdd() {
    Poly p1 = StackPop();
    Poly p2 = StackPop();

    Poly toPush = PolyAdd(&p1, &p2);
    StackPush(toPush);

    PolyDestroy(&p1);
    PolyDestroy(&p2);
}

void StackMul() {
    Poly p1 = StackPop();
    Poly p2 = StackPop();

    Poly toPush = PolyMul(&p1, &p2);
    StackPush(toPush);

    PolyDestroy(&p1);
    PolyDestroy(&p2);
}

void StackNeg() {
    Node *n = s.head;
    Poly p = PolyNeg(&n->p);

    PolyDestroy(&n->p);
    n->p = p;
}

void StackSub() {
    Poly p1 = StackPop();
    Poly p2 = StackPop();

    Poly toPush = PolySub(&p1, &p2);
    StackPush(toPush);

    PolyDestroy(&p1);
    PolyDestroy(&p2);
}

bool StackIsEq() {
    return PolyIsEq(&s.head->p, &s.head->next->p);
}

poly_exp_t StackDeg() {
    return PolyDeg(&s.head->p);
}

poly_exp_t StackDegBy(unsigned var_idx) {
    return PolyDegBy(&s.head->p, var_idx);
}

void StackAt(poly_coeff_t x) {
    Poly p = StackPop();

    Poly toPush = PolyAt(&p, x);
    StackPush(toPush);

    PolyDestroy(&p);
}

/**
 * Wypisuje rekurencyjnie wielomian @p p.
 * @param[in] p : wielomian
 */
void PolyPrint(const Poly *p);

/**
 * Wypisuje rekurencyjnie jednomian @p m.
 * @param[in] m : jednomian
 */
void MonoPrint(const Mono *m) {
    PolyPrint(&m->p);
    printf(",%d", m->exp);
}

void PolyPrint(const Poly *p) {
    if (PolyIsCoeff(p)) {
        printf("%ld", p->coeff);
    } else {
        Poly toPrint = PolyClone(p);

        if (toPrint.size > 1) {
            if (toPrint.monos[0].exp == -1 && toPrint.monos[1].exp > 0) {
                toPrint.monos[0].exp = 0;
            } else if (toPrint.monos[0].exp == -1) {
                Poly subtrahend = toPrint.monos[0].p;

                Poly tmp = PolySub(&toPrint, &subtrahend);
                Poly coeffWrapper = (Poly) {.monos = malloc(sizeof(Mono)), .size = 1,
                        .coeff = 0};
                assert(coeffWrapper.monos != NULL);
                Mono m = MonoFromPoly(&subtrahend, 0);
                m.exp = 0;
                coeffWrapper.monos[0] = m;

                Poly res = PolyAdd(&tmp, &coeffWrapper);
                PolyDestroy(&tmp);
                PolyDestroy(&toPrint);
                PolyDestroy(&coeffWrapper);
                toPrint = res;
            }
        }

        for (unsigned j = 0; j < toPrint.size; ++j) {
            printf("(");
            MonoPrint(&toPrint.monos[j]);
            printf(")");
            if (j < toPrint.size - 1) {
                printf("+");
            }
        }

        PolyDestroy(&toPrint);
    }
}

void StackPrint() {
    if (StackSize()) {
        Poly p = s.head->p;
        PolyPrint(&p);
        printf("\n");
    }
}

void StackCompose(unsigned count) {
    Poly p = StackPop();

    if (!count) {
        Poly tmp = PolyCompose(&p, 0, NULL);
        StackPush(tmp);
    } else {
        Poly *x = malloc(count * sizeof(Poly));

        for (unsigned j = 0; j < count; ++j) {
            x[j] = StackPop();
        }

        Poly tmp = PolyCompose(&p, count, x);
        StackPush(tmp);

        for (unsigned j = 0; j < count; ++j) {
            PolyDestroy(&x[j]);
        }
        free(x);
    }

    PolyDestroy(&p);
}