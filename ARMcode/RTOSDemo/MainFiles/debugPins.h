#ifndef DEBUGPINS_H
#define DEBUGPINS_H
// keep track of pin state
typedef enum {False = 0, True} bool;
bool initialVal = True;

bool updatePin(bool state);
bool initPin();
#endif