#include "mbed.h"
#include "arm_book_lib.h"
#include <string> // Added to resolve std::string error
#include <chrono> // to try fix time issue
using namespace std; // Added for std namespace access

#define BLINKING_RATE     150ms
#define BLINKING_RATE2     500ms
#define DEBOUNCE_DELAY    50ms
#define STATE_ANNOUNCEMENT 5000ms
#define LOCKOUT_DURATION     60s

UnbufferedSerial uartUsb(USBTX, USBRX, 115200);
void pcSerialComStringWrite( const char* str );
void gas_temp_stats();
int checkforemergency(int alarm);

// Initialize pins
DigitalIn b1(BUTTON1);
DigitalIn d2(D2);
DigitalIn d3(D3);
DigitalIn d4(D4);
DigitalIn d5(D5);
DigitalIn d6(D6);
DigitalIn d7(D7);
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

//Reads a single button press with debounce and waits for release
char readButton() {
    while (true) {
        if (d4 == 1) {
            ThisThread::sleep_for(DEBOUNCE_DELAY);
            if (d4 == 1) {
                while (d4 == 1); // Wait for release
                return '1';
            }
        } else if (d5 == 1) {
            ThisThread::sleep_for(DEBOUNCE_DELAY);
            if (d5 == 1) {
                while (d5 == 1); // Wait for release
                return '2';
            }
        } else if (d6 == 1) {
            ThisThread::sleep_for(DEBOUNCE_DELAY);
            if (d6 == 1) {
                while (d6 == 1); // Wait for release
                return '3';
            }
        } else if (d7 == 1) {
            ThisThread::sleep_for(DEBOUNCE_DELAY);
            if (d7 == 1) {
                while (d7 == 1); // Wait for release
                return '4';
            }
        }
    }
}

int main() {
    b1.mode(PullDown);
    d2.mode(PullDown);
    d3.mode(PullDown);
    d4.mode(PullDown);
    d5.mode(PullDown);
    d6.mode(PullDown);
    d7.mode(PullDown);

    led1 = OFF;
    led2 = OFF;
    led3 = OFF;

    gas_temp_stats();

bool pressed = false;

    int alarm = 0;
    while (true) {
        if (d2 == 1 || d3 == 1) {
            alarm = 1;
            gas_temp_stats();
        }
        if (alarm == 1) {
            led2 = ON;
        }
        alarm = checkforemergency(alarm);
    }
}

int checkforemergency(int alarm) {
    string correctPassword = "1234"; //Configurable password
    string enteredPassword;
    int attempts = 0;
    int emergency = 0;

    if (d2 == 1 && d3 == 1) {
        emergency = 1;
        while (emergency == 1) {
            led2 = OFF;
            led3 = ON;
           pcSerialComStringWrite( "Emergency Protocal Active\r\n");
            ThisThread::sleep_for(STATE_ANNOUNCEMENT);

            while (attempts < 5) {
                enteredPassword = "";
                // Collect 4 button presses in order
                for (int i = 0; i < 4; i++) {
                    enteredPassword += readButton();
                    pcSerialComStringWrite( "Button Pressed\r\n");
                }

                // Wait for Enter (B1) button press
                while (b1 == 0) {
                    ThisThread::sleep_for(10ms);
                }
                ThisThread::sleep_for(DEBOUNCE_DELAY);

                if (enteredPassword == correctPassword) {
                    emergency = 0;
                    alarm = 0;
                    led1 = ON;
                    led2 = OFF;
                    led3 = OFF;
                    pcSerialComStringWrite( "Password Entered Correctly\n Emergancy Protocal Reset\r\n");
                    break;
                } else {
                    attempts++;
                    led1 = !led1;
                    ThisThread::sleep_for(BLINKING_RATE);
                    led1 = OFF;
                    pcSerialComStringWrite( "Password Attempt Made\r\n");
                    pcSerialComStringWrite( "Emergency Protocal Active\r\n");
                    pcSerialComStringWrite( "Temperature: ?\n Gas Level: ?\r\n");
                }
            }

            if (attempts >= 5) {
                // Lock system - turn off all LEDs
                led1 = OFF;
                led2 = OFF;
                led3 = OFF;
                int looptime = 0;
                pcSerialComStringWrite( "Emergency Protocal Active\r\n");
                pcSerialComStringWrite( "System Locked for 1 minute\r\n");
                while (looptime<120) {
                    // Locked state
                    led3 = !led3;
                    ThisThread::sleep_for(BLINKING_RATE2);
                    looptime++;
                }
               led3= ON;
                attempts = 0;
            }
        }
    }
    return alarm;
}
void pcSerialComStringWrite( const char* str )
{
    uartUsb.write( str, strlen(str) );
}
void gas_temp_stats(){
    if (d2 == 0){
        pcSerialComStringWrite("Temperature: Safe Level\n");
    }else if(d2 == 1){
        pcSerialComStringWrite("Temperature: Exceeded safe level\n");
    }if (d3 == 0){
        pcSerialComStringWrite("Gas: Safe Level\n");
    }else if(d3 == 1){
        pcSerialComStringWrite("Gas: Exceeded safe level\n");
    }
    ThisThread::sleep_for(STATE_ANNOUNCEMENT/2);
}
