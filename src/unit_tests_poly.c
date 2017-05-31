/*
 * Copyright 2008 Google Inc.
 * Copyright 2015 Tomasz Kociumaka
 * Copyright 2016, 2017 IPP team
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <setjmp.h>

#include "cmocka.h"
#include "poly.h"

int mock_fprintf(FILE* const file, const char *format, ...) CMOCKA_PRINTF_ATTRIBUTE(2, 3);
int mock_printf(const char *format, ...) CMOCKA_PRINTF_ATTRIBUTE(1, 2);

/**
 * Pomocnicze bufory, do których piszą atrapy funkcji printf i fprintf oraz
 * pozycje zapisu w tych buforach. Pozycja zapisu wskazuje bajt o wartości 0.
 */
static char fprintf_buffer[256];
static char printf_buffer[256];
static int fprintf_position = 0;
static int printf_position = 0;

extern int mock_main();

/**
 * Atrapa funkcji fprintf sprawdzająca poprawność wypisywania na stderr.
 */
int mock_fprintf(FILE* const file, const char *format, ...) {
    int return_value;
    va_list args;

    assert_true(file == stderr);
    /* Poniższa asercja sprawdza też, czy fprintf_position jest nieujemne.
    W buforze musi zmieścić się kończący bajt o wartości 0. */
    assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));

    va_start(args, format);
    return_value = vsnprintf(fprintf_buffer + fprintf_position,
                             sizeof(fprintf_buffer) - fprintf_position,
                             format,
                             args);
    va_end(args);

    fprintf_position += return_value;
    assert_true((size_t)fprintf_position < sizeof(fprintf_buffer));
    return return_value;
}

/**
 * Atrapa funkcji printf sprawdzająca poprawność wypisywania na stderr.
 */
int mock_printf(const char *format, ...) {
    int return_value;
    va_list args;

    /* Poniższa asercja sprawdza też, czy printf_position jest nieujemne.
    W buforze musi zmieścić się kończący bajt o wartości 0. */
    assert_true((size_t)printf_position < sizeof(printf_buffer));

    va_start(args, format);
    return_value = vsnprintf(printf_buffer + printf_position,
                             sizeof(printf_buffer) - printf_position,
                             format,
                             args);
    va_end(args);

    printf_position += return_value;
    assert_true((size_t)printf_position < sizeof(printf_buffer));
    return return_value;
}

/**
 *  Pomocniczy bufor, z którego korzystają atrapy funkcji operujących na stdin.
 */
static char input_stream_buffer[256];
static int input_stream_position = 0;
static int input_stream_end = 0;
int read_char_count;

/**
 * Atrapa funkcji scanf używana do przechwycenia czytania z stdin.
 */
int mock_scanf(const char *format, ...) {
    va_list fmt_args;
    int ret;

    va_start(fmt_args, format);
    ret = vsscanf(input_stream_buffer + input_stream_position, format, fmt_args);
    va_end(fmt_args);

    if (ret < 0) { /* ret == EOF */
        input_stream_position = input_stream_end;
    }
    else {
        assert_true(read_char_count >= 0);
        input_stream_position += read_char_count;
        if (input_stream_position > input_stream_end) {
            input_stream_position = input_stream_end;
        }
    }
    return ret;
}

/**
 * Atrapa funkcji getchar używana do przechwycenia czytania z stdin.
 */
int mock_getchar() {
    if (input_stream_position < input_stream_end)
        return input_stream_buffer[input_stream_position++];
    else
        return EOF;
}

/**
 * Funkcja wołana przed każdym testem korzystającym z stdout lub stderr.
 */
static int test_setup(void **state) {
    (void)state;

    memset(fprintf_buffer, 0, sizeof(fprintf_buffer));
    memset(printf_buffer, 0, sizeof(printf_buffer));
    printf_position = 0;
    fprintf_position = 0;

    /* Zwrócenie zera oznacza sukces. */
    return 0;
}

/**
 * Funkcja inicjująca dane wejściowe dla programu korzystającego ze stdin.
 */
static void init_input_stream(const char *str) {
    memset(input_stream_buffer, 0, sizeof(input_stream_buffer));
    input_stream_position = 0;
    input_stream_end = strlen(str);
    assert_true((size_t)input_stream_end < sizeof(input_stream_buffer));
    strcpy(input_stream_buffer, str);
}

/**
 * Test polecenia PolyCompose dla `p = 0` i `count` równym `0`.
 * @param state : stan
 */
static void TestComposeZeroZero(void **state) {
    (void)state;

    Poly p = PolyZero();
    unsigned count = 0;

    Poly res = PolyCompose(&p, count, NULL);

    assert_true(PolyIsEq(&p, &res));
}

/**
 * Test polecenia PolyCompose dla `p = 0`, `count` równym `1` i `x[0] = const`.
 * @param state : stan
 */
static void TestComposeZeroConst(void **state) {
    (void)state;

    Poly p = PolyZero();
    unsigned count = 1;
    Poly x[1] = {PolyFromCoeff(777)};

    Poly res = PolyCompose(&p, count, x);

    assert_true(PolyIsEq(&p, &res));
}

/**
 * Test polecenia PolyCompose dla `p = const` i `count` równym `0`.
 * @param state : stan
 */
static void TestComposeConstZero(void **state) {
    (void)state;

    Poly p = PolyFromCoeff(5);
    unsigned count = 0;

    Poly res = PolyCompose(&p, count, NULL);

    assert_true(PolyIsEq(&p, &res));
}

/**
 * Test polecenia PolyCompose dla `p = const`, `count` równym `1` oraz
 * `x[0] = const` (`x[0]` różne od `p`).
 * @param state : stan
 */
static void TestComposeConstConst(void **state) {
    (void)state;

    Poly p = PolyFromCoeff(5);
    unsigned count = 1;
    Poly x[1] = {PolyFromCoeff(24)};

    Poly res = PolyCompose(&p, count, x);

    assert_true(PolyIsEq(&p, &res));
}

/**
 * Test polecenia PolyCompose dla `p = x_0` i `count` równym `0`.
 * @param state : stan
 */
static void TestComposeIdentityZero(void **state) {
    (void)state;

    Poly tmp = PolyFromCoeff(1);
    Mono m[1] = {MonoFromPoly(&tmp, 1)};
    Poly p = PolyAddMonos(1, m);

    unsigned count = 0;

    Poly res = PolyCompose(&p, count, NULL);
    Poly expectedRes = PolyZero();

    assert_true(PolyIsEq(&expectedRes, &res));

    PolyDestroy(&p);
}

/**
 * Test polecenia PolyCompose dla `p = x_0`, `count` równym `1` i `x[0] = const`.
 * @param state : stan
 */
static void TestComposeIdentityConst(void **state) {
    (void)state;

    Poly tmp = PolyFromCoeff(1);
    Mono m[1] = {MonoFromPoly(&tmp, 1)};
    Poly p = PolyAddMonos(1, m);
    Poly x[1] = {PolyFromCoeff(99)};

    unsigned count = 1;

    Poly res = PolyCompose(&p, count, x);

    assert_true(PolyIsEq(&x[0], &res));

    PolyDestroy(&p);
}

/**
 * Test polecenia PolyCompose dla `p = x_0`, `count` równym `1` i `x[0] = x_0`.
 * @param state : stan
 */
static void TestComposeIdentityIdentity(void **state) {
    (void)state;

    Poly tmp = PolyFromCoeff(1);
    Mono m[1] = {MonoFromPoly(&tmp, 1)};
    Poly p = PolyAddMonos(1, m);
    Poly x[1] = {PolyClone(&p)};

    unsigned count = 1;

    Poly res = PolyCompose(&p, count, x);

    assert_true(PolyIsEq(&p, &res));

    PolyDestroy(&p);
    PolyDestroy(&res);
    PolyDestroy(&x[0]);
}

/**
 * Test parsowania polecenia COMPOSE bez argumentu.
 * @param state : stan
 */
static void TestComposeParameterNone(void **state) {
    (void)state;

    init_input_stream("2\nCOMPOSE ");
    assert_int_equal(mock_main(), 0);
    assert_string_equal(fprintf_buffer, "ERROR 2 WRONG COUNT\n");
}

/**
 * Test parsowania polecenia COMPOSE z argumentem równym `0`.
 * @param state : stan
 */
static void TestComposeParameterZero(void **state) {
    (void)state;

    init_input_stream("(((1,6),5),2)+((1,0)+(1,2),3)+(5,7)\nCOMPOSE 0\nPRINT\nPOP\nCOMPOSE 0");
    assert_int_equal(mock_main(), 0);
    assert_string_equal(printf_buffer, "0\n");
    assert_string_equal(fprintf_buffer, "ERROR 5 STACK UNDERFLOW\n");
}

/**
 * Test parsowania polecenia COMPOSE z argumentem równym `UINT_MAX`.
 * @param state : stan
 */
static void TestComposeParameterMax(void **state) {
    (void)state;

    init_input_stream("(((1,6),5),2)+((1,0)+(1,2),3)+(5,7)\nCOMPOSE 4294967295");
    assert_int_equal(mock_main(), 0);
    assert_string_equal(fprintf_buffer, "ERROR 2 STACK UNDERFLOW\n");
}

/**
 * Test parsowania polecenia COMPOSE z argumentem równym `-1`.
 * @param state : stan
 */
static void TestComposeParameterTooSmall(void **state) {
    (void)state;

    init_input_stream("(((1,6),5),2)+((1,0)+(1,2),3)+(5,7)\nCOMPOSE -1");
    assert_int_equal(mock_main(), 0);
    assert_string_equal(fprintf_buffer, "ERROR 2 WRONG COUNT\n");
}

/**
 * Test parsowania polecenia COMPOSE z argumentem przekraczającym typ `unsigned`
 * o jeden.
 * @param state : stan
 */
static void TestComposeParameterTooBig(void **state) {
    (void)state;

    init_input_stream("(((1,6),5),2)+((1,0)+(1,2),3)+(5,7)\nCOMPOSE 4294967296");
    assert_int_equal(mock_main(), 0);
    assert_string_equal(fprintf_buffer, "ERROR 2 WRONG COUNT\n");
}

/**
 * Test parsowania polecenia COMPOSE z argumentem znacznie przekraczającym typ
 * `unsigned`.
 * @param state : stan
 */
static void TestComposeParameterLarge(void **state) {
    (void)state;

    init_input_stream("(((1,6),5),2)+((1,0)+(1,2),3)+(5,7)\nCOMPOSE 42949672954294967295");
    assert_int_equal(mock_main(), 0);
    assert_string_equal(fprintf_buffer, "ERROR 2 WRONG COUNT\n");
}

/**
 * Test parsowania polecenia COMPOSE z argumentem złożonym z samych liter.
 * @param state : stan
 */
static void TestComposeParameterLetters(void **state) {
    (void)state;

    init_input_stream("(((1,6),5),2)+((1,0)+(1,2),3)+(5,7)\nCOMPOSE aSfghEbas");
    assert_int_equal(mock_main(), 0);
    assert_string_equal(fprintf_buffer, "ERROR 2 WRONG COUNT\n");
}

/**
 * Test parsowania polecenia COMPOSE z argumentem alfanumerycznym.
 * @param state : stan
 */
static void TestComposeParameterAlphanumeric(void **state) {
    (void)state;

    init_input_stream("(((1,6),5),2)+((1,0)+(1,2),3)+(5,7)\nCOMPOSE 452asgew");
    assert_int_equal(mock_main(), 0);
    assert_string_equal(fprintf_buffer, "ERROR 2 WRONG COUNT\n");
}

int main(void) {
    const struct CMUnitTest PolyComposeFunctionTests[] = {
            cmocka_unit_test(TestComposeZeroZero),
            cmocka_unit_test(TestComposeZeroConst),
            cmocka_unit_test(TestComposeConstZero),
            cmocka_unit_test(TestComposeConstConst),
            cmocka_unit_test(TestComposeIdentityZero),
            cmocka_unit_test(TestComposeIdentityConst),
            cmocka_unit_test(TestComposeIdentityIdentity)
    };

    const struct CMUnitTest PolyComposeParseTests[] = {
            cmocka_unit_test_setup(TestComposeParameterNone, test_setup),
            cmocka_unit_test_setup(TestComposeParameterZero, test_setup),
            cmocka_unit_test_setup(TestComposeParameterMax, test_setup),
            cmocka_unit_test_setup(TestComposeParameterTooSmall, test_setup),
            cmocka_unit_test_setup(TestComposeParameterTooBig, test_setup),
            cmocka_unit_test_setup(TestComposeParameterLarge, test_setup),
            cmocka_unit_test_setup(TestComposeParameterLetters, test_setup),
            cmocka_unit_test_setup(TestComposeParameterAlphanumeric, test_setup)
    };

    return cmocka_run_group_tests(PolyComposeFunctionTests, NULL, NULL) || cmocka_run_group_tests(PolyComposeParseTests, NULL, NULL);
}