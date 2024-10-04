#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "C:/Program Files (x86)/LabJack/Drivers/LabJackUD.h"

int main() {
    LJ_ERROR lj_cue;
    LJ_HANDLE lj_handle = 0;
    double sensorAIN1;
    int counter = 0;
    int exitStrategy, activations, buzzerActivations = 0, sensorBlockedCount = 0, firstBlock = 1;
    int duration = 0;

    // Initialize LabJack
    lj_cue = OpenLabJack(LJ_dtU3, LJ_ctUSB, "1", 1, &lj_handle);
    lj_cue = ePut(lj_handle, LJ_ioPIN_CONFIGURATION_RESET, 0, 0, 0);

    // Configure PWM signal for buzzer on FIO4
    lj_cue = AddRequest(lj_handle, LJ_ioPUT_CONFIG, LJ_chTIMER_COUNTER_PIN_OFFSET, 4, 0, 0);
    lj_cue = AddRequest(lj_handle, LJ_ioPUT_CONFIG, LJ_chTIMER_CLOCK_BASE, LJ_tc1MHZ_DIV, 0, 0);
    lj_cue = AddRequest(lj_handle, LJ_ioPUT_CONFIG, LJ_chTIMER_CLOCK_DIVISOR, 1, 0, 0);
    lj_cue = AddRequest(lj_handle, LJ_ioPUT_CONFIG, LJ_chNUMBER_TIMERS_ENABLED, 1, 0, 0);
    lj_cue = AddRequest(lj_handle, LJ_ioPUT_TIMER_MODE, 0, LJ_tmPWM8, 0, 0);
    lj_cue = Go();

    // Turn off LED on DAC0 initially
    lj_cue = ePut(lj_handle, LJ_ioPUT_DAC, 1, 5.0, 0);

    // Welcome message and ask for exit strategy
    printf("Welcome to the HSI255 Project program.\n");
    printf("Choose exit strategy:\n");
    printf("1. Time (seconds)\n");
    printf("2. Number of buzzer activations\n");
    printf("Choose your exit: ");
    scanf("%d", &exitStrategy);

    if (exitStrategy == 1) {
        printf("Enter the duration in seconds: ");
        scanf("%d", &duration);
    } else if (exitStrategy == 2) {
        printf("Enter the number of buzzer activations: ");
        scanf("%d", &activations);
    } else {
        printf("Invalid selection. Exiting program.\n");
        return 1;
    }

    printf("Press any key to start the program...\n");
    getchar(); // To capture the enter key after scanf
    getchar(); // To capture the actual key press

    // Main loop
    while ((exitStrategy == 1 && counter < duration * 4) || (exitStrategy == 2 && sensorBlockedCount < activations)) {
        // Read sensor state
        lj_cue = AddRequest(lj_handle, LJ_ioGET_AIN, 1, 0, 0, 0); // Optical sensor on AIN1
        lj_cue = Go();
        lj_cue = GetResult(lj_handle, LJ_ioGET_AIN, 1, &sensorAIN1);

        if (sensorAIN1 > 1.0) { // Sensor is unblocked
            lj_cue = ePut(lj_handle, LJ_ioPUT_DAC, 1, 0.0, 0); // Turn on LED on DAC0
            lj_cue = ePut(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 0, 0); // Turn off buzzer on FIO4
            printf("Sensor is unblocked. LED is ON. Buzzer is OFF.\n");
        } else { // Sensor is blocked
            if (firstBlock) {
                sensorBlockedCount++;
                firstBlock = 0;
                printf("Sensor Blocked. Count: %d\n", sensorBlockedCount);
            }
            lj_cue = ePut(lj_handle, LJ_ioPUT_DAC, 1, 5.0, 0); // Turn off LED on DAC0
            lj_cue = ePut(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 32768, 0); // Turn on buzzer on FIO4
            printf("Sensor is blocked. LED is OFF. Buzzer is ON.\n");
        }

        Sleep(250); // Wait 250 ms (4 times per second)
        counter++;

        if (sensorAIN1 > 1.0) {
            firstBlock = 1;
        }
    }

    // Turn off LED and buzzer
    lj_cue = ePut(lj_handle, LJ_ioPUT_DAC, 1, 5.0, 0); // Turn off LED on DAC0
    lj_cue = ePut(lj_handle, LJ_ioPUT_TIMER_VALUE, 0, 0, 0); // Turn off buzzer on FIO4
    printf("Program ended. LED and buzzer are turned off.\n");

    Close();
    return 0;
}
