#include "unity/unity.h"
#include "../BMP390/bmp390.c"


//command for initial compilation
// gcc -Wall -Wextra -std=c11 -Iplatform/rp2040 tests/bmp390_test_host.c tests/unity/unity.c tests/stubs.c -o tests/bmp390_test_host -lm

//-Iplatform/rp2040 — adds that directory to the include search path, which is what let i2c_platform.h resolve to rp2040 for now

//making this global for convienience
static bmp390_calib_t calib_data;

void setUp(void) {
    // set stuff up here
    calib_data = (bmp390_calib_t){
        .par_t1   = 0,
        .par_t2   = 0,
        .par_t3   = 0,
        .par_p1   = 0,
        .par_p2   = 0,
        .par_p3   = 0,
        .par_p4   = 0,
        .par_p5   = 0,
        .par_p6   = 0,
        .par_p7   = 0,
        .par_p8   = 0,
        .par_p9   = 0,
        .par_p10  = 0,
        .par_p11  = 0,
        .t_lin    = 0,
    };
}

void tearDown(void) {
    // clean stuff up here
}

void test_function_should_doBlahAndBlah(void) {
    //test stuff
    TEST_IGNORE();
}

void test_power_of_two_exact(void) {
    //static float power_of_two(uint16_t pow);
    TEST_ASSERT_EQUAL_FLOAT(1.0f, power_of_two(0));
    TEST_ASSERT_EQUAL_FLOAT(2.0f, power_of_two(1));
    TEST_ASSERT_EQUAL_FLOAT(8.0f, power_of_two(3));
    TEST_ASSERT_EQUAL_FLOAT(32.0f, power_of_two(5));
    TEST_ASSERT_EQUAL_FLOAT(256.0f, power_of_two(8));
}

//group A
void test_compensate_pressure_partial_out1(void) {
    calib_data.par_p5   = 2;
    calib_data.par_p6   = 3;
    calib_data.par_p7   = 5;
    calib_data.par_p8   = 7;
    calib_data.t_lin    = 1;
    uint32_t uncomp_press = 11;
    float expected = 2 + 3 + 5 + 7;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //isolate p5
    calib_data.t_lin = 0;
    expected = 2;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //isolate p6
    calib_data.t_lin = 2;
    calib_data.par_p5 = 0;
    calib_data.par_p7 = 0;
    calib_data.par_p8 = 0;
    expected = 3 * 2;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //isolate p7
    calib_data.par_p6 = 0;
    calib_data.par_p7 = 5;
    expected = 5 * 2 * 2;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //isolate p8
    calib_data.par_p7 = 0;
    calib_data.par_p8 = 7;
    expected = 7 * 2 * 2 * 2;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));
}

//group B
void test_compensate_pressure_partial_out2(void) {
    //whole equation
    calib_data.par_p1 = 2;
    calib_data.par_p2 = 3;
    calib_data.par_p3 = 5;
    calib_data.par_p4 = 7;
    calib_data.t_lin = 1;
    uint32_t uncomp_press = 11;
    float expected = 11 * (2 + 3 + 5 + 7);
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //p1
    calib_data.t_lin = 0;
    uncomp_press = 11;
    expected = 11 * 2;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //p2
    calib_data.par_p1 = 0;
    calib_data.par_p2 = 3;
    calib_data.par_p3 = 0;
    calib_data.par_p4 = 0;
    calib_data.t_lin = 13;
    uncomp_press = 11;
    expected = 11 * 3 * 13;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //p3
    calib_data.par_p1 = 0;
    calib_data.par_p2 = 0;
    calib_data.par_p3 = 5;
    calib_data.par_p4 = 0;
    calib_data.t_lin = 13;
    uncomp_press = 11;
    expected = 11 * 5 * 13 * 13;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //p4
    calib_data.par_p1 = 0;
    calib_data.par_p2 = 0;
    calib_data.par_p3 = 0;
    calib_data.par_p4 = 7;
    calib_data.t_lin = 13;
    uncomp_press = 11;
    expected = 11 * 7 * 13 * 13 * 13;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //uncompress
    calib_data.par_p1 = 2;
    calib_data.par_p2 = 3;
    calib_data.par_p3 = 5;
    calib_data.par_p4 = 7;
    calib_data.t_lin = 13;
    uncomp_press = 0;
    expected = 0;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));
}

//group C
void test_compensate_pressure_partial_data4(void) {
    //whole equation
    calib_data.par_p9 = 2;
    calib_data.par_p10 = 3;
    calib_data.par_p11 = 5;
    calib_data.t_lin = 7;
    uint32_t uncomp_press = 11;
    float expected = 11 * 11 * (2 + 3 * 7 + 11 * 5);
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //p9
    calib_data.par_p9 = 2;
    calib_data.par_p10 = 0;
    calib_data.par_p11 = 0;
    calib_data.t_lin = 0;
    uncomp_press = 11;
    expected = 11 * 11 * 2;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //p10
    calib_data.par_p9 = 0;
    calib_data.par_p10 = 3;
    calib_data.par_p11 = 0;
    calib_data.t_lin = 2;
    uncomp_press = 11;
    expected = 11 * 11 * 2 * 3;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //p11
    calib_data.par_p9 = 0;
    calib_data.par_p10 = 0;
    calib_data.par_p11 = 5;
    calib_data.t_lin = 0;
    uncomp_press = 11;
    expected = 11 * 11 * 11 * 5;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));
}

//all together
void test_compensate_pressure_total(void) {
    //whole equation
    calib_data = (bmp390_calib_t){
        .par_t1   = 0,
        .par_t2   = 0,
        .par_t3   = 0,
        .par_p1   = 3,
        .par_p2   = 5,
        .par_p3   = 7,
        .par_p4   = 11,
        .par_p5   = 13,
        .par_p6   = 17,
        .par_p7   = 19,
        .par_p8   = 23,
        .par_p9   = 29,
        .par_p10  = 31,
        .par_p11  = 37,
        .t_lin    = 1,
    };
    uint32_t uncomp_press = 2;
    float partial_out1 = 13 + 17 + 19 + 23;
    float partial_out2 = 2 * (3 + 5 + 7 + 11);
    float partial_data4 = 2 * 2 * (29 + 31 + 2 * 37);
    float expected = partial_out1 + partial_out2 + partial_data4;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));
}


// not needed when using generate_test_runner.rb
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_power_of_two_exact);
    RUN_TEST(test_compensate_pressure_partial_out1);
    RUN_TEST(test_compensate_pressure_partial_out2);
    RUN_TEST(test_compensate_pressure_partial_data4);
    RUN_TEST(test_compensate_pressure_total);
    return UNITY_END();
}