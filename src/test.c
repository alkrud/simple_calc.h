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

bool get_test_cases(TestCase* *output, size_t *count)
{
    for (size_t i = 0; i < test_cases_len; i++) {
        size_t k = 0;
        const char *str = test_cases[i];
        for (; test_cases[i] 
    }
}

int main(void)
{
    size_t success = 0;

    TestCase *cases;
    size_t total;

    if (!get_test_cases(&cases, &total)) {
        fprintf(stderr, "Failed to get test cases\n");
        return 1;
    }

    for (size_t i = 0; i < total; i++) {
        EXPECTED(cases[i].input, cases[i].answer);
    }

    printf("%zu out of %zu tests succeeded\n", success, total);

    return 0;
}
