#include "gpio.h"

int main() {

    // bare metal programs never return from main
    // potential crash if we return from main, so just loop forever
    while(1) {}   
}