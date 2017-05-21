/** @file
   Implementacja parsera kalkulatora wielomianów

   @author Antoni Zawodny <az337756@students.mimuw.edu.pl>
   @copyright Uniwersytet Warszawski
   @date 2017-05-22
*/

#include "parser.h"
#include "polystack.h"
#include "vector.h"
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>

/**
 * Obecny wiersz wejścia.
 */
unsigned __row;

/**
 * Obecna kolumna wejścia.
 */
unsigned __col;

/**
 * Zmienna przechowująca wynik pomyślnego parsowania współczynnika/punktu,
 * w którym wyliczamy wartość wielomianu.
 */
poly_coeff_t __coeff;

/**
 * Zmienna przechowująca wynik pomyślengo parsowania wykładnika jednomianu.
 */
poly_exp_t __exp;

/**
 * Zmienna przechowująca wynik pomyślnego parsowania indeksu zmiennej wielomianu.
 */
unsigned __idx;

/**
 * Ostatnio wczytany znak.
 */
char __lastChar;

/**
 * Liczba nawiasów otwierających minus liczba nawiasów zamykających.
 */
unsigned __brackets;

/**
 * Stany parsera wielomianu.
 * Mamy następujące stany możliwe do osiągnięcia przez parser:
 * - LEFT_BRACKET - ostatnim wczytanym znakiem jest '(',
 * - COEFFICIENT - ostatnim wczytanym znakiem jest koniec współczynnika,
 * - COMMA - ostatnim wczytanym znakiem jest przecinek,
 * - EXPONENT - ostatnim wczytanym znakiem jest koniec wykładnika,
 * - RIGHT_BRACKET - ostatnim wczytanym znakiem jest ')',
 * - PLUS - ostatnim wczytanym znakiem jest '+',
 * - QUICK_COEFF - stan osiągany wtw. gdy linijka przechowuje współczynnik,
 *                 ostatnim wczytanym znakiem jest początek współczynnika.
 */
typedef enum ParsePolyState {LEFT_BRACKET, COEFFICIENT, COMMA, EXPONENT,
                             RIGHT_BRACKET, PLUS, QUICK_COEFF} ParsePolyState;

/**
 * Wczytuje pojedynczy znak ze standardowego wejścia.
 */
void GetChar() {
    __lastChar = (char)getchar();
    ++__col;
}

/**
 * Sprawdza czy obecna litera jest cyfrą.
 * @return czy obecna litera jest cyfrą?
 */
inline bool IsDigit() {
    return ('0' <= __lastChar && __lastChar <= '9');
}

/**
 * Sprawdza czy obecna litera jest minusem.
 * @return czy obecna litera jest minusem?
 */
inline bool IsMinus() {
    return (__lastChar == '-');
}

/**
 * Sprawdza czy obecna litera jest plusem.
 * @return czy obecna litera jest plusem?
 */
inline bool IsPlus() {
    return (__lastChar == '+');
}

/**
 * Sprawdza czy obecna litera jest nawiasem otwierającym.
 * @return czy obecna litera jest nawiasem otwierającym?
 */
inline bool IsLeftBracket() {
    return (__lastChar == '(');
}

/**
 * Sprawdza czy obecna litera jest nawiasem zamykającym.
 * @return czy obecna litera jest nawiasem zamykającym?
 */
inline bool IsRightBracket() {
    return (__lastChar == ')');
}

/**
 * Sprawdza czy obecna litera jest przecinkiem.
 * @return czy obecna litera jest przecinkiem?
 */
inline bool IsComma() {
    return (__lastChar == ',');
}

/**
 * Sprawdza czy obecna litera jest zerem.
 * @return czy obecna litera jest zerem?
 */
inline bool IsZero() {
    return (__lastChar == '0');
}

/**
 * Sprawdza czy doszliśmy do końca standardowego wejścia.
 * @return czy wczytaliśmy już wszystkie znaki?
 */
bool IsEOF() {
    return (__lastChar == EOF);
}

/**
 * Sprawdza, czy parser jest obecnie na końcu linii.
 * @return czy parser jest na końcu linii?
 */
bool IsLineFinished() {
    return (__lastChar == '\n' || IsEOF());
}

/**
 * Wczytuje znaki do napotkania znaku końca linii albo końca pliku.
 */
void ReadUntilEndline() {
    while (!IsLineFinished()) {
        GetChar();
    }
}

/**
 * Rodzaj błędu przy parsowaniu komendy.
 */
typedef enum CommandErrorType {STACK_UNDERFLOW, WRONG_COMMAND, WRONG_VALUE,
    WRONG_VARIABLE} CommandErrorType;

/**
 * Wypisuje odpowiednią informację na temat błędu @p e i przechodzi do końca
 * linii.
 * @param[in] e : rodzaj błędu
 */
void PrintCommandError(CommandErrorType e) {
    switch (e) {
        case STACK_UNDERFLOW:
            fprintf(stderr, "ERROR %u STACK UNDERFLOW\n", __row);
            break;
        case WRONG_COMMAND:
            fprintf(stderr, "ERROR %u WRONG COMMAND\n", __row);
            break;
        case WRONG_VALUE:
            fprintf(stderr, "ERROR %u WRONG VALUE\n", __row);
            break;
        case WRONG_VARIABLE:
            fprintf(stderr, "ERROR %u WRONG VARIABLE\n", __row);
            break;
        default:
            break;
    }

    ReadUntilEndline();
}

/**
 * Wypisuje komunikat błędu przy parsowaniu wielomianu, a następnie przechodzi
 * do końca obecnej linii.
 */
void PrintPolyError() {
    fprintf(stderr, "ERROR %u %u\n", __row, __col);

    ReadUntilEndline();
}

/**
 * Zwraca wynik próby sparsowania liczby typu `poly_coeff_t` pczynając od
 * obecnego znaku.
 * Funkcja wczytuje znaki ze standardowego wejścia do pierwszego nie pasującego
 * do typu `poly_coeff_t`.
 * @return czy udało się sparsować współczynnik/punkt, w którym wyliczamy
 * wartość wielomianu?
 */
bool IsCoeffParsed() {
    static const char *MINCOEFF = "-9223372036854775808";
    static const char *MAXCOEFF = "9223372036854775807";
    static const int MINCOEFF_SIZE = 21;

    char number[21] = "xxxxxxxxxxxxxxxxxxxx";
    int numberSize = 1;
    bool isNegative = IsMinus();
    bool isZero = IsZero();

    if (!IsDigit() && !IsMinus()) {
        return false;
    }

    number[0] = __lastChar;

    do {
        GetChar();

        if (numberSize == 1 && isZero) {
            __coeff = 0;
            return true;
        } else if (numberSize == 1 && isNegative) {
            if (!IsDigit()) {
                return false;
            } else if (IsZero()) {
                GetChar();
                __coeff = 0;
                return true;
            }
        } else if (!IsDigit()) {
            break;
        }

        number[numberSize] = __lastChar;
        ++numberSize;
    } while (numberSize < MINCOEFF_SIZE - 2 + (int)isNegative);

    if (numberSize == MINCOEFF_SIZE - 2 + (int)isNegative) {
        if ((isNegative && strncmp(number, MINCOEFF, MINCOEFF_SIZE - 1) > 0)
                   || (!isNegative && strncmp(number, MAXCOEFF, MINCOEFF_SIZE - 2) > 0)) {
            return false;
        } else {
            GetChar();
            __coeff = atol(number);
            return true;
        }
    } else {
        __coeff = atol(number);
        return true;
    }
}

/**
 * Zwraca wynik próby sparsowania nieujemnej liczby typu `int` albo `unsigned`
 * (w zależności od wartości @p isUnsigned) poczynając od obecnego znaku.
 * W przypadku powodzenia działania parsera liczba zapisywana jest do zmiennej
 * globalnej `__idx` jeśli @p isUnsigned jest równe `true`, a w przypadku
 * gdy @p isUnsigned jest równe `false`, sparsowana liczba zapisywana jest do
 * zmiennej `__exp`.
 * @param[in] isUnsigned : czy parsować do zmiennej `__idx`
 * @return czy udało się sparsować współczynnik/punkt, w którym wyliczamy
 * wartość wielomianu?
 */
bool IsIntParsed(bool isUnsigned) {
    static const char *MAXEXP = "2147483647";
    static const char *MAXIDX = "4294967295";
    static const int MAXINT_SIZE = 11;

    char number[11] = "xxxxxxxxxx";
    int numberSize = 1;
    bool isZero = IsZero();

    if (!IsDigit()) {
        return false;
    }

    number[0] = __lastChar;

    do {
        GetChar();

        if (numberSize == 1 && isZero) {
            if (isUnsigned) {
                __idx = 0;
            } else {
                __exp = 0;
            }

            return true;
        } else if (!IsDigit()) {
            break;
        }

        number[numberSize] = __lastChar;
        ++numberSize;
    } while (numberSize < MAXINT_SIZE - 1);

    if (numberSize == MAXINT_SIZE - 1) {
        if (isUnsigned && strncmp(number, MAXIDX, MAXINT_SIZE - 1) > 0) {
            return false;
        } else if (!isUnsigned && strncmp(number, MAXEXP, MAXINT_SIZE - 1) > 0) {
            return false;
        } else {
            if (isUnsigned) {
                __idx = (unsigned)strtoul(number, NULL, 10);
            } else {
                __exp = atoi(number);
            }

            GetChar();

            return true;
        }
    } else {
        if (isUnsigned) {
            __idx = (unsigned)strtoul(number, NULL, 10);
        } else {
            __exp = atoi(number);
        }

        return true;
    }
}

/**
 * Zwraca wynik próby sparsowania nieujemnej liczby typu `unsigned` poczynając
 * od obecnego znaku.
 * @return czy udało się sparsować liczbę typu `unsigned`?
 */
bool IsIdxParsed() {
    return IsIntParsed(true);
}

/**
 * Zwraca wynik próby sparsowania nieujemnej liczby typu `poly_coeff_t`
 * poczynając od obecnego znaku.
 * @return czy udało się sparsować liczbę typu `poly_coeff_t`?
 */
bool IsExpParsed() {
    return IsIntParsed(false);
}

/**
 * Komendy akceptowane przez parser.
 */
const char *COMMANDS[] = {"ZERO", "IS_COEFF", "IS_ZERO", "CLONE", "ADD", "MUL",
                          "NEG", "SUB", "IS_EQ", "DEG", "DEG_BY ", "AT ",
                          "PRINT", "POP"};

/**
 * Pozycje komend w tablicy `COMMANDS`.
 */
enum ComPos {ZERO_POS, IS_COEFF_POS, IS_ZERO_POS, CLONE_POS, ADD_POS, MUL_POS,
    NEG_POS, SUB_POS, IS_EQ_POS, DEG_POS, DEG_BY_POS, AT_POS,
    PRINT_POS, POP_POS};

/**
 * Liczba komend akceptowana przez parser.
 */
const unsigned COMMANDS_SIZE = 14;

/**
 * Sprawdza czy @p s ma szansę być komendą.
 * @param[in] s : potencjalna komenda
 * @param[in] size : długość @p s
 * @return czy @p s jest prefiksem którejś z komend?
 */
bool IsCommandSubstr(const char *s, size_t size) {
    for (unsigned j = 0; j < COMMANDS_SIZE; ++j) {
        if (!strncmp(s, COMMANDS[j], size)) {
            return true;
        }
    }

    return false;
}

/**
 * Zwraca najmniejszy indeks `j` tablicy `COMMANDS` taki, że @p size jest równe
 * długości `COMMANDS[j]` i `strncmp(COMMANDS[j], s, size) == 0`, a jeśli
 * takowy nie istnieje, to zwracany jest rozmiar tablicy `COMMANDS`.
 * @param[in] s : potencjalna komenda
 * @param[in] size : rozmiar potencjalnej komendy
 * @return najmniejszy indeks `j` t. że `COMMANDS[j]` jest równe s
 */
unsigned CommandPos(const char *s, size_t size) {
    for (unsigned j = 0; j < COMMANDS_SIZE; ++j) {
        if (size == strlen(COMMANDS[j]) && !strncmp(s, COMMANDS[j], size)) {
            return j;
        }
    }

    return COMMANDS_SIZE;
}

/**
 * Przetwarza pojedynczą linię ze standardowego wejścia jako komendę
 * kalkulatora.
 */
void ParseCommand() {
    char com[9] = "xxxxxxxx";
    com[0] = __lastChar;
    unsigned charCount = 1;

    while (IsCommandSubstr(com, charCount) && CommandPos(com, charCount) == COMMANDS_SIZE) {
        GetChar();
        com[charCount] = __lastChar;
        ++charCount;
    }

    unsigned pos;

    if (!IsCommandSubstr(com, charCount)) {
        PrintCommandError(WRONG_COMMAND);
        return;
    } else if (CommandPos(com, charCount) == DEG_POS) {
        GetChar();

        if (IsLineFinished()) {
            pos = DEG_POS;
        } else if (__lastChar != '_') {
            PrintCommandError(WRONG_COMMAND);
            return;
        } else {
            com[charCount] = __lastChar;
            ++charCount;

            while (IsCommandSubstr(com, charCount) && CommandPos(com, charCount) == COMMANDS_SIZE) {
                GetChar();
                com[charCount] = __lastChar;
                ++charCount;
            }

            if (!IsCommandSubstr(com, charCount)) {
                PrintCommandError(WRONG_COMMAND);
                return;
            } else {
                pos = DEG_BY_POS;
            }

        }
    } else {
        pos = CommandPos(com, charCount);
    }

    if (__lastChar == ' ') {
        GetChar();

        switch (pos) {
            case DEG_BY_POS:
                if (IsIdxParsed()) {
                    if (StackIsEmpty()) {
                        PrintCommandError(STACK_UNDERFLOW);
                    } else if (!IsLineFinished()) {
                        PrintCommandError(WRONG_VARIABLE);
                    } else {
                        printf("%d\n", StackDegBy(__idx));
                    }
                } else {
                    PrintCommandError(WRONG_VARIABLE);
                }
                break;
            case AT_POS:
                if (IsCoeffParsed()) {
                    if (StackIsEmpty()) {
                        PrintCommandError(STACK_UNDERFLOW);
                    } else if (!IsLineFinished()) {
                        PrintCommandError(WRONG_VALUE);
                    } else {
                        StackAt(__coeff);
                    }
                } else {
                    PrintCommandError(WRONG_VALUE);
                }
                break;
            default:
                assert(false);
                return;
        }
    } else {
        if (pos != DEG_POS) {
            GetChar();
        }

        if (!IsLineFinished()) {
            PrintCommandError(WRONG_COMMAND);
            return;
        }

        switch (pos) {
            case ZERO_POS:
                StackZero();
                break;

            case IS_COEFF_POS:
                if (!StackSize()) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    printf("%d\n", StackIsCoeff());
                }
                break;

            case IS_ZERO_POS:
                if (!StackSize()) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    printf("%d\n", StackIsZero());
                }
                break;

            case CLONE_POS:
                if (!StackSize()) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    StackClone();
                }
                break;

            case ADD_POS:
                if (StackSize() < 2) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    StackAdd();
                }
                break;

            case MUL_POS:
                if (StackSize() < 2) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    StackMul();
                }
                break;

            case NEG_POS:
                if (!StackSize()) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    StackNeg();
                }
                break;

            case SUB_POS:
                if (StackSize() < 2) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    StackSub();
                }
                break;

            case IS_EQ_POS:
                if (StackSize() < 2) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    printf("%d\n", StackIsEq());
                }
                break;

            case DEG_POS:
                if (!StackSize()) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    printf("%d\n", StackDeg());
                }
                break;

            case PRINT_POS:
                if (!StackSize()) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    StackPrint();
                }
                break;

            case POP_POS:
                if (!StackSize()) {
                    PrintCommandError(STACK_UNDERFLOW);
                } else {
                    Poly p = StackPop();
                    PolyDestroy(&p);
                }
                break;

            default:
                assert(false);
                return;
        }
    }
}

/**
 * Przetwarza pojedynczą linię ze standardowego wejścia jako wielomian.
 * Funkcja symuluje działanie automatu ze stosem, gdzie stanami są wartości
 * typu `ParsePolyState`, stanami początkowymi są stany `LEFT_BRACKET`,
 * `COEFFICIENT` i `QUICK_COEFF`, stanami końcowymi są stany `LEFT_BRACKET`,
 * `COEFFICIENT` i `QUICK_COEFF`.
 */
void ParsePolynomial() {
    __col = 1;
    ParsePolyState state;
    __brackets = 0;

    if (!IsLeftBracket() && !IsMinus() && !IsDigit()) {
        PrintPolyError();
        return;
    }

    state = (IsLeftBracket()) ? LEFT_BRACKET : QUICK_COEFF;

    if (state == LEFT_BRACKET) {
        ++__brackets;
        GetChar();
    }

    while (true) {
        switch (state) {
            case LEFT_BRACKET:
                ParseVectorNewLayer();

                if (!IsLeftBracket() && !IsMinus() && !IsDigit()) {
                    PrintPolyError();
                    return;
                } else if (IsLeftBracket()) {
                    __brackets++;
                    GetChar();
                } else if (!IsCoeffParsed()) {
                    PrintPolyError();
                    return;
                } else {
                    state = COEFFICIENT;
                }
                break;

            case COEFFICIENT:
                if (!IsComma() || !__brackets) {
                    PrintPolyError();
                    return;
                } else {
                    state = COMMA;
                    GetChar();
                }
                break;

            case COMMA:
                if (!IsDigit()) {
                    PrintPolyError();
                    return;
                } else if (!IsExpParsed()) {
                    PrintPolyError();
                    return;
                } else {
                    state = EXPONENT;
                }
                break;

            case EXPONENT:
                if (!IsRightBracket()) {
                    PrintPolyError();
                    return;
                } else {
                    ParseVectorPolyAddMonos(__coeff, __exp);
                    --__brackets;
                    state = RIGHT_BRACKET;
                    GetChar();
                }
                break;

            case RIGHT_BRACKET:
                if (!IsComma() && !IsPlus() && !IsLineFinished()) {
                    PrintPolyError();
                    return;
                } else if (IsComma()) {
                    if (__brackets == 0) {
                        PrintPolyError();
                        return;
                    } else {
                        state = COMMA;
                        GetChar();
                    }
                } else if (IsPlus()) {
                    state = PLUS;
                    GetChar();
                } else {
                    if (__brackets > 0) {
                        PrintPolyError();
                    } else {
                        StackPush(ParseVectorResult());
                    }
                    return;
                }
                break;

            case PLUS:
                if (IsLeftBracket()) {
                    __brackets++;
                    state = LEFT_BRACKET;
                    GetChar();
                } else {
                    PrintPolyError();
                    return;
                }
                break;

            case QUICK_COEFF:
                if (IsCoeffParsed()) {
                    if (IsLineFinished()) {
                        StackPush(PolyFromCoeff(__coeff));
                    } else {
                        PrintPolyError();
                    }
                } else {
                    PrintPolyError();
                }
                return;
        }
    }
}

/**
 * Przetwarza pojedynczą linię ze standardowego wejścia.
 * Funkcja najpierw próbuje przetworzyć linijkę jako polecenie kalkulatora,
 * a jeśli to nie wyjdzie, to następuje próba przetworzenia jej jako wielomianu.
 * Jeśli zaś ostatnim wczytanym znakiem był EOF, to funkcja nic nie robi.
 */
void ParseLine() {
    __col = 0;

    GetChar();

    if (IsEOF()) {
        return;
    } else if (isalpha(__lastChar)) {
        ParseCommand();
    } else {
        ParseVectorInit();

        ParsePolynomial();

        ParseVectorClear();
    }

    ++__row;
}

void Parse() {
    StackInit();

    __row = 1;

    do {
        ParseLine();
    } while (!IsEOF());

    StackClear();
}