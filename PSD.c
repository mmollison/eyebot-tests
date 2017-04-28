/**PSD (Position Sensitive Device) Interface
This is a program to test each Eyebot 7 API function related to PSDs
The API describes them as:

Position Sensitive Devices (PSDs) using infrared beams to measure distance.
The accuracy varies from sensor to sensor and needs to be calibrated in the HDT to get correct distance readings.
int PSDGet(int psd);                            // Read distance value from PSD sensor [1..6]
int PSDGetRaw(int psd);                         // Read raw value from PSD sensor [1..6]


*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "eyebot.h"
#include "types.h"


int main (){

	LCDSetPrintf(0,0,"Testing PSD Functions");
	LCDMenu("READ", "PAUSE", " ", "EXIT");
	int k = -1;

	 do {
		k = KEYRead(); //keyread for KEY4
		if (k==KEY1){
		do {
			k = KEYRead();
			if (k == KEY4)
				return 0;
			
			LCDSetPrintf(2, 0, "PSD 1 (LHS)");
			int returned = PSDGetRaw(1);
			LCDSetPrintf(3, 0, "Raw Value: %d", returned);
			returned = PSDGet(1);
			LCDSetPrintf(4, 0, "Cal.  Value: %d", returned);

			LCDSetPrintf(6, 0, "PSD 2 (FRONT)");
			returned = PSDGetRaw(2);
			LCDSetPrintf(7, 0, "Raw Value: %d", returned);
			returned = PSDGet(2);
			LCDSetPrintf(8, 0, "Cal.  Value: %d", returned);

			LCDSetPrintf(10, 0, "PSD 3 (RHS)");
			returned = PSDGetRaw(3);
			LCDSetPrintf(11, 0, "Raw Value: %d", returned);
			returned = PSDGet(3);
			LCDSetPrintf(12, 0, "Cal.  Value: %d", returned);
			LCDSetPrintf(3, 30, "Raw values should read approx:");

			LCDSetPrintf(5, 30, "4000,2000,1000 @ 10,35,40cm respectively");
			
		} while (k != KEY2);
		}
		else if (k ==KEY4){
			return 0;
		}
	} while (k != KEY4);
	

	return 0;
	
	
	
}



