#include <Arduino.h>

#define MOTOR_ON 1
#define MOTOR_OFF 0
#define dustHIGH 1
#define dustLOW 0
#define Auto_On 1
#define Auto_Off 0

class dust{
  private:
    int pin; int A; int B; byte motor; byte state; byte autoM;
    float dust_value;
    float dustDensityug;

  public:
    dust(int A, int B);
    void Minit();
    void getDvalue(int pin);
    float getden();
    void checkState();
    void actionon();
    void actionoff();
    byte getMotor();
    byte getState();
    void actionAutoOn();
    void actionAutoOff();
    byte getAuto();
};
