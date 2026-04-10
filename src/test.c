#include <string.h>
#define SIMPLE_CALC_IMPLEMENTATION
#include "simple_calc.h"
#include "stdio.h"
#include "test_cases.h"
#include <stdlib.h>

#define EXPECTED(expr, expected)\
    do {\
        double eps = 1e-5;\
        double result = sc_calculate(expr, -1);\
        bool equal;\
        double diff = expected - result;\
        if (diff < 0) diff *= -1;\
        if (diff <= eps) {\
            success += 1;\
            equal = true;\
        }\
        else {\
            equal = false;\
        }\
        printf("{\n");\
        printf("  input  : %s\n", expr);\
        printf("  answer : %lf\n", (double)expected);\
        printf("  output : %lf\n", result);\
        printf("  status : %s\n", equal ? "SUCCESS" : "FAILURE");\
        printf("}\n");\
        if (!equal) return 0;\
    } while (0)

typedef struct {
    const char *input;
    double answer;
} TestCase;

typedef struct {
    TestCase *items;
    size_t count;
    size_t capacity;
} TestList;

bool get_test_cases(TestList *output)
{
    if (!output) return false;
    for (size_t i = 0; i < test_cases_len; i++) {
        const char *str = test_cases[i];
        if (!str) continue;
        const size_t str_len = strlen(str);
        if (str_len == 0) continue;
        size_t k;
        for (k = 0; str[k] != '#'; k++);
        if (k == 0) continue;
        char *input = malloc(sizeof(char) * (k + 1));
        memcpy(input, str, k);
        input[k] = '\0';
        k += 1;
        if (k >= str_len) {
            free(input);
            return false;
        };
        TestCase test;
        test.input = input;
        test.answer = strtod(&str[k], NULL);
        sc_list_append(*output, test);
    }

    return output->count > 0;
}

int main(void)
{
    size_t success = 0;

    TestList cases = {0};

    if (!get_test_cases(&cases)) {
        fprintf(stderr, "Failed to get test cases\n");
        return 1;
    }

    for (size_t i = 0; i < cases.count; i++) {
        EXPECTED(cases.items[i].input, cases.items[i].answer);
    }

    printf("%zu out of %zu tests succeeded\n", success, cases.count);

    return 0;
}
