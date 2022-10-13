// Lab 2
// ENGR-2350
// Name: Julian Chen
// RIN: 662024294

#include "engr2350_msp432.h"
#include <stdlib.h>


// Add function prototypes here, as needed.
void Timer_Init();
void Timer_ISR();
void GPIOInit();
void populateArray();
void colorControl();
void GetInputs();
void showColors();
void userGuess();
void failed();
void victory();


// Add global variables here, as needed.
uint8_t colorArray[10];
uint32_t timer = TIMER_A1_BASE;
uint8_t startbtn;//pushbutton to start the game
//bumpers
uint8_t bmp0;
uint8_t bmp1;
uint8_t bmp2;
uint8_t bmp3;
uint8_t bmp4;
uint8_t bmp5;
uint8_t currentColor = 0;
Timer_A_UpModeConfig config;
uint8_t buttonCounter = 0;// time between button presses. flag
uint8_t failFlag = 0;
uint8_t victoryFlag = 0;


int main(void)
{

    SysInit();
    GPIOInit();
    populateArray();

    printf("INSTRUCTIONS: The RSLK will flash a sequence of colors.\r\n"
            "Using the bumpers on the RSLK, repeat the sequence that was displayed.");


    while(1){

        //while startbtn not pushed, press bumpers and get colors.
        colorControl();
        GetInputs();

        //if startbtn pushed, BiLED turns red and game begins
        if(startbtn){
            //delay 1 second
            __delay_cycles(24e6);
            while(1){
                GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN5);
                GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN4);
                showColors();
                userGuess();
                if(failFlag){
                    failed();
                    break;
                }
                else if(victoryFlag){
                    victory();
                    break;
                }
                else{
                currentColor++;
                }
            }
        }
        // BiLED is off when start button isn't pressed
        else{
            GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN5 | GPIO_PIN5);
        }


   }


}

// Add function declarations here as needed
void Timer_Init(){
    config.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    config.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_64;
    config.timerPeriod = 1125000;//3 second period
    config.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_ENABLE;
    Timer_A_registerInterrupt(timer, TIMER_A_CCRX_AND_OVERFLOW_INTERRUPT, Timer_ISR);
    Timer_A_configureUpMode(timer, &config);
    Timer_A_startCounter(timer, TIMER_A_UP_MODE);
}

void Timer_ISR(){
    Timer_A_clearInterruptFlag(timer);// acknowledge the interrupt
    buttonCounter++;
}

void GPIOInit(){
    // Add initializations of inputs and outputs
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN0 | GPIO_PIN1 | GPIO_PIN2);// sets RGB LED as output
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN4 | GPIO_PIN5); // Set P2.4 and P2.5 to output BiLED
    GPIO_setAsInputPinWithPullDownResistor(GPIO_PORT_P3, GPIO_PIN2); // Set P3.2 startbtn to input

    GPIO_setAsInputPinWithPullUpResistor( GPIO_PORT_P4, GPIO_PIN0 | GPIO_PIN2 | GPIO_PIN3 | GPIO_PIN5 | GPIO_PIN6 | GPIO_PIN7); //BMP0-BMP5
}

//get inputs of start button and bumpers
void GetInputs(){
    startbtn = GPIO_getInputPinValue(GPIO_PORT_P3,GPIO_PIN2);
    bmp0 = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0);
    bmp1 = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2);
    bmp2 = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3);
    bmp3 = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5);
    bmp4 = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6);
    bmp5 = GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7);
}

//RSLK Will flash the sequence of colors in the colorArray
void showColors(){
    uint8_t i;
    for(i = 0; i <= currentColor; i++){
        //if 0, bmp0 RED
        if (colorArray[i] == 0){
            //flash for half a second
            __delay_cycles(12e6);
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
            __delay_cycles(12e6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
        }
        //if 1, bmp1 GREEN
        else if (colorArray[i] == 1){
            __delay_cycles(12e6);
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
            __delay_cycles(12e6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
        }
        //if 2, bmp2 BLUE
        else if (colorArray[i] == 2){
            __delay_cycles(12e6);
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
            __delay_cycles(12e6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
        }
        //if 3, bmp3 YELLOW
        else if (colorArray[i] == 3){
            __delay_cycles(12e6);
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN0);
            __delay_cycles(12e6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN0);
        }
        //if 4, bmp4 PURPLE
        else if (colorArray[i] == 4){
            __delay_cycles(12e6);
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN0);
            __delay_cycles(12e6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN0);
        }
        //if 5, bmp5 CYAN
        else if (colorArray[i] == 5){
            __delay_cycles(12e6);
            GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN2);
            __delay_cycles(12e6);
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN2);
        }
    }
}

//BiColor will light green and user will guess the sequence
void userGuess(){
    //BiLED Green and tracker i
    GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN4);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN5);
    uint8_t i = 0;
    uint8_t add3 = 0;//2nd flag.

    //while time is under 3 seconds
    while(buttonCounter == 0 || add3){

        //if user has reached the end of the sequence
        if(i == currentColor){
            //if user has reached 9th element, set victory flag
            if(i == 9){
                victoryFlag = 1;
            }
            break;
        }

        add3 = 0;// 2nd flag
        //if bmp0 RED pressed, while bmp0 held, show red. Make decision after letting go of bmp0. i++ and reset timer if correct.
        if(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)){
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);
            //if correct, add 3 sec, increment i
            if(colorArray[i] == 0){
                add3 = 1;
                i++;
            }else{
                //else failed
                failFlag = 1;
                break;
            }

        }
        //bmp1 green pressed
        else if(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2)){
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);
            //if correct, add 3 sec
            if(colorArray[i] == 1){
                add3 = 1;
                i++;
            }else{
                //else failed
                failFlag = 1;
                break;
            }
        }
        //bmp2 blue pressed
        else if(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3)){
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);
            //if correct, add 3 sec
            if(colorArray[i] == 2){
                add3 = 1;
                i++;
            }else{
                //else failed
                failFlag = 1;
                break;
            }
        }
        //bmp3 yellow pressed
        else if(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5)){
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN0);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN0);
            //if correct, add 3 sec
            if(colorArray[i] == 3){
                add3 = 1;
                i++;
            }else{
                //else failed
                failFlag = 1;
                break;
            }
        }
        //bmp4 purple pressed
        else if(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6)){
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN0);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN0);
            //if correct, add 3 sec
            if(colorArray[i] == 4){
                add3 = 1;
                i++;
            }else{
                //else failed
                failFlag = 1;
                break;
            }
        }
        //bmp5 purple pressed
        else if(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7)){
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN1);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN1);
            //if correct, add 3 sec
            if(colorArray[i] == 5){
                add3 = 1;
                i++;
            }else{
                //else failed
                failFlag = 1;
                break;
            }
        }

        //
        if(buttonCounter !=1 && add3 != 1){
            failFlag = 1;
            break;
        }


    }


}

void failed(){
    //while btn isnt pressed, flash the BiLED Red
    printf("INCORRECT");
    while(!startbtn){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN5);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN4);
        __delay_cycles(6e6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN5 | GPIO_PIN5);
    }
}

void victory(){
    //while btn isnt pressed, flash the BiLED Green
    printf("YOU HAVE WON!");
    while(!startbtn){
        GPIO_setOutputHighOnPin(GPIO_PORT_P2,GPIO_PIN4);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN5);
        __delay_cycles(6e6);
        GPIO_setOutputLowOnPin(GPIO_PORT_P2,GPIO_PIN5 | GPIO_PIN5);
    }
}

void colorControl(){
            //if bmp0 is touching, RGB RED
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN0)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN0);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN0);


            //while bmp1 is touching, RGB GREEN
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN2)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1);

            //while bmp2 is touching, RGB BLUE
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN3)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2);

            //while bmp3 is touching, RGB YELLOW
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN0);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN0);

            //while bmp4 is touching, RGB PURPLE
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN6)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN0);
            }
            GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN0);

            //while bmp5 is touching, RGB CYAN
            while(!GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN7)){
                  GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN1 | GPIO_PIN2);
            }
              GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN2 | GPIO_PIN1);

}

//populates color array. random int from 0-5
void populateArray(){
    uint8_t i;
    for(i = 0; i < 10; i++){
        colorArray[i] = rand()%6;
        printf("added %u to color array\r\n", colorArray[i]);
    }
}
