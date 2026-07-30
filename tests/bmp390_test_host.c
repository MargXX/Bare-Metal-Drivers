#include "unity/unity.h"
#include "../BMP390/bmp390.c"


//command for initial compilation
// gcc -Wall -Wextra -std=c11 -Iplatform/rp2040 tests/bmp390_test_host.c tests/unity/unity.c tests/bmp390_i2c_fake.c tests/systick_fake.c -o tests/build/bmp390_test_host -lm

//-Iplatform/rp2040 — adds that directory to the include search path, which is what let i2c_platform.h resolve to rp2040 for now


#define REG_COUNT 256
extern uint8_t regs[];
extern uint32_t clock_ms;

//making this global for convienience
static bmp390_calib_t calib_data;
static bmp390_t dev;
static bool completed;

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

    dev = (bmp390_t){0};
    
    for (size_t i = 0; i<REG_COUNT; i++) {
        regs[i] = 0;
    }
    regs[BMP390_REG_CHIP_ID] = BMP390_CHIP_ID_VALUE;

    completed = false;

    clock_ms = 0;
}

void tearDown(void) {
    // clean stuff up here
}

//template so I dont forget
//TODO: clean this up after testing is set up
// void test_function_should_doBlahAndBlah(void) {
//     //test stuff
//     TEST_IGNORE();
// }

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


    //testing negatives
    calib_data.par_t1   = 1087;
    calib_data.par_t2   = 1091;
    calib_data.par_t3   = 1093;
    //no other changes
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));

    //whole equation
    calib_data = (bmp390_calib_t){
        .par_t1   = 0,
        .par_t2   = 0,
        .par_t3   = 0,
        .par_p1   = -3,
        .par_p2   = -5,
        .par_p3   = -7,
        .par_p4   = -11,
        .par_p5   = 13,
        .par_p6   = 17,
        .par_p7   = -19,
        .par_p8   = -23,
        .par_p9   = -29,
        .par_p10  = -31,
        .par_p11  = -37,
        .t_lin    = 1,
    };
    uncomp_press = 2;
    partial_out1 = 13 + 17 - 19 - 23;
    partial_out2 = 2 * (-3 - 5 - 7 - 11);
    partial_data4 = 2 * 2 * (-29 - 31 + 2 * -37);
    expected = partial_out1 + partial_out2 + partial_data4;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_pressure(&calib_data, uncomp_press));
    
}

//all together
void test_compensate_temperature_total(void) {
    //whole equation
    calib_data = (bmp390_calib_t){
        .par_t1   = 3,
        .par_t2   = 5,
        .par_t3   = 7,
        .par_p1   = 11,
        .par_p2   = 13,
        .par_p3   = 17,
        .par_p4   = 19,
        .par_p5   = 23,
        .par_p6   = 29,
        .par_p7   = 31,
        .par_p8   = 37,
        .par_p9   = 41,
        .par_p10  = 43,
        .par_p11  = 47,
        .t_lin    = 53,
    };
    uint32_t uncomp_temp = 2;
    float partial_data1 =  2 - 3;
    float partial_data2 = partial_data1 * 5;
    float expected = partial_data2 + partial_data1 * partial_data1 * 7;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_temperature(&calib_data, uncomp_temp));
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , calib_data.t_lin);

    //test nothing else matters
    calib_data = (bmp390_calib_t){
        .par_t1   = 3,
        .par_t2   = 5,
        .par_t3   = 7,
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
    uncomp_temp = 2;
    partial_data1 =  2 - 3;
    partial_data2 = partial_data1 * 5;
    expected = partial_data2 + partial_data1 * partial_data1 * 7;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_temperature(&calib_data, uncomp_temp));
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , calib_data.t_lin);

    //t1
    calib_data.par_t1   =-3;
    calib_data.par_t2   = 0;
    calib_data.par_t3   = 1;
    uncomp_temp = 0;
    partial_data1 =  3;
    partial_data2 = partial_data1 * 0;
    expected = partial_data2 + partial_data1 * partial_data1 * 1;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_temperature(&calib_data, uncomp_temp));
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , calib_data.t_lin);

    //t2
    calib_data.par_t1   = 0;
    calib_data.par_t2   = 5;
    calib_data.par_t3   = 0;
    uncomp_temp = 1;
    partial_data1 =  1;
    partial_data2 = partial_data1 * 5;
    expected = partial_data2 + partial_data1 * partial_data1 * 0;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_temperature(&calib_data, uncomp_temp));
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , calib_data.t_lin);

    //t3
    calib_data.par_t1   = 0;
    calib_data.par_t2   = 1;
    calib_data.par_t3   = 7;
    uncomp_temp = 1;
    partial_data1 =  1 - 0;
    partial_data2 = partial_data1 * 1;
    expected = partial_data2 + partial_data1 * partial_data1 * 7;
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , compensate_temperature(&calib_data, uncomp_temp));
    TEST_ASSERT_FLOAT_WITHIN(0.01, expected , calib_data.t_lin);
}

//mainly checking i2c_fake proper connection
void test_check_ID_uses_i2c(void) {
    uint8_t id = 0xFF;
    dev.addr = BMP390_I2C_ADDR_DEFAULT;
    dev.i2c_num = 0;

    //set ID reg to correct
    // regs[BMP390_REG_CHIP_ID] = BMP390_CHIP_ID_VALUE; //now in setup
    completed = bm_bmp390_read_chip_id(&dev,&id);
    
    TEST_ASSERT_EQUAL_INT32(BMP390_CHIP_ID_VALUE, id);
    TEST_ASSERT(completed);

    //set ID reg to incorrect
    regs[BMP390_REG_CHIP_ID] = 0x67; //funny number
    completed = bm_bmp390_read_chip_id(&dev,&id);
    
    TEST_ASSERT_EQUAL_INT32(0x67, id);
    TEST_ASSERT(completed);
}

void test_calibration_parse_syntheic(void) {
    //set up registers with real calib data
    uint8_t syn_dump[] = {//spacing for different pars
        0xF1, 0x01, 
        0xF2, 0x02, 
        0xF3,       //signed
        0x04, 0xF4, //signed
        0x05, 0xF5, //signed
        0xF6,       //signed
        0xF7,       //signed
        0x08, 0xF8, 
        0x09, 0xF9, 
        0xFA,       //signed
        0xFB,       //signed
        0x0C, 0xFC, //signed
        0xFD,       //signed
        0xFE,       //signed
    }; //have signed > 0x80 to confirm negatives
    //write fake to registers
    for (uint8_t i = 0; i<BMP390_CALIB_DATA_LEN; i++) {
        regs[BMP390_REG_CALIB_DATA + i] = syn_dump[i];
    } 
    //calculated expected answers from datasheet
    bmp390_calib_t c_exp = {
        .par_t1   =(uint16_t)(syn_dump[0] | ( syn_dump[1] << 8 )) * power_of_two(8),   
        .par_t2   =(uint16_t)(syn_dump[2] | ( syn_dump[3] << 8 )) / power_of_two(30),  
        .par_t3   = (int8_t)(syn_dump[4]) / power_of_two(48),  
        .par_p1   =((int16_t)(syn_dump[5] | ( syn_dump[6] << 8 ) ) - power_of_two(14)) / power_of_two(20),   
        .par_p2   =((int16_t)(syn_dump[7] | ( syn_dump[8] << 8 ) ) - power_of_two(14)) / power_of_two(29),   
        .par_p3   = (int8_t)(syn_dump[9]) / power_of_two(32),  
        .par_p4   = (int8_t)(syn_dump[10]) / power_of_two(37),  
        .par_p5   =(uint16_t)(syn_dump[11] | ( syn_dump[12] << 8 ) ) * power_of_two(3),   
        .par_p6   =(uint16_t)(syn_dump[13] | ( syn_dump[14] << 8 ) ) / power_of_two(6),   
        .par_p7   = (int8_t)(syn_dump[15]) / power_of_two(8),   
        .par_p8   = (int8_t)(syn_dump[16]) / power_of_two(15),  
        .par_p9   = (int16_t)(syn_dump[17] | ( syn_dump[18] << 8 ))/ power_of_two(48),  
        .par_p10  = (int8_t)(syn_dump[19])  / power_of_two(48),  
        .par_p11  = (int8_t)(syn_dump[20])  / power_of_two(65),  
        .t_lin = 0.0f, //doesnt matter
    };

    //parse performed by init
    
    completed = bm_bmp390_init(&dev, 0, BMP390_I2C_ADDR_DEFAULT);
    TEST_ASSERT(completed);
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_t1 ) * 0.01f + 1e-9f, c_exp.par_t1 , dev.calib.par_t1 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_t2 ) * 0.01f + 1e-9f, c_exp.par_t2 , dev.calib.par_t2 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_t3 ) * 0.01f + 1e-9f, c_exp.par_t3 , dev.calib.par_t3 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p1 ) * 0.01f + 1e-9f, c_exp.par_p1 , dev.calib.par_p1 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p2 ) * 0.01f + 1e-9f, c_exp.par_p2 , dev.calib.par_p2 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p3 ) * 0.01f + 1e-9f, c_exp.par_p3 , dev.calib.par_p3 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p4 ) * 0.01f + 1e-9f, c_exp.par_p4 , dev.calib.par_p4 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p5 ) * 0.01f + 1e-9f, c_exp.par_p5 , dev.calib.par_p5 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p6 ) * 0.01f + 1e-9f, c_exp.par_p6 , dev.calib.par_p6 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p7 ) * 0.01f + 1e-9f, c_exp.par_p7 , dev.calib.par_p7 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p8 ) * 0.01f + 1e-9f, c_exp.par_p8 , dev.calib.par_p8 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p9 ) * 0.01f + 1e-9f, c_exp.par_p9 , dev.calib.par_p9 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p10) * 0.01f + 1e-9f, c_exp.par_p10, dev.calib.par_p10);
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p11) * 0.01f + 1e-9f, c_exp.par_p11, dev.calib.par_p11);
    

}


// second precision fixture, real NVM capture
void test_calibration_parse_real_capture(void) {
    //set up registers with real calib data
    uint8_t real_dump[] = {//spacing for different pars
        0x20, 0x6D, 
        0xA7, 0x4D, 
        0xF9, 
        0xCD, 0x1B, 
        0x4D, 0x16, 
        0x06, 
        0x01, 
        0x3D, 0x4C, 
        0x5A, 0x5C, 
        0x03, 
        0xFA, 
        0xAA, 0x0F, 
        0x05, 
        0xF5,
    };
    //write to fake registers
    for (uint8_t i = 0; i<BMP390_CALIB_DATA_LEN; i++) {
        regs[BMP390_REG_CALIB_DATA + i] = real_dump[i];
    } 
    
    
    //calculated expected answers from datasheet
    bmp390_calib_t c_exp = {
        .par_t1   =(uint16_t)(real_dump[0] | ( real_dump[1] << 8 )) * power_of_two(8),   
        .par_t2   =(uint16_t)(real_dump[2] | ( real_dump[3] << 8 )) / power_of_two(30),  
        .par_t3   = (int8_t)(real_dump[4]) / power_of_two(48),  
        .par_p1   =((int16_t)(real_dump[5] | ( real_dump[6] << 8 ) ) - power_of_two(14)) / power_of_two(20),   
        .par_p2   =((int16_t)(real_dump[7] | ( real_dump[8] << 8 ) ) - power_of_two(14)) / power_of_two(29),   
        .par_p3   = (int8_t)(real_dump[9]) / power_of_two(32),  
        .par_p4   = (int8_t)(real_dump[10]) / power_of_two(37),  
        .par_p5   =(uint16_t)(real_dump[11] | ( real_dump[12] << 8 ) ) * power_of_two(3),   
        .par_p6   =(uint16_t)(real_dump[13] | ( real_dump[14] << 8 ) ) / power_of_two(6),   
        .par_p7   = (int8_t)(real_dump[15]) / power_of_two(8),   
        .par_p8   = (int8_t)(real_dump[16]) / power_of_two(15),  
        .par_p9   = (int16_t)(real_dump[17] | ( real_dump[18] << 8 ))/ power_of_two(48),  
        .par_p10  = (int8_t)(real_dump[19])  / power_of_two(48),  
        .par_p11  = (int8_t)(real_dump[20])  / power_of_two(65),  
        .t_lin = 0.0f, //doesnt matter
    };

    //parse performed by init
    completed = bm_bmp390_init(&dev, 0, BMP390_I2C_ADDR_DEFAULT);
    TEST_ASSERT(completed);
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_t1 ) * 0.01f + 1e-9f, c_exp.par_t1 , dev.calib.par_t1 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_t2 ) * 0.01f + 1e-9f, c_exp.par_t2 , dev.calib.par_t2 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_t3 ) * 0.01f + 1e-9f, c_exp.par_t3 , dev.calib.par_t3 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p1 ) * 0.01f + 1e-9f, c_exp.par_p1 , dev.calib.par_p1 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p2 ) * 0.01f + 1e-9f, c_exp.par_p2 , dev.calib.par_p2 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p3 ) * 0.01f + 1e-9f, c_exp.par_p3 , dev.calib.par_p3 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p4 ) * 0.01f + 1e-9f, c_exp.par_p4 , dev.calib.par_p4 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p5 ) * 0.01f + 1e-9f, c_exp.par_p5 , dev.calib.par_p5 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p6 ) * 0.01f + 1e-9f, c_exp.par_p6 , dev.calib.par_p6 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p7 ) * 0.01f + 1e-9f, c_exp.par_p7 , dev.calib.par_p7 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p8 ) * 0.01f + 1e-9f, c_exp.par_p8 , dev.calib.par_p8 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p9 ) * 0.01f + 1e-9f, c_exp.par_p9 , dev.calib.par_p9 );
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p10) * 0.01f + 1e-9f, c_exp.par_p10, dev.calib.par_p10);
    TEST_ASSERT_FLOAT_WITHIN(fabsf(c_exp.par_p11) * 0.01f + 1e-9f, c_exp.par_p11, dev.calib.par_p11);

}

void test_read_and_compensate_calculations(void) {
    
    calib_data = (bmp390_calib_t){
        .par_t1   = 3,
        .par_t2   = 5,
        .par_t3   = 7,
        .par_p1   = 11,
        .par_p2   = 13,
        .par_p3   = 17,
        .par_p4   = 19,
        .par_p5   = 23,
        .par_p6   = 29,
        .par_p7   = 31,
        .par_p8   = 37,
        .par_p9   = 41,
        .par_p10  = 43,
        .par_p11  = 47,
        .t_lin    = 53,
    };
    uint8_t press_temp_data[] = {
        //press first
        0x01, 0x81, 0xF1,
        //temp second
        0x02, 0x82, 0xF2,
    };
    for (uint8_t i = 0; i < BMP390_DATA_LEN; i++) {
        regs[BMP390_REG_DATA_0 + i] = press_temp_data[i];
    }
    //actual results
    dev.calib = calib_data;
    dev.addr = BMP390_I2C_ADDR_DEFAULT;
    dev.initialized = true;
    dev.configured = true;
    bmp390_data_t data_out;
    completed = read_and_compensate(&dev, &data_out);

    //expected answers
    uint32_t press_raw_exp = 
        (uint32_t)  press_temp_data[0] +   
                    (press_temp_data[1] << 8) + 
                    (press_temp_data[2] << 16);
    uint32_t temp_raw_exp = 
        (uint32_t)  press_temp_data[3] +   
                    (press_temp_data[4] << 8) + 
                    (press_temp_data[5] << 16);
    float exp_temp = compensate_temperature(&calib_data, temp_raw_exp);
    float exp_pres = compensate_pressure(&calib_data, press_raw_exp);

    TEST_ASSERT(completed);
    TEST_ASSERT_FLOAT_WITHIN(fabsf(exp_temp) * 0.01f + 1e-9f, exp_temp, data_out.temperature_c);
    TEST_ASSERT_FLOAT_WITHIN(fabsf(exp_pres) * 0.01f + 1e-9f, exp_pres, data_out.pressure_pa);
}

void test_read_and_compensate_guards(void) {
    bmp390_data_t data_out;
    completed = read_and_compensate(NULL, &data_out);
    dev.initialized = true;
    dev.configured = true;
    completed |= read_and_compensate(&dev, NULL);
    dev.initialized = false;
    dev.configured = true;
    completed |= read_and_compensate(&dev, &data_out);
    dev.initialized = true;
    dev.configured = false;
    completed |= read_and_compensate(&dev, &data_out);

    TEST_ASSERT(!completed);
}

// not needed when using generate_test_runner.rb
int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_power_of_two_exact);
    RUN_TEST(test_compensate_pressure_partial_out1);
    RUN_TEST(test_compensate_pressure_partial_out2);
    RUN_TEST(test_compensate_pressure_partial_data4);
    RUN_TEST(test_compensate_pressure_total);
    RUN_TEST(test_compensate_temperature_total);
    RUN_TEST(test_check_ID_uses_i2c);
    RUN_TEST(test_calibration_parse_syntheic);
    RUN_TEST(test_calibration_parse_real_capture);
    RUN_TEST(test_read_and_compensate_calculations);
    RUN_TEST(test_read_and_compensate_guards);
    return UNITY_END();
}