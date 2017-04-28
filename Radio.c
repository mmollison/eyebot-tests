/**Radio Communication Interface
This is a program to test each Eyebot 7 API function related to the Radio Comms interface
The API describes them as:

These functions require a WiFi modules for each robot, one of them (or an external router) in DHCP mode, all others in slave mode.
Radio can be activated/deactivated via a HDT entry. The names of all participating nodes in a network can also be stored in the HDT file.
int RADIOInit(void);                            // Start radio communication
int RADIOGetID(void);                           // Get own radio ID
int RADIOSend(int id, BYTE* buf, int size);     // Send spec. number of bytes to ID destination
int RADIOReceive(int *id_no, BYTE* buf, int size); // Read num. of bytes from ID source
int RADIOCheck(int *id_no);                         // Non-blocking check whether message is waiting
int RADIOStatus(int IDlist[]);                  // Returns number of robots (incl. self) and list of IDs in network
int RADIORelease(void);                         // Terminate radio communication
ID numbers match last byte of robots' IP addresses.

*/


#include <math.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "eyebot.h"
#include <errno.h>



#define MAXSIZ      1028                  //max size of buffer to send
#define MINSIZ      1                  //miin 0 or 1?
#define BUF_SIZ     1028                //buffer size :s
//static int RADIOInit(void);         //0  main
//static int RADIOGetID(void);        //1  main
static int Select_ID(void);         //2  done
static int Select_Size(void);       //3  done - check max and min
static int Send(void);         //4 
//static int RADIOReceive(void);      //5
//static int RADIOCheck(void);        //6
static int Get_Status(void);       //7
/*
 *How does the array fill??
 *if 10 robots are connects with IPs 2, 4, 6, ..., 20
 *is the array {2,4,6,8,10,...,20, 0, 0, ...}
 *or {0,0,2,0,4,0,6,...}, what about IP == 0?
 *
 *And what if one of the 10 robots drops out?
 *Will it's ID be cleared from the array?
 *
 *I have assumed at this point the integer returned 
 *is the number of bots on the network
 *
 */
//static int RADIORelease(void);      //8
int Football(void);              //9

static int k = -1; //keycode
static int ID = -1;
static int ID_Send = -1; //ID to send to
static int size = -1;


//initiate to something else?
int ID_List[256] = {0};
int numofbots = -1;
int ID_Rec = -1; //ID to receive from set by sender?
static char* tests[]={"RADIOInit","RADIOGetID", "Select_ID to Send to", "Select_Size of BYTE buffer to send","RADIOSend","RADIOReceive", "RADIOCheck", "RADIOStatus", "RADIORelease","Like football, with numbers","exit"};
BYTE* buf_send;
BYTE* buf_rec;
static int Select_ID(void){
    //to send to

    ID = 0;                 
    
    do{
        if (ID==256){
            ID = 0;
        }
        if (ID==-1){
            ID=255;
        }
        LCDClear();
        LCDSetPrintf(0,0, "ID num to send buffer to");
        LCDSetPrintf(2,1, "Press MORE or LESS to adjust");
        LCDSetPrintf(3,1, "Press SET to confirm");
        LCDSetPrintf(5,2, "Selected ID: %d", ID);
        LCDMenu("LESS","SET","MORE","EXIT");
        k = -1;
        k = KEYGet();
        switch (k) {
            case KEY1:
                ID--;
                break;
            case KEY2:
                return ID;
            case KEY3:
                ID++;
                break;
            case KEY4:
                exit(0);
            default:
                break;
        }
    } while (k != KEY2);
                
    return ID;
}


static int Select_Size(void){
    //to send
    //max size of buf to send??
    //min size 0 or 1??

    size=MINSIZ;
    do{
        if (size==MAXSIZ+1){
            size = MINSIZ;
        }
        if (size==MINSIZ-1){
            size = MAXSIZ;
        }
        LCDClear();
        LCDSetPrintf(0,0, "Number of bytes to send");
        LCDSetPrintf(2,1, "Press MORE or LESS to adjust");
        LCDSetPrintf(3,1, "Press SET to confirm");
        LCDSetPrintf(5,2, "Selected size: %d", size);
        LCDMenu("LESS","SET","MORE","EXIT");
        k = -1;
        k = KEYGet();
        switch (k) {
            case KEY1:
                size--;
                break;
            case KEY2:
                return size;
            case KEY3:
                size++;
                break;
            case KEY4:
                exit(0);
            default:
                break;
        }
    } while (k != KEY2);
                
    return size;
} 

static int Send(void){

    if (size == -1 || ID_Send == -1)
    {
        LCDSetPrintf(3,0, "You need to Select_ID and Select_Size first");
        OSWait(3000);
        return 1;
    }
        

    buf_send = malloc(sizeof(BYTE)*MAXSIZ);
        //BYTE buffer "buf_send" is allocated memory for MAXSIZ bytes
        //does it need to be dynamically allocated?
        //is there a limit to the size of the buffer within the API?
        //for testing purposes should I limit it to (say) 1028?
        //
        //
        //
        //if dynamic allocating
    if (buf_send==NULL)
        {
        fprintf(stderr,"Out of memory (1).\n");
        exit(1);
        }
    errno = 0;
    
    int i;      //as in the serial function, I am using a text file 
                //for the source of the buffer
    FILE* fp = fopen("/home/pi/eyebot/demo/TEST.dir/garbage.txt", "r");
    //sorry   
    if (fp == NULL)
        {
        fprintf(stderr,"Error opening file %d.\n",errno);
        exit(2);
        }
    else {};
    int num_bytes = 0;

    //fill the buffer provided not at EOF and not whitespace/newline etc.
    while (!feof(fp) && num_bytes<size)
    {
        char c = fgetc(fp);
        if (!isspace(c))
            buf_send[num_bytes++] = c;
    }
    
    //if file is shorter than buffer, max size to send is num of bytes read.
    if (feof(fp))
    {
        size = num_bytes+1;
    }

    /* Close file */
    fclose(fp);
    int ret = -1;
    do{
        LCDClear();
        LCDSetPrintf(0,0, "RADIOSend Test");
        
        
        LCDSetPrintf(2,1, "Ready to send %d bytes",size);
        LCDSetPrintf(3,1, "to device, IP ending in %d",ID_Send);
        LCDSetPrintf(5,2, "Bytes to be send:");
        LCDSetPrintf(6,1, "%s",buf_send);
        LCDMenu("SEND","RETURN","TO MENU","EXIT");
        if (k == KEY1)
        {
            LCDSetPrintf(8,1,"Attempted/ing to send, returned %d (0 = success)", ret);
        }

        k = -1;
        k = KEYGet();
        switch (k) {

            case KEY1:
                ret = RADIOSend(ID_Send,buf_send,size);
                break;
            case KEY2: 
                return ret;
                break;
            case KEY3: 
                return ret;
                break;
            case KEY4: 
                LCDSetPrintf(7, 3, "Exiting");
                OSWait(500);
                exit(0);
        }

    }while (k!=1);
    return ret;
}


static int Get_Status(void)
{

    numofbots= RADIOStatus(ID_List) ;
    LCDClear();

    LCDSetPrintf(2,1,"Attempted RADIOStatus, which thinks ");
    if (numofbots<1){
        LCDSetPrintf(3,1,"0 bots are on the network. Something is wrong");
        OSWait(3000);
        return -1;
    }
    else
    {

        int linenum = 0;
        LCDSetPrintf(3,1,"%d bots are on the network. They are:",numofbots);


        for (int i = 0; i < numofbots; i++) {
            LCDSetPos(4,0);
            LCDPrintf( "%d, ",ID_List[i]);
        }   

        OSWait(numofbots*1000);
        return numofbots;


    }


}


/** static int Football(void)
 *
 * the purpose of this function is to ensure check, status, send, and receive 
 * work across all the bots, in a quick easy manner.
 * assuming a number of bots are set up, all should run the 
 * "Football" program from the demos menu. 
 * They will all attain their individual ID number (last byte of IP),
 * as well as perform RADIOCheck, to know where to send to next.
 *
 * The robot with the lowest ID number will be the only one capable
 * of starting or ending the game, and determines how it will be
 * played.
 *
 * The other robots will not have menu options, and will display only
 *      their ID number, 
 *      the number of players,  
 *      their next target,
 *      their individual kick count,
 *      the current count.
 *
 * Once Start has been pressed, the captain (low ID) will pass (send)
 * the ball(count) to the next ID number in the array 
 * (re)filled by RADIOStatus.
 *
 * RADIOStatus will be re-performed prior to each kick, to ensure the 
 * number is not passed to any injured robots, and any new players can
 * join (hopefully...).
 *
 * After starting, the menu options will change to AUTO, PASS, FINISH,
 * ETERNITY.
 *
 * AUTO will use the integer set by the Select_Size function to
 * make the captain pass the number "size" times ("size" loops),
 * before waiting for a keypress
 *
 * PASS will make the captain wait for your next move, a single
 * loop, so to speak
 *
 * FINISH will send a command to all players to stop waiting for
 * more passes, and to return the total number of passes as 
 * each robot last saw the number.
 *
 * ETERNITY will maybe exit, maybe not. I doubt any of this will
 * work at all so why would this bit?
 *
 *
 *
 *
 * uint32_t ball is the number that will be passed around, incremented
 * by each player 0 to 4294967295, 4 bytes, but 10 characters...
 *
 * BYTE* kickme is char rep. of above number.
 *
 * uint32_t my_kicks is the number of kicks by each individual player
 *
 * int IDs[] is the array for RADIOStatus
 *
 * int numofplayers is the number of currently connected bots
 *
 * int myID is my ID
 *
 *
 *
 *
 *
 *
 */

int Football(void)
{
    if (ID==-1){
        RADIOInit();
    }
    int IDs[256] = {0};
    int myID = RADIOGetID();
    int numofplayers = RADIOStatus(IDs);
    int minID=256;
    for (int i = 0; i<256; i++){
        if (IDs[i]<minID && IDs[i]>0)
            minID= IDs[i];
    }

    bool cap = false;
    if (minID == myID){
        cap = true;
        LCDSetPrintf(0,1,"Welcome to my football with numbers game");
        LCDSetPrintf(1,1,"This is real football (soccer), meaning");
        LCDSetPrintf(2,1,"we pass the number to each other,");
        LCDSetPrintf(3,1,"incrementing it each time. Very little else.");
        LCDSetPrintf(4,1,"You are the captain, and must make the");
        LCDSetPrintf(5,1,"first pass (START), after which you may ");
        LCDSetPrintf(6,1,"AUTOmatically kick the number Select_Size times, ");
        LCDSetPrintf(7,1,"PASS the ball when it comes back, once, let other");
        LCDSetPrintf(8,1,"players know the game has FINISHed, or press");
        LCDSetPrintf(9,1,"ETERNITY for undefined behaviour");
        LCDSetPrintf(10,1,"Other players are dumb, they just kick the number");
        LCDMenu("START", "RETURN","TO MENU","EXIT");
        k=-1;
        k=KEYGet();


    }
    switch (k){
        case KEY1: // START



    }
}

int main (){
    int i = 0; //which function	
    int ret = -1; //return val
    do{
        if (i == -1)
            i = 10;
        if (i == 11)
            i = 0;

        LCDClear();
        LCDSetPrintf(0,0, "Radio Communication Function Tester");
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
                        ret = RADIOInit();
                        LCDSetPrintf(8,1,"Attempted RADIOInit, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        break;
                    case 1:
                        ID = RADIOGetID();
                        LCDSetPrintf(8,1,"Attempted RADIOGetID, which returned %d",ret);
                        LCDSetPrintf(9,1,"Function should return last byte of robot IP");
                        break;
                    case 2:
                        ret = Select_ID();
                        LCDSetPrintf(8,1,"Sending to %d",ret);
                        break;
                    case 3:
                        ret = Select_Size();
                        LCDSetPrintf(8,1,"Sending %d  Bytes",ret);
                        break;
                    case 4:
                        ret = Send();
                        LCDSetPrintf(8,1,"Attempted to RADIOSend, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        break;
                    case 5:
                        //BYTE* buf_rec = malloc(sizeof(BYTE)*MAXSIZ);??
                        //int bytes_received = RADIOReceive(&ID_Rec, buf_rec, MAXSIZ);
                        ret = RADIOReceive(&ID_Rec, buf_rec,&size);
                        LCDSetPrintf(8,1,"Attempted RADIOReceive, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        LCDSetPrintf(10,1,"Received %d BYTES:\n %s",size,buf_rec);
                        break;
                    case 6:
                        ret = RADIOCheck(&ID_Rec);
                        LCDSetPrintf(8,1,"Attempted RADIOCheck, which returned %d",ret);
                        LCDSetPrintf(9,1,"(1 = waiting?)");
                        LCDSetPrintf(10,1,"Character waiting from device ID %d",ID_Rec);
                        break;
                        
                    case 7:
                        ret = Get_Status();     //go to Status function to print array
                        LCDSetPrintf(8,1,"Attempted RADIOStatus, which returned %d",ret);
                        LCDSetPrintf(9,1,"(-1 = fail, else num of bots on network)");
                        break;
                    case 8:
                        ret = RADIORelease();
                        LCDSetPrintf(8,1,"Attempted RADIORelease, which returned %d",ret);
                        LCDSetPrintf(9,1,"(0 = success)");
                        break;
                    case 9:
                        ret=Football();
                        LCDSetPrintf(8,1,"If you are reading this, Ping Pong may have worked");
                        LCDSetPrintf(9,1,"I doubt it, but if so, %d passes were made", ret);
                        LCDSetPrintf(10,1,"It's more likely you exited and/or my code was terrible");
                        

                    default:
                        LCDSetPrintf(8,1,"I shouldn't happen, so I'm exiting");
                        exit(0);
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
