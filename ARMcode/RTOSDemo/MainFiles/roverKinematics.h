#ifndef ROVERKINEMATICS_H
#define ROVERKINEMATICS_H

/*****************************
 * AUTHOR: Bishwamoy Sinha Roy
 *****************************/

#include <stdint.h>     
#include <math.h>       

#define rov_width 11 // width of the rover for kinemetic equations
#define rov_width_half 6
#define pi (float)3.14 // pi used for transformations
#define pi_half (float)1.57  // pi/2
#define A -( ( (float)4.0 ) / pi ) // used for the optimized sine and cosine operations {sine(x) = A(x-pi)+(4/pi^2)(x-pi)abs(x-pi)}
#define B ( (float)4.0 )/(pi*pi)
#define convToRadians pi / (float)180.0
#define convToDeg ( ((float)180.0) / pi )
#define InchesInOneFeet 12
#define distInNinetyDeg ((float)(45.0))*((float)rov_width)*pi / ((float) 180.0)

//use optimization (will have error)
#define useOpt 0
#define smoothenMap 0

typedef struct __rvPoseStruct {
	int x;
	int y;
	float orientation;
} rvPoseStruct;

rvPoseStruct resultOfRV_Translation;

/*
Purpose: This function is an optimized version of sine and cosing provided by math.h
		  sine(x) = A(x-pi)+(4/pi^2)(x-pi)abs(x-pi)
		  Approximate for [0,360]
*/
float sine_optimized( float angle );

/*
Purpose: This function is an optimized version of sine and cosing provided by math.h
		  sine(x) = A(x-pi)+(4/pi^2)(x-pi)abs(x-pi)
		  cosine(x) = sine(x + pi/2)
		  Approximate for [0,360]
*/
float cosine_optimized( float angle );

void convertCoordinate(int refX, int refY, float orientation, int newX, int newY);

/**
 * Purpose: Given the rover's pose, this function will move it a certain distance
 * ARGS:
 * ref_x: x-coordinate of reference point (Rover)
 * ref_y: y-coordinate of reference point (Rover)
 * orientation: orientation of rover 
**/
void translateRoverVector(int ref_x, int ref_y, float orientation, 
							signed int rightFeet, signed int rightInches, signed int leftFeet, signed int leftInches);

uint32_t calcDistance(int x1, int y1, int x2, int y2);
uint32_t convToInches(int feet, int inches);
#endif
							   
