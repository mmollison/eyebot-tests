/**V-Omega Driving Interface
This is a program to test each Eyebot 7 API function related to the V-Omega Driving interface
The API describes them as:

This is a high level wheel control for differential driving. It always uses motor 1 (left) and motor 2 (right).
Motor spinning directions, motor gearing and vehicle width are set in the HDT file.
int VWSetSpeed(int linSpeed, int angSpeed);     // Set fixed linSpeed  [mm/s] and [degrees/s]
int VWGetSpeed(int *linSspeed, int *angSpeed);  // Read current speeds [mm/s] and [degrees/s]
int VWSetPosition(int x, int y, int phi);       // Set robot position to x, y [mm], phi [degrees]
int VWGetPosition(int *x, int *y, int *phi);    // Get robot position as x, y [mm], phi [degrees]
int VWStraight(int dist, int lin_speed);        // Drive straight, dist [mm], lin. speed [mm/s]
int VWTurn(int angle, int ang_speed);           // Turn on spot, angle [degrees], ang. speed [degrees/s]
int VWCurve(int dist, int angle, int lin_speed);// Drive Curve, dist [mm], angle (orientation change) [degrees], lin. speed [mm/s]
int VWDrive(int dx, int dy, int lin_speed);     // Drive x[mm] straight and y[mm] left, x>|y|
int VWDriveRemain(void);                        // Return remaining drive distance in [mm]
int VWDriveDone(void);                          // Non-blocking check whether drive is finished (1) or not (0)
int VWDriveWait(void);                          // Suspend current thread until drive operation has finished
int VWStalled(void);                            // Returns number of stalled motor [1..2], 3 if both stalled, 0 if none
All VW functions return 0 if OK and 1 if error (e.g. destination unreachable)

*/


#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "eyebot.h"

#define ANG      360
#define DIST     100 		// 10cm
//#define SPEED    100		// 100mm/s linear
//#define ASPEED    15		// 15 deg/s
//static int SelTest(void);       // in main
static int SetSpeed(void);      //0
static int GetSpeed(void);      //1
//static int SetPos(void);        //2 Set to 0,0,0
//static int GetPos(void);        //3 in main
//static int Straight(void);      //4 in main, edit to set DIST maybe
//static int Turn(void);          //5 in main, edit to set angle maybe
//static int Curve(void);         //6 in main, edit to set dist and angle maybe
//static int Drive(void);         //7 in main, edit to set dx and dy maybe
//static int DriveRemain(void);   //8
static int DriveWait(void);     //9
static int Stalled(void);       //10

static char* tests[] = {"Set Speed", "Get Speed", "Set Position", "Get Position", "Drive Straight", "Turn", "Drive Curve", "Drive", "Drive Remaining", "Drive Done", "Drive Wait", "Drive Done", "Exit"};
static int speeds[] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180,190,200};
static int angles[] = {0,10,20,30,40,50,60,70,80,90,100,110,120,130,140,150,160,170,180};
static int k = -1; //keycode
static int SPEED = 100;         //100 mm/s
static int ASPEED = 20;         //15 deg/s


static int GetSpeed(void){
    int ret = -1;
    ret = VWGetSpeed(SPEED,ASPEED); //or *SPEED or &SPEED? pointers...

    return ret;
}

static int SetSpeed(void){

    int ret = -1;               //return status
    int i = 0;                  //index for speed
    int j = 0;                  //index for aspeed
    k=NULL;
    do{
        if (i==21){
            i = 0;
        }
        if (j==19){
            j=0;
        }
        SPEED = speeds[i];
        ASPEED = angles[j];
        LCDClear();
        LCDSetPrintf(0,0, "V-W Set Speed Tester");
        LCDSetPrintf(2,1, "Press SPEED and ASPEED to adjust");
        LCDSetPrintf(3,1, "Press SET to confirm");
        LCDSetPrintf(5,2, "Selected speed: %d", SPEED);
        LCDSetPrintf(5,2, "Selected angular speed: %d", ASPEED);
        LCDMenu("SET","SPEED","ASPEED","EXIT");
        k = -1;
        k = KEYGet();
        switch (k) {

            case KEY1:
                ret = VWSetSpeed(SPEED,ASPEED);
                break;
            case KEY2: 
                i++;
                break;
            case KEY3: 
                j++;
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);
        }

    }while (k!=1);
    return ret;
}






int main (){
    int i = 0; //which function	
    int ret = -1; //return val
    do{
        if (i == -1)
            i = 11;
        if (i == 12)
            i = 0;

        LCDClear();
        LCDSetPrintf(0,0, "V-W Drive Function Tester");
        LCDSetPrintf(2,1, "Select which function you wish to test");
        LCDSetPrintf(3,1, "Note some functions may require other test functions to be run first");
        LCDSetPrintf(5,2, "Selected Function:");
        LCDSetPrintf(6,10,"%s", tests[i]);
        LCDMenu("PREV","SELECT","NEXT","EXIT");
        k = -1;
        
        k = KEYGet();
        switch (k) {

            case KEY1:
                i--;
                break;
            case KEY2: 
            
                switch(i) {
                    case 0:
                        ret = SetSpeed();
                        LCDSetPrintf(8,1,"Attempted VWSetSpeed, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        break;
                    case 1:
                        ret = GetSpeed();
                        LCDSetPrintf(8,1,"Attempted VWGetSpeed, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        LCDSetPrintf(10,1,"Got lin.spd = %d and ang.spd = %d", SPEED,ASPEED);
                        break;
                    case 2:
                        ret = VWSetPosition(0,0,0);

                        LCDSetPrintf(8,1,"Attempted VWSetPosition to 0,0,0, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        break;
                    case 3:
                        int x =-1;
                        int y = -1;
                        int phi = -1;
                        ret = VWGetPosition(x,y,phi);

                        LCDSetPrintf(8,1,"Attempted VWGetPosition, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        LCDSetPrintf(10,1,"Got x: %d, y: %d, phi: %d",x,y,phi);                        
                        break;
                    case 4:
                        ret = VWStraight(DIST,SPEED);

                        LCDSetPrintf(8,1,"Attempted VWStraight, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        LCDSetPrintf(10,1,"should've driven %d mm at %d mm/s",DIST,SPEED);
                        break;
                    case 5:
                        ret = VWTurn(ANG,ASPEED);
                        LCDSetPrintf(8,1,"Attempted VWTurn, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        LCDSetPrintf(10,1,"should've rotated %d degrees at %d mm/s",ANG,ASPEED);
                        break;
                    case 6:
                        ret = VWCurve(DIST,180,ASPEED);
                        LCDSetPrintf(8,1,"Attempted VWCurve, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        LCDSetPrintf(10,1,"should've curved 180 degrees over %d mm at %d mm/s",DIST,SPEED);
                        break;
                    case 7:
                        ret = VWDrive(50,0,SPEED);
                        LCDSetPrintf(8,1,"Attempted VWDrive to (50,0), which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        ret = VWDrive(50,50,SPEED);
                        LCDSetPrintf(11,1,"Attempted VWDrive to (100,50), which returned %d",ret);
                        LCDSetPrintf(12,1,"(0 = success)");

                        ret = VWDrive(0,50,SPEED);
                        LCDSetPrintf(14,1,"Attempted VWDrive to (100,100), which returned %d",ret);
                        LCDSetPrintf(15,1,"(0 = success)");

                        
                        break;
                    case 8:
                        int remain = 0;
                        ret = Drive(500,0,SPEED);
                        int line = 8;
                        for (line; line < 20 ; line++) {
                            remain = VWDriveRemain();
                            LCDSetPrintf(line,1,"VWDriveRemain returned %d",remain);
                            OSWait(100);
                            
                        }


                        break;
                    case 9:
                        LCDSetPrintf(8,1,"Testing VWDriveWait, I shouldn't write anyhthing until I stop");
                        ret = VWDrive(500,50,SPEED);
                        ret = VWDriveWait;
                        LCDSetPrintf(11,1,"Attempted VWDriveWait to (500,0), which returned %d",ret);
                        LCDSetPrintf(12,1,"(0 = success)");
                        break;
                    case 10:
                        LCDSetPrintf(8,1,"Testing VWStalled; Stall a motor");
                        LCDSetPrintf(9,1,"0 if none, 3 if both. L = 1, R = 2");
                        ret = VWDrive(500,0,100);
                        int line = 9;
                        for (line; line < 20 ; line++) {
                            ret = VWStalled();
                            LCDSetPrintf(line,1,"VWStalled returned %d",ret);
                            OSWait(100);
                            
                        }

                        break;
                    case 11:
                        exit(0);
                    default:
                        //uh oh
                        //

                


                }

                OSWait(2000);       //wait 2s to display return message
                break;

            
            case KEY3: 
                i++;
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);
        }




    }while (k!=4);
    

}                   
