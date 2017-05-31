/** @file
   Interfejs klasy stosu wielomianów

   @author Antoni Zawodny <az337756@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-22
*/

#ifndef __POLYSTACK_H__
#define __POLYSTACK_H__

#include "poly.h"

/**
 * Inicjalizuje stos wielomianów.
 */
void StackInit();

/**
 * Kładzie wielomian @p p na szczyt stosu.
 * Przejmuje na własność pamięć przetrzymywaną przez wielomian @p p.
 * @param[in] p : wielomian
 */
void StackPush(const Poly p);

/**
 * Zwraca wielomian ze szczytu stosu.
 * @return wielomian ze szczytu stosu
 */
Poly StackPop();

/**
 * Zwraca rozmiar stosu.
 * @return rozmiar stosu
 */
unsigned StackSize();

/**
 * Sprawdza czy stos jest pusty.
 * @return czy stos jest pusty?
 */
bool StackIsEmpty();

/**
 * Usuwa wszystkie elementy ze stosu.
 */
void StackClear();

/**
 * Wrzuca wielomian zerowy na szczyt stosu.
 */
void StackZero();

/**
 * Sprawdza czy wielomian na szczycie stosu jest współczynnikiem.
 * @return czy wielomian ze szczytu stosu jest współczynnikiem?
 */
bool StackIsCoeff();

/**
 * Sprawdza czy wielomian na szczycie stosu jest zerowy.
 * @return czy wielomian ze szczytu stosu jest zerowy?
 */
bool StackIsZero();

/**
 * Wstawia na stos kopię wielomianu ze szczytu.
 */
void StackClone();

/**
 * Dodaje dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek
 * stosu ich sumę.
 */
void StackAdd();

/**
 * Dodaje dwa wielomiany z wierzchu stosu, usuwa je i wstawia na wierzchołek
 * stosu ich iloczyn.
 */
void StackMul();

/**
 * Neguje wielomian na wierzchołku stosu.
 */
void StackNeg();

/**
 * Odejmuje od wielomianu z wierzchołka wielomian pod wierzchołkiem, usuwa je
 * i wstawia na wierzchołek stosu różnicę.
 */
void StackSub();

/**
 * Sprawdza, czy dwa wielomiany na wierzchu stosu są równe.
 * @return czy dwa wielomiany na wierzchu stosu są równe?
 */
bool StackIsEq();

/**
 * Zwraca stopień wielomianu ze szczytu stosu (−1 dla wielomianu tożsamościowo
 * równego zeru).
 * @return stopień wielomianu ze szczytu stosu
 */
poly_exp_t StackDeg();

/**
 * Zwraca stopień wielomianu ze szczytu stosu ze względu na zmienną o numerze
 * @p var_idx (−1 dla wielomianu tożsamościowo równego zeru).
 * @param[in] var_idx : indeks zmiennej
 * @return stopień wielomianu ze szczytu stosu ze względu na zmienną o numerze
 * @p var_idx
 */
poly_exp_t StackDegBy(unsigned var_idx);

/**
 * Wylicza wartość wielomianu w punkcie `x`, usuwa wielomian z wierzchołka
 * i wstawia na stos wynik operacji.
 * @param[in] x : punkt
 */
void StackAt(poly_coeff_t x);

/**
 * Wypisuje na standardowe wyjście wielomian z wierzchołka stosu w formacie
 * akceptowanym przez parser.
 */
void StackPrint();

/**
 * TODO
 * @param[in] count :
 */
void StackCompose(unsigned count);

#endif /* __POLYSTACK_H__ */