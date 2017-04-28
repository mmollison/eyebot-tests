/**USB/Serial
 * This is a program to test each Eyebot 7 API function related to USB
 * Serial Comms The API describes them as:
 *      int  SERInit(int interface, int baud,int handshake);
 *          Initiate connection (see parameters below), interface number as in HDT file
 *
 *          how does int handshake work?
 *
 *      int SERSendChar(int interface, char ch);
 *          send single character 
int SERSend(int interface, char *buf); 
 *          send string (Null terminated)
char SERReceiveChar(int interface);
 *          Receive single character 
 *      int  SERReceive(int interface, char *buf);
 *          Receive String (Null terminated), function returns number of chars received
bool SERCheck(int interface);
 *          Non-blocking check if character is waiting 
 int  SERFlush(int interface)
 *          Flush interface buffers 
 int  SERClose(int interface); 
 *          Close Interface
 *      Communication Parameters: Baudrate: 50 .. 230400
 *      Handshake: NONE, RTSCTS Interface: 0 (serial port), 1..20 (USB devices,
 *      names are assigned via HDT entries) USB 1 - Eyebot   USB 2 - GPS


*/

#include "eyebot.h"
#include "types.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

char* in;
static bool flushed;
static int bd[] = {50,75,110,134,150,200,300,600,1200,1800,2400,4800,9600,19200,38400,57600,76800,115200};
static int linenumber,k,interface,baud,shake,init;
static char ch;
static char sel_char(char ch);
static void sel_buff(void);
static int baud_select(int baud);
static int int_select(void);


static int baud_select(int baud){
    int ret = baud;
    LCDClear();
    LCDSetPrintf(0, 0, "Welcome to the Speed Selecter");
    LCDSetPrintf(2, 1, "Please select the speed you are testing");

    LCDMenu("PREV", "SELECT", "NEXT", "EXIT");
    LCDSetPrintf(3, 1, "Use PREV & NEXT to select a speed and SELECT to confirm");
    int i = 0 ; // int to traverse bd array
    k=-1;
    do {
        k=-1;
        k = KEYRead();
        LCDSetPrintf(5, 2, "Chosen Speed: %d                     ", ret);

        switch (k) {
            case KEY1:
                if (ret < 51)
                    i = 17;
                else
                    --i;
                break;
            case KEY2:
               return ret;
            case KEY3:
                if (i == 17) 
                    i = 0;
                else
                    i++;
                break;

            case KEY4:
                LCDSetPrintf(6, 0, "Returning");
                OSWait(500);
                k = KEY4;
                break;
        }
        ret = bd[i];
    } while (k != KEY4);

    return ret;
}


static char sel_char(char ch) {
    char ret = ch;
    k=-1;
    while(1){
        k=-1;
        LCDClear();
        LCDSetPrintf(0, 0, "Welcome to the Character Selecter");
        LCDSetPrintf(2, 1, "Please select the character you wish to send");
        LCDSetPrintf(4, 2, "Current Selewindoction:  %c",ret);
        LCDMenu("PREV","CONFIRM","NEXT","EXIT");
        k = KEYGet();
        switch (k) {
            case KEY1:
                if (ret == 0)
                    ret = 255;
                else
                    --ret;
                break;
            case KEY2:
               return ret;
            case KEY3:
                if (ret==255) 
                    ret = 0;
                else
                    ret++;
                break;

            case KEY4:
                LCDSetPrintf(6, 0, "Exiting");
                OSWait(500);
                exit(0);
        }
    } while (k != KEY2);

    return ret;
}


static void sel_buff() {
    int lines_allocated = 128;
    int max_line_len = 100;

    /* Allocate lines of text */
    char **lines = (char **)malloc(sizeof(char*)*lines_allocated);
    if (lines==NULL)
        {
        fprintf(stderr,"Out of memory (1).\n");
        exit(1);
        }
    errno = 0;
    
    int i;      //number of lines actually assigned
    FILE *fp = fopen("/home/pi/eyebot/demo/TEST.dir/garbage.txt", "r");
    //sorry   
    if (fp == NULL)
        {
        fprintf(stderr,"Error opening file %d.\n",errno);
        exit(2);
        }
    else {};

    for (i=0;1;i++)
        {
        int j;

        /* Have we gone over our line allocation? */
        if (i >= lines_allocated)
            {
            int new_size;

            /* Double our allocation and re-allocate */
            new_size = lines_allocated*2;
            lines = (char **)realloc(lines,sizeof(char*)*new_size);
            if (lines==NULL)
                {
                fprintf(stderr,"Out of memory.\n");
                exit(3);
                }
            lines_allocated = new_size;
            }
        /* Allocate space for the next line */
        lines[i] = malloc(max_line_len);
        if (lines[i]==NULL)
            {
            fprintf(stderr,"Out of memory (3).\n");
            exit(4);
            }
        if (fgets(lines[i],max_line_len-1,fp)==NULL)
            break;

        /* Get rid of CR or LF at end of line */
        for (j=strlen(lines[i])-1;j>=0 && (lines[i][j]=='\n' || lines[i][j]=='\r');j--)
            ;
        lines[i][j+1]='\0';
        }
    /* Close file */
    fclose(fp);
 
    if (linenumber == -1){
        linenumber = 0;
    }
    
    
    k = -1;
    do{
        k = -1;
        LCDClear();
        LCDSetPrintf(0, 0, "Welcome to the buffer selecter");
        LCDSetPrintf(2, 1, "Please select the string you wish to send");
        LCDSetPrintf(4, 2, "Current Selection:");
        LCDSetPrintf(6,0,lines[linenumber]);
        LCDMenu("PREV","CONFIRM","NEXT","EXIT");
        k = KEYGet();
        fprintf(stdout,"%s\n",lines[linenumber]);
        switch (k) {
            case KEY1:
                if (linenumber==0) {
                    linenumber=i-1;
                }
                else linenumber--;
                break;
            case KEY2:
                in = lines[linenumber];
                return;
            case KEY3:
                if (linenumber==i-1) {
                    linenumber = 0;
                }
                else linenumber++;
                break;
            case KEY4:
                LCDSetPrintf(6, 0, "Exiting");
                OSWait(500);
                exit(0);
        }
    } while (k != KEY2);

    return;
}


int int_select() {
    LCDClear();
    LCDSetPrintf(0, 0, "Welcome to the Interface Selecter");
    LCDSetPrintf(2, 1, "Please select the interface you are testing");
    LCDMenu("RS232","USB1","USB2","EXIT");
    k=-1;
    while(1){
        k = KEYGet();
        switch (k) {
            case KEY1:
                return 0;
            case KEY2:
                return 1;      
            case KEY3: 
                return 2;
            case KEY4: {
                LCDSetPrintf(7, 0, "Exiting");
                OSWait(1000);
                exit(0);
            }
        }
    }
    return -1;
}


int main() {
start:
    k=-1;
    interface=-1;
    do {
        LCDClear();
        LCDSetPrintf(0, 0, "Welcome to the Serial Comms Function Tester");
        LCDSetPrintf(2, 1, "Please select the interface you are testing");
        switch (interface) {
            case -1:
                LCDSetPrintf(4,2, "Press change");
                break;
            case 0:
                LCDSetPrintf(4,2,"RS232 Selected");
                break;
            case 1:
                LCDSetPrintf(4,2, "USB1 (Eyebot) Selected");
                break;
            case 2:
                LCDSetPrintf(4,2, "USB2 (GPS) Selected");
                break;
        }   
        k = -1;
        LCDMenu("CHANGE","CONTINUE","","EXIT");
        k = KEYGet();
        switch (k) {

            case KEY1:
                interface=int_select();
                k=-1;
                break;
            case KEY2: 
                if (interface == -1){
                    LCDSetPrintf(6, 3, "Cannot continue until interface is chosen!");
                    OSWait(1000);
                    break;
                }
                else
                    break;
            case KEY3: 
                LCDSetPrintf(6, 3, "Not an option");
                OSWait(1000);
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);
        }


    } while (k!=KEY2 || interface == -1);

    k = -1;
    baud = -1;
    do { -
        LCDClear();
        LCDSetPrintf(0, 0, "Welcome to the Serial Comms Function Tester");
        LCDSetPrintf(2, 1, "Please select the speed you are testing");

        if (baud == -1)
            LCDSetPrintf(4,2, "No speed chosen, press change");
        else
            LCDSetPrintf(4,2, "Selected baud: %d",baud);
        k = -1;
        LCDMenu("CHANGE","CONTINUE","","EXIT");
        k = KEYGet();
        switch (k) {
            case KEY1:
                baud=baud_select(baud);
                k=-1;
                break;
            case KEY2: 
                if (baud == -1){
                    LCDSetPrintf(6, 3, "Cannot continue until baud is chosen!");
                    OSWait(1000);
                    break;
                }
                else
                    break;
            case KEY3: 
                LCDSetPrintf(6, 3, "Not an option");
                OSWait(1000);
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0); LCDClear();

        }
    }while (k!=KEY2 || baud == -1);


    k = -1;
    shake = -1;
    do {
        LCDClear();
        LCDSetPrintf(0, 0, "Welcome to the Serial Comms Function Tester");
        LCDSetPrintf(2, 1, "Please select the handshake method");

        if (shake == -1)
            LCDSetPrintf(4,2, "No method selected, press something");
        else if (shake == 0)
            LCDSetPrintf(4,2, "Handshake method: None");
        else
            LCDSetPrintf(4,2, "Handshake method: RTSCTS");
        k = -1;
        LCDMenu("NONE","RTSCTS","CONTINUE","EXIT");
        k = KEYGet();
        switch (k) {
            case KEY1:
                shake = NONE; //not int :s... #define?
                break;
            case KEY2:
                shake = RTSCTS;                    
                break;
            case KEY3: 
                if (shake == -1){
                    LCDSetPrintf(6, 3, "Cannot continue until handshake method is chosen!");
                    OSWait(1000);
                    break;
                }
                else
                    break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0); 

        }
    }while (k!=KEY3 || shake == -1);


    k = -1;
    init = -1;
    do {
        LCDClear();
        LCDSetPrintf(0, 0, "Welcome to the Serial Comms Function Tester");
        LCDSetPrintf(2, 1, "All parameters selected, test SERInit");
        LCDSetPrintf(3, 1, "Current parameters are:");
        LCDSetPrintf(4, 2, "SERInit:     %d,  -1:not tried, 1: failed, 0:WINNING",init);
        LCDSetPrintf(5, 2, "Interface:   ");
        LCDSetPrintf(6, 2, "Speed:       %d",baud);
        LCDSetPrintf(7, 2, "Handshake:   ");
        if (shake == 0)
            LCDSetPrintf(7,13, "None");
        else
            LCDSetPrintf(7,13, "RTSCTS");
        if (interface == 0)
            LCDSetPrintf(5,13, "RS232");
        else if (interface==1)
            LCDSetPrintf(5,13, "USB1");
        else
            LCDSetPrintf(5,13,"USB2");

        if (init == 0){
            LCDSetPrintf(9,3, "Successfully initiated Serial connection");
        }

        LCDSetPrintf(8, 1, "Continue to initiate connection?");
        k=-1;
        LCDMenu("SERInit","RESTART","CONTINUE","EXIT");
        k = KEYGet();
        switch (k) {
            case KEY1:
                init = SERInit(interface, baud, shake);
                k=-1;
                break;
            case KEY2:
                k=-1;
                goto start;                   
                break;
            case KEY3: 
                if (init != 0){
                    LCDSetPrintf(9, 3, "Cannot continue until SERInit succeeds");
                    OSWait(1000);
                    k=-1;
                    break;
                }
                else
                    break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);

        }
    }while (k!=KEY3);
    
      
    k = -1;
    init = -1;
    bool waiting =  false;
    bool noCheck = false;
    char received = 'a';
    int checked = 0;
    do {
        LCDClear();
        waiting = SERCheck(interface);
        LCDSetPrintf(0, 0, "Welcome to the Serial Comms Function Tester");
        LCDSetPrintf(2, 1, "This function tests the Check and ReceiveChar functions on the chosen interface");
        LCDSetPrintf(3, 1, "Please send data via the chosen interface, and SERCheck should return true,");
        LCDSetPrintf(4, 1, "indicating a character is waiting to be read. CONTINUE to next test.");
        LCDSetPrintf(5, 1, "If SERCheck, press RECEIVE to display the next waiting character.");
        LCDSetPrintf(6, 1, "If !SERCheck, but you want to test SERReceiveChar anyway, do(n't) touch ME");
        LCDSetPrintf(8, 2, "SERCheck: %s", waiting ? "True" : "False");
        LCDSetPrintf(9, 3, "(True = character waiting, False = buffer is empty");
        if (checked == 1)
            LCDSetPrintf(11,1,"SERReceiveChar seems to have worked and received %c",received);
        else 
            LCDSetPrintf(11,1,"Waiting for SERReceiveChar to think it succeeded");
        k=-1;
        if (waiting || noCheck)
            LCDMenu("RECEIVE","","CONTINUE","EXIT");
        else{
            LCDMenu("","ME","CONTINUE","EXIT");
        }
        k = KEYGet();
        switch (k) {
            case KEY1:
                received = SERReceiveChar(interface);
                checked = 1;
                k=-1;
                break;
            case KEY2:
                noCheck = true;
                k=-1;
                break;
            case KEY3: 
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);

        }
    }while (k!=KEY3);


    k = -1;
    init = -1; //length of received string for this function
    waiting =  NULL;
    noCheck = false;
    char* receive = "nothing yet\n";
    checked = -1;
    do {
        LCDClear();
        waiting = SERCheck(interface);
        LCDSetPrintf(0, 0, "Welcome to the Serial Comms Function Tester");
        LCDSetPrintf(2, 1, "This function tests the Check and Receive functions on the chosen interface");
        LCDSetPrintf(3, 1, "Please send data via the chosen interface, and SERCheck should return true,");
        LCDSetPrintf(4, 1, "indicating a character is waiting to be read. CONTINUE to next test.");
        LCDSetPrintf(5, 1, "If SERCheck, press RECEIVE to display the NULL TERMINATED string.");
        LCDSetPrintf(6, 1, "If !SERCheck, but you want to test SERReceive anyway, don't touch ME");
        LCDSetPrintf(8, 2, "SERCheck: %s", waiting ? "True" : "False");
        LCDSetPrintf(9, 3, "(True = character waiting, False = buffer is empty");
        if (checked == 1){
            LCDSetPrintf(11,1,"SERReceive thinks you sent %d characters:", init);
            LCDSetPrintf(12,2,"%s",receive);
        }
        else 
            LCDSetPrintf(11,1,"Waiting for SERReceive to think it succeeded");
        k=-1;
        if (waiting || noCheck)
            LCDMenu("RECEIVE","","CONTINUE","EXIT");
        else{
            LCDMenu("","ME","CONTINUE","EXIT");
        }
        k = KEYGet();
        switch (k) {
            case KEY1:
                init = SERReceive(interface, receive);
                checked = 1;
                k=-1;
                break;
            case KEY2:
                noCheck = true;
                k=-1;
                break;
            case KEY3: 
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);

        }
    }while (k!=KEY3);


    k = -1;
    ch= '0';
    int sent = -1;
    do {
        LCDClear();
        LCDSetPrintf(0, 0, "Welcome to the Serial Comms Function Tester");
        LCDSetPrintf(2, 1, "Seial Connection established, character sending test");
        LCDSetPrintf(3, 1, "SELECT to select and send a character");
        LCDSetPrintf(4, 2, "Current character:     %c",ch);
        LCDSetPrintf(5, 1, "FLUSH to flush the serial buffer");
        LCDSetPrintf(6, 1, "CONTINUE to proceed to the next test");
        if (sent == -1)
            LCDSetPrintf(7,3, "Nothing has been sent yet");
        else if (sent = 0)
            LCDSetPrintf(7,3, "Successfully (on my end) sent character %c",ch);
        else 
            LCDSetPrintf(7,3, "Attempt to send %c failed",ch);
        k=-1;
        LCDMenu("SELECT","FLUSH","CONTINUE","EXIT");
        k = KEYGet();
        switch (k) {
            case KEY1:
                ch = sel_char(ch);
                sent = SERSendChar(interface, ch);
                k=-1;
                break;
            case KEY2:
                SERFlush(interface);
                k=-1;
                break;
            case KEY3: 
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);

        }
    }while (k!=KEY3);

    interface = 0;
    baud =115200;

    k = -1;
    linenumber = -1;
    sent = -1;
    do {
        LCDClear();
        LCDSetPrintf(0, 0, "Welcome to the Serial Comms Function Tester");
        LCDSetPrintf(2, 1, "Seial Connection established, buffer sending test");
        LCDSetPrintf(3, 1, "SELECT: select and send a string buffer");
        LCDSetPrintf(4, 1, "FLUSH to flush the serial buffer");
        LCDSetPrintf(5, 1, "CONTINUE to proceed to the next test");
        if (sent == -1)
            LCDSetPrintf(8,3, "Nothing has been sent yet");
        else if (sent = 0){
            LCDSetPrintf(8,3, "Successfully (on my end) sent:");
            LCDSetPrintf(9,4,in);
        }         
        else {
            LCDSetPrintf(8,3, "Failed to send buffer:");
            LCDSetPrintf(9,4,in);
        }
        k=-1;
        LCDMenu("SELECT","FLUSH","CONTINUE","EXIT");
        k = KEYGet();
        switch (k) {
            case KEY1:
                sel_buff();
                sent = SERSend(interface, in);
                k=-1;
                break;
            case KEY2:
                SERFlush(interface);
                k=-1;
                break;
            case KEY3: 
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);

        }
    }while (k!=KEY3);
    
    goto start;

    return 0;
}

