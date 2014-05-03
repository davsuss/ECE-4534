#include "roverKinematics.h"

/*
Purpose: This function is an optimized version of sine and cosing provided by math.h
		  sine(x) = A(x-pi)+(4/pi^2)(x-pi)abs(x-pi)
		  Approximate for [0,360]
*/
float sine_optimized( float angle )
{
#if useOpt == 1
	float firstTerm = A*(angle - pi);
	float secondTerm = B*(angle - pi)*fabs(angle - pi);
	return (int)((double)(firstTerm + secondTerm));
#else
	return sin(angle);
#endif
}
/*
Purpose: This function is an optimized version of sine and cosing provided by math.h
		  sine(x) = A(x-pi)+(4/pi^2)(x-pi)abs(x-pi)
		  cosine(x) = sine(x + pi/2)
		  Approximate for [0,360]
*/
float cosine_optimized( float angle )
{
	//cos(x) = sin(x+pi/2)
	return cos(angle);
}

void convertCoordinate(int refX, int refY, float orientation, int newX, int newY)
{
	float cosine = cosine_optimized(orientation * convToRadians);
	float sine = sine_optimized(orientation * convToRadians);
	float convX = cosine * (float)newX - sine*(float)newY + refX;
	float convY = sine*(float)newX + cosine*(float)newY + refY;
	resultOfRV_Translation.x = (int)((double)round(convX));
	resultOfRV_Translation.y = (int)((double)round(convY));
}

/**
 * Purpose: Given the rover's pose, this function will move it a certain distance
 * ARGS:
 * ref_x: x-coordinate of reference point (Rover)
 * ref_y: y-coordinate of reference point (Rover)
 * orientation: orientation of rover
 * distance: Distance the rover moved (center of rover) 
**/
void translateRoverVector(int ref_x, int ref_y, float orientation, 
							int rightFt, int rightIn, int leftFt, int leftIn)
{
	int rightDist = convToInches(rightFt, rightIn);
	int leftDist = convToInches(leftFt, leftIn);

	//printf("rightDist = %d\n", (int)((double)rightDist));
	//printf("leftDist = %d\n", (int)((double)leftDist));

	if( rightDist != leftDist )
	{
		//Radius of Turn
		int R = ((float)rov_width_half)*( ( rightDist + leftDist ) / ( rightDist - leftDist ) );
		//Change in orientation
		float theta = ((float)rightDist - (float)leftDist)/((float)rov_width);
#if smoothenMap == 1
		theta = (fabs((int)((double)(theta*convToDeg))) == 93.0) ? 
						(((theta < 0.0)?-1.0:1.0)*(float)90.0)*convToRadians : theta;
#endif 
		//printf("theta = %d", (int)((double)(theta*convToDeg)));	   
		// calculate cosine and sine of the angle (angle inputted is in degrees)
		float cosine_trav = cosine_optimized(theta);
		float sine_trav = sine_optimized(theta);
		float cosine_orien = cosine_optimized(orientation * convToRadians);
		float sine_orien = sine_optimized(orientation * convToRadians);
		//ICC - center of rotation
		int ICC_x = 0 - R*sine_orien;
		int ICC_y = 0 + R*cosine_orien;												   

		int newx = cosine_trav*(0 - ICC_x) - sine_trav*(0 - ICC_y) + ICC_x;
		int newy = sine_trav*(0- ICC_x) + cosine_trav*(0- ICC_y) + ICC_y;
		resultOfRV_Translation.orientation = orientation + (theta * convToDeg);
		convertCoordinate(ref_x, ref_y, resultOfRV_Translation.orientation, newx, newy);
	}
	else
	{
		resultOfRV_Translation.orientation = orientation;
		convertCoordinate(ref_x, ref_y, orientation, rightDist, (int)0);
	}
}

uint32_t convToInches(int feet, int inches)
{
	return (feet*InchesInOneFeet) + inches;
}

uint32_t calcDistance(int x1, int y1, int x2, int y2)
{
	return sqrt(pow((x2 - x1),2) + pow((y2 - y1),2));
}
