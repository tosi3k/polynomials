/** @file
   Interfejs wektora wektorów jednomianów

   @author Antoni Zawodny <az337756@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-22
*/

#ifndef __VECTOR_H__
#define __VECTOR_H__

#include "poly.h"

/**
 * Inicjalizuje wektor wektorów jednomianów.
 */
void ParseVectorInit();

/**
 * Dodaje nową warstwę na wierzch wektora wektorów.
 */
void ParseVectorNewLayer();

/**
 * Wrzuca na koniec wierzchniej warstwy wektora wektorów jednomian @p m.
 * @param[in] m : jednomian
 */
void ParseVectorAddMono(Mono m);

/**
 * Sumuje jednomiany w wierzchniej warstwie wektora wektorów, usuwa wierzchnią
 * warstwę i wrzuca na koniec nowej wierzchniej warstwy jednomian z wielomianem
 * będącym sumą wspomnianych jednomianów i wykładnikiem równym @p e.
 * Jeśli wierzchnia warstwa wektora wektorów jest pusta, usuwa ją i wrzuca
 * wielomian (@p coeff, @p e) na koniec nowej wierzchniej warstwy.
 * @param[in] coeff : współczynnik
 * @param[in] e : wykładnik
 */
void ParseVectorPolyAddMonos(poly_coeff_t coeff, poly_exp_t e);

/**
 * Czyści wektor wektorów z całej dynamicznie zaalokowanej pamięci.
 */
void ParseVectorClear();

/**
 * Zwraca wielomian będący sumą jednomianów z pierwszej warstwy.
 * Wywłaszcza program, gdy wektor wektorów ma liczbę warstw różną od 1.
 * @return wielomian
 */
Poly ParseVectorResult();

#endif /* __VECTOR_H__ */