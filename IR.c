/**IR Comms
	This is a program to test each Eyebot 7 API function related to IR Remote Control
	The API describes them as:

	These commands allow sending commands to an EyeBot via a standard infrared TV remote (IRTV). IRTV models can be enabled or disabled via a HDT entry.
	Supported IRTV models are: Chunghop L960E Learn Remote
	int IRTVGet(void);                              // Blocking read of IRTV command
	int IRTVRead(void);                             // Non-blocking read, return 0 if nothing
	int IRTVFlush(void);                            // Empty IRTV buffers
	int IRTVGetStatus(void);                        // Checks to see if IRTV is activated (1) or off (0)
	Defined Constants for IRTV buttons are:
	IRTV_0 .. IRTV_9, IRTV_RED, IRTV_GREEN, IRTV_YELLOW, IRTV_BLUE,
	IRTV_LEFT, IRTV_RIGHT, IRTV_UP, IRTV_DOWN, IRTV_OK, IRTV_POWER

*/

#include <stdio.h>
#include <stdlib.h>
#include <eyebot.h>
static int k, IR_Got, IR_Read, IR_Stat, IR_Flush;

int main (){
    int i = 0;
	IR_Stat = IRTVGetStatus();	
start:

	k = -1; //keyread
    
    do{
        LCDSetPrintf(0,0,"Testing IR Functions");

        if (IR_Stat){
            LCDSetPrintf(2,1,"IRTVGetStatus Succeeded, IR input enabled");
            LCDSetPrintf(3,1,"Select which function you would like to test");
            LCDMenu("IRTVGet","IRTVREAD","IRTVFlush","EXIT");
            }
        else{
            LCDSetPrintf(2,1,"IRTVGetStatus failed, ensure sensor is attached and enabled in hdt.txt");
               
            LCDSetPrintf(4,1,"exiting program");
            OSWait(1000);
            exit(0);
        }
	
	    k=-1;

        k = KEYGet();
		switch(k){
            case KEY1: //get
                i = 8;

                IR_Got = -1;
                do {
                    k = -1;
                                        LCDSetPrintf(5,2,"Testing IRTVGet (blocking)");
                    if (IR_Got != -1){
                        LCDSetPrintf(7,3,"GOT:   %d", IR_Got);
                        LCDSetPrintf(9,i,"%d ", IR_Read);
                        i++;
                    }

                    LCDSetPrintf(10,2,"Press GET to read buffer,");
                    LCDSetPrintf(11,2,"Press BACK to return to function selection, and");
                    LCDSetPrintf(12,2,"Press EXIT to loop for eternity.");
                    LCDMenu("GET","","BACK","EXIT");
                    k = KEYGet();
                    switch(k){
                        case KEY1:
                            IR_Got = IRTVGet();
                            k=-1;
                            break;
                        case KEY2:
                            k=-1;
                            break;
                        case KEY3:
                            k=-1;
                            goto start;
                        case KEY4:
                            exit(0);
                            break;

                    }
                }while(1);
            case KEY2:
                IR_Read = -1;
                i = 8;
                do {
                    k = -1;
                    
                    LCDSetPrintf(5,2,"Testing IRTVRead (nonblocking)");
                    if (IR_Read != -1){
                        LCDSetPrintf(7,3,"Read:");
                        LCDSetPrintf(8,i,"%d ", IR_Read);
                        i++;
                    }

                    LCDSetPrintf(10,2,"Press READ to read buffer,");
                    LCDSetPrintf(11,2,"Press BACK to return to function selection, and");
                    LCDSetPrintf(12,2,"Press EXIT to loop for eternity.");
                    LCDMenu("READ","","BACK","EXIT");
                    k = KEYGet();
                    switch(k){
                        case KEY1:
                            IR_Got = IRTVGet();
                            k=-1;
                            break;
                        case KEY2:
                            k=-1;
                            break;
                        case KEY3:
                            k=-1;
                            goto start;
                        case KEY4:
                            exit(0);
                            break;
                        default:
                            k=-1;
                            break;

                    }
                }while(1);
            case KEY3: //je ne sais pas?
                IR_Read = -1;
                i = 8;

                do {
                    k = -1;
                    IR_Read = IRTVRead();
                    LCDSetPrintf(5,2,"Testing IRTVFlush (somehow)");
                    LCDSetPrintf(7,3,"Next item in buffer is: ");
                    LCDSetPrintf(8,i,"%d ", IR_Read);
                    LCDSetPrintf(10,2,"Press FLUSH to flush buffer,");
                    LCDSetPrintf(11,2,"Press BACK to return to function selection, and");
                    LCDSetPrintf(12,2,"Press EXIT to loop for eternity.");
                    LCDMenu("READ","","BACK","EXIT");
                    k = KEYGet();
                    switch(k){
                        case KEY1:
                            IR_Flush = IRTVFlush();
                            k=-1;
                            break;
                        case KEY2:
                            k=-1;
                            break;
                        case KEY3:
                            k=-1;
                            goto start;
                        case KEY4:
                            exit(0);
                            break;
                        default:
                            k=-1;
                            break;

                    }
                }while(1);
            case KEY4:
                exit(0);
        }



            
            
          			
			
		}while(1);
		
		
	
	
	
	return 0;
}



