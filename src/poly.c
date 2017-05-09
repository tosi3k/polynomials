/** @file
   Implementacja klasy wielomianów

   @author Antoni Zawodny <az337756@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-09
*/

#include <assert.h>
#include "poly.h"

/**
 * Wywłaszcza program w przypadku niepowodzenia alokacji pamięci przydzielonej wskaźnikowi @p ptr.
 * @param[in] ptr : wskaźnik, który powinien być różny od NULL;
 */
static inline void checkAllocation(const void *ptr) {
    assert(ptr != NULL);
}

void PolyDestroy(Poly *p) {
    if (!PolyIsCoeff(p)) {
        for (unsigned i = 0; i < p->size; ++i) {
            MonoDestroy(&p->monos[i]);
        }
        free(p->monos);
    }
}

Poly PolyClone(const Poly *p) {
    Poly res = (Poly) {.monos = NULL, .size = p->size, .coeff = p->coeff};
    if (!PolyIsCoeff(p)) {
        res.monos = malloc(sizeof(Mono) * p->size);
        checkAllocation(res.monos);
        
        for (unsigned j = 0; j < p->size; ++j) {
            Mono m = MonoClone(&p->monos[j]);
            res.monos[j] = m;
        }
    }
    return res;
}

bool PolyIsEq(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) != PolyIsCoeff(q)) {
        return false;
    } else if (PolyIsCoeff(p)) {
        return p->coeff == q->coeff;
    } else if (p->size != q->size) {
        return false;
    } else {
        for (unsigned j = 0; j < p->size; ++j) {
            if (p->monos[j].exp != q->monos[j].exp) {
                return false;
            } else if (!PolyIsEq(&p->monos[j].p, &q->monos[j].p)) {
                return false;
            }
        }
        return true;
    }
}

/**
 * Neguje rekurencyjnie współczynniki wielomianu @p p.
 * @param[in,out] p : wielomian do zanegowania
 */
static void PolyNegRec(Poly *p) {
    if (PolyIsCoeff(p)) {
        p->coeff = -p->coeff;
    } else {
        for (unsigned j = 0; j < p->size; ++j) {
            PolyNegRec(&p->monos[j].p);
        }
    }
}

Poly PolyNeg(const Poly *p) {
    Poly result = PolyClone(p);
    PolyNegRec(&result);
    return result;
}

Poly PolySub(const Poly *p, const Poly *q) {
    Poly minusQ = PolyNeg(q);
    Poly result = PolyAdd(p, &minusQ);
    PolyDestroy(&minusQ);
    return result;
}

poly_exp_t PolyDegBy(const Poly *p, unsigned var_idx) {
    if (PolyIsZero(p)) {
        return -1;
    } else if (PolyIsCoeff(p)) {
        return 0;
    } else {
        poly_exp_t result = 0;
        if (var_idx) {
            for (unsigned j = 0; j < p->size; ++j) {
                poly_exp_t tmp = PolyDegBy(&p->monos[j].p, var_idx - 1);
                result = (result < tmp) ? tmp : result;
            }
        } else {
            result = (result < p->monos[p->size - 1].exp) ? p->monos[p->size - 1].exp : result;
        }
        return result;
    }
}

poly_exp_t PolyDeg(const Poly *p) {
    if (PolyIsZero(p)) {
        return -1;
    } else if (PolyIsCoeff(p)) {
        return 0;
    } else {
        poly_exp_t result = 0;
        for (unsigned j = 0; j < p->size; ++j) {
            poly_exp_t tmp = p->monos[j].exp + PolyDeg(&p->monos[j].p);
            if (p->monos[j].exp == -1) {
                ++tmp;
            }
            result = (tmp > result) ? tmp : result;
        }
        return result;
    }
}

Poly PolyAdd(const Poly *p, const Poly *q) {
    if (PolyIsZero(p)) {
        return PolyClone(q);
    } else if (PolyIsZero(q)) {
        return PolyClone(p);
    } else if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff + q->coeff);
    } else if (PolyIsCoeff(p)) {
        if (q->monos[0].exp >= 0) {
            Poly result;
            result.size = q->size + 1;
            result.coeff = 0;
            result.monos = malloc(sizeof(Mono) * (q->size + 1));
            checkAllocation(result.monos);
            result.monos[0] = (Mono) {.p = PolyClone(p), .exp = -1};
            for (unsigned j = 1; j <= q->size; ++j) {
                result.monos[j] = MonoClone(&q->monos[j - 1]);
            }
            return result;
        } else {
            Poly result;
            if (q->monos[0].p.coeff == -p->coeff) {
                Mono *newMonos = malloc(sizeof(Mono) * (q->size - 1));
                checkAllocation(newMonos);
                for (unsigned j = 0; j < q->size - 1; ++j) {
                    newMonos[j] = MonoClone(&q->monos[j + 1]);
                }
                result = (Poly) {.monos = newMonos, .size = q->size - 1, .coeff = 0};
            } else {
                result = PolyClone(q);
                result.monos[0].p.coeff += p->coeff;
            }
            return result;
        }
    } else if (PolyIsCoeff(q)) {
        return PolyAdd(q, p);
    }
    else {
        unsigned i = 0, j = 0, newSize = 0;
        Mono *monosPlaceholder = malloc(sizeof(Mono) * (p->size + q->size));
        checkAllocation(monosPlaceholder);
        while (i < p->size || j < q->size) {
            if (i < p->size && j < q->size) {
                if (p->monos[i].exp < q->monos[j].exp) {
                    monosPlaceholder[newSize] = MonoClone(&p->monos[i]);
                    newSize++;
                    i++;
                }
                else if (p->monos[i].exp > q->monos[j].exp) {
                    monosPlaceholder[newSize] = MonoClone(&q->monos[j]);
                    newSize++;
                    j++;
                }
                else {
                    Poly tmp1 = PolyClone(&p->monos[i].p);
                    Poly tmp2 = PolyClone(&q->monos[j].p);
                    Poly tmp3 = PolyNeg(&tmp2);
                    bool newComponent = !PolyIsEq(&tmp1, &tmp3);
                    PolyDestroy(&tmp1), PolyDestroy(&tmp2), PolyDestroy(&tmp3);
                    if (newComponent) {
                        Poly toAdd = PolyAdd(&p->monos[i].p, &q->monos[j].p);
                        monosPlaceholder[newSize] = MonoFromPoly(&toAdd, p->monos[i].exp);
                        newSize++;
                    }
                    i++;
                    j++;
                }
            } else if (i < p->size) {
                monosPlaceholder[newSize] = MonoClone(&p->monos[i]);
                newSize++;
                i++;
            } else {
                monosPlaceholder[newSize] = MonoClone(&q->monos[j]);
                newSize++;
                j++;
            }
        }
        Mono *properMonos;
        if (newSize) {
            properMonos = malloc(sizeof(Mono) * newSize);
            checkAllocation(properMonos);
            for (unsigned k = 0; k < newSize; ++k) {
                properMonos[k] = monosPlaceholder[k];
            }
            if (newSize == 1 && properMonos[0].exp == -1) {
                Poly result = properMonos[0].p;
                free(properMonos);
                free(monosPlaceholder);
                return result;
            }
        } else {
            properMonos = NULL;
        }
        free(monosPlaceholder);
        Poly result = (Poly) {.monos = properMonos, .size = newSize, .coeff = 0};
        return result;
    }
}

/**
 * Zwraca współczynnik @p coeff podniesiony do potęgi @p exp.
 * @param[in] coeff : współczynnik
 * @param[in] exp :
 * @return `coeff^exp`
 */
static poly_coeff_t CoeffPower(poly_coeff_t coeff, poly_exp_t exp) {
    if (exp <= 0) {
        return 1;
    }
    poly_coeff_t result = 1;
    while (exp) {
        if (exp & 1) {
            result *= coeff;
        }
        exp >>= 1;
        coeff *= coeff;
    }
    return result;
}

/**
 * Mnoży skalarnie wielomian @p p przez stałą @p mult.
 * @param[in,out] p : wielomian
 * @param[in] mult : skalar
 */
static void PolyScalarMul(Poly *p, poly_coeff_t mult) {
    if (PolyIsCoeff(p)) {
        p->coeff *= mult;
        return;
    } else {
        unsigned newSize = 0;
        for (unsigned j = 0; j < p->size; ++j) {
            PolyScalarMul(&p->monos[j].p, mult);
            newSize += !PolyIsZero(&p->monos[j].p);
        }
        
        if (!newSize) {
            free(p->monos);
            p->monos = NULL;
            p->coeff = 0;
        } else {
            Mono *newMonos = malloc(sizeof(Mono) * newSize);
            checkAllocation(newMonos);
            unsigned counter = 0;
            for (unsigned j = 0; j < p->size; ++j) {
                if (!PolyIsZero(&p->monos[j].p)) {
                    newMonos[counter] = p->monos[j];
                    ++counter;
                }
            }
            free(p->monos);
            p->monos = newMonos;
        }
    }
}

Poly PolyAt(const Poly *p, poly_coeff_t x) {
    if (PolyIsCoeff(p)) {
        return PolyClone(p);
    } else {
        Poly result = PolyFromCoeff(0);
        for (unsigned j = 0; j < p->size; ++j) {
            poly_coeff_t mult = CoeffPower(x, p->monos[j].exp);

            Poly a = PolyClone(&p->monos[j].p);
            PolyScalarMul(&a, mult);

            Poly tmp = PolyAdd(&result, &a);

            PolyDestroy(&result), PolyDestroy(&a);
            result = tmp;
        }
        return result;
    }
}

/**
 * Sprawdza, czy jednomian @p a posiada większy wykładnik od wykładnika jednomianu @p b.
 * @param[in] a : jednomian
 * @param[in] b : jednomian
 * @return czy wykładnik jednomianu @p a jest większy od wykładnika jednomianu @p b?
 */
static inline int MonoComparator(const void *a, const void *b) {
    return (((Mono*)a)->exp > ((Mono*)b)->exp) ? 1 : 0;
}

/**
 * Sprowadza wielomian @p do postaci pożądanej przez implementację.
 * @param[in,out] p : wielomian
 */
static void NormalizePoly(Poly *p) {
    if (!PolyIsCoeff(p)) {
        poly_coeff_t val;
        bool polyIsBad = false;
        if (p->size == 1 && p->monos[0].exp == -1) {
            Poly q = p->monos[0].p;
            free(p->monos);
            *p = q;
        } else if (p->monos[0].exp == 0 && p->monos[0].p.monos[0].exp == -1) {
            val = p->monos[0].p.monos[0].p.coeff;
            polyIsBad = true;
        } else if (p->size > 1 && p->monos[0].exp == -1 && p->monos[1].exp == 0
                   && p->monos[1].p.monos[0].exp == -1) {
            val = p->monos[1].p.monos[0].p.coeff;
            polyIsBad = true;
        }
        if (polyIsBad) {
            Poly toAdd = PolyFromCoeff(val);
            Poly toSub = (Poly) {.monos = malloc(sizeof(Mono)), .size = 1, .coeff = 0};
            checkAllocation(toSub.monos);
            Poly coeffPlaceholder = (Poly) {.monos = malloc(sizeof(Mono)), .size = 1, .coeff = 0};
            checkAllocation(coeffPlaceholder.monos);

            Mono m2 = MonoFromPoly(&toAdd, 0);
            Mono m1 = MonoFromPoly(&coeffPlaceholder, 0);
            toSub.monos[0] = m1;
            coeffPlaceholder.monos[0] = m2;
            
            Poly tmp = PolyAdd(p, &toAdd);
            PolyDestroy(p);
            *p = tmp;
            
            tmp = PolySub(p, &toSub);
            PolyDestroy(p);
            PolyDestroy(&toSub);
            *p = tmp;
        }
    }
}

Poly PolyAddMonos(unsigned count, const Mono monos[]) {
    if (!count) {
        return PolyFromCoeff(0);
    } else {
        Mono *monosCopy = malloc(sizeof(Mono) * count);
        checkAllocation(monosCopy);
        for (unsigned j = 0; j < count; ++j) {
            monosCopy[j] = monos[j];
            if (monosCopy[j].exp == 0 && PolyIsCoeff(&monosCopy[j].p)) {
                monosCopy[j].exp = -1;
            }
        }
        
        qsort(monosCopy, count, sizeof(Mono), MonoComparator);

        poly_exp_t lastExp = -2;
        unsigned newSize = 0;
        Poly summand = PolyFromCoeff(0);
        
        Mono *monosPlaceholder = malloc(sizeof(Mono) * count);
        checkAllocation(monosPlaceholder);

        for (unsigned j = 0; j < count; ++j) {
            if (lastExp != monosCopy[j].exp) {
                if (!PolyIsZero(&summand)) {
                    monosPlaceholder[newSize] = (Mono) {.p = summand, .exp = lastExp};
                    newSize++;
                }
                summand = PolyClone(&monosCopy[j].p);
                lastExp = monosCopy[j].exp;
            } else {
                Poly tmp = PolyAdd(&summand, &monosCopy[j].p);
                PolyDestroy(&summand);
                summand = tmp;
            }
        }

        if (!PolyIsZero(&summand)) {
            monosPlaceholder[newSize] = (Mono) {.p = summand, .exp = lastExp};
            newSize++;
        }

        Poly result;
        if (!newSize) {
            result = PolyFromCoeff(0);
        } else {
            Mono *properMonos = malloc(sizeof(Mono) * newSize);
            checkAllocation(properMonos);
            for (unsigned j = 0; j < newSize; ++j) {
                properMonos[j] = monosPlaceholder[j];
            }
            result = (Poly) {.monos = properMonos, .size = newSize, .coeff = 0};
        }

        for (unsigned j = 0; j < count; ++j) {
            MonoDestroy(&monosCopy[j]);
        }
        
        free(monosCopy);
        free(monosPlaceholder);
        
        NormalizePoly(&result);
        
        return result;
    }
}

Poly PolyMul(const Poly *p, const Poly *q) {
    if (PolyIsCoeff(p) && PolyIsCoeff(q)) {
        return PolyFromCoeff(p->coeff * q->coeff);
    } else if (PolyIsCoeff(p)) {
        Poly result = PolyClone(q);
        PolyScalarMul(&result, p->coeff);
        return result;
    } else if (PolyIsCoeff(q)) {
        return PolyMul(q, p);
    } else {
        Mono *monos = malloc(sizeof(Mono) * p->size * q->size);
        checkAllocation(monos);
        for (unsigned i = 0; i < p->size; ++i) {
            for (unsigned j = 0; j < q->size; ++j) {
                Poly tmp = PolyMul(&p->monos[i].p, &q->monos[j].p);
                monos[i * q->size + j] = MonoFromPoly(&tmp, p->monos[i].exp + q->monos[j].exp);
                if (p->monos[i].exp == -1 || q->monos[j].exp == -1) {
                    monos[i * q->size + j].exp++;
                    if (p->monos[i].exp == 1 || q->monos[j].exp == 1) {
                        monos[i * q->size + j].exp = 1;
                    }
                }
            }
        }
        Poly result = PolyAddMonos(p->size * q->size, monos);
        free(monos);
        return result;
    }
}