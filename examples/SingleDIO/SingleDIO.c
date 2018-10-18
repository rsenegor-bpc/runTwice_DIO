/******************************************************************/
/*  PD2-DIO-128I/MT test                                          */
/*  R. Senegor, October 2018                                      */
/*  BluePoint Controls                                            */
/*                                                                */
/*  EXTERNAL LOOPBACK FIXTURE REQUIRED FOR SUCCESS                */
/*                                                                */
/*  This file will allow the user to individually test            */
/*  UEI PD2-DIO-128I/MT cards to verify DIO operation.            */
/*                                                                */
/*  It does so by addressing and exciting each DIO port           */
/*  via nested loops. During operation all LEDs should            */
/*  incrementally illuminate then turn off.                       */
/*                                                                */
/*  Software adapted from United Electronic Industries, Inc. test */
/******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include "win_sdk_types.h"
#include "powerdaq.h"
#include "powerdaq32.h"

#include "ParseParams.h"


typedef enum _state
{
   closed,
   unconfigured,
   configured,
   running
} tState;

typedef struct _singleDioData
{
   int board;                    // board number to be used for the AI operation
   int handle;                   // board handle
   int nbOfPorts;                // nb of ports of 16 lines to use
   unsigned char OutPorts;       // mask for ports of 16 lines to use for output
   int nbOfSamplesPerPort;       // number of samples per port
   unsigned long portList[64];
   double scanRate;              // sampling frequency on each port
   tState state;                 // state of the acquisition session
} tSingleDioData;

FILE *f = NULL;

int InitSingleDIO(tSingleDioData *pDioData);
int SingleDIO(tSingleDioData *pDioData);
void CleanUpSingleDIO(tSingleDioData *pDioData);


static tSingleDioData G_DioData;

// exit handler
void SingleDIOExitHandler(int status, void *arg)
{
   CleanUpSingleDIO((tSingleDioData *)arg);
}


int InitSingleDIO(tSingleDioData *pDioData)
{
   Adapter_Info adaptInfo;
   int retVal = 0;

   // get adapter type
   retVal = _PdGetAdapterInfo(pDioData->board, &adaptInfo);
   if (retVal < 0)
   {
      fprintf(f,"SingleDIO: _PdGetAdapterInfo error %d\n", retVal);
      system ("dialog --title 'ERROR: DIO Test' --msgbox 'No adapter detected error!\n\nPress enter to continue.' 10 25");
      system("clear");
      exit(EXIT_FAILURE);
   }

   if(adaptInfo.atType & atPD2DIO) {
      fprintf(f,"This is a PD2-DIO board\n");
   }
   else
   {
      fprintf(f,"This board is not a PD2-DIO\n");
      system ("dialog --title 'ERROR: DIO Test' --msgbox 'This is not a PD2-DIO adapter error!\n\nPress enter to continue.' 10 25");
      system("clear");
      exit(EXIT_FAILURE);
   }

   pDioData->handle = PdAcquireSubsystem(pDioData->board, DigitalIn, 1);
   if(pDioData->handle < 0)
   {
      fprintf(f,"SingleDIO: PdAcquireSubsystem failed\n");
      system ("dialog --title 'ERROR: DIO Test' --msgbox 'Can't recognize adapter error!\n\nPress enter to continue.' 10 25");
      system("clear");
      exit(EXIT_FAILURE);
   }

   pDioData->state = unconfigured;

   retVal = _PdDIOReset(pDioData->handle);
   if (retVal < 0)
   {
      fprintf(f,"SingleDIO: PdDInReset error %d\n", retVal);
      system ("dialog --title 'ERROR: DIO Test' --msgbox 'Adapter reset error!\n\nPress enter to continue.' 10 25");
      system("clear");
      exit(EXIT_FAILURE);
   }

   return 0;
}


int SingleDIO(tSingleDioData *pDioData)
{
   int retVal;
   int i, j, failFlag=0;
   DWORD readVal, writeVal;
   int count = 0;


   /*Check successful log file creation*/
   if (f == NULL) {
        system ("dialog --title 'ERROR: DIO Test' --msgbox 'Error opening log file!\n\nNo log will be generated.\n\nPress enter to continue.' 10 25");
        system("clear");
        exit(1);
   }

   time_t t = time(NULL);
   struct tm *tm = localtime(&t);

   fprintf(f, "PD2-DIO loopback test\n");
   fprintf(f, "%s\n\n", asctime(tm));


   system ("dialog --title 'PD2-DIO-128I/MT Test' --infobox 'Running DIO test\n\nLEDs will flash in sequence...\n\nTest will take approx. 5 minutes' 10 25");

   /*Header on log file (including current time and date)*/
   
   
   retVal = _PdDIOEnableOutput(pDioData->handle, pDioData->OutPorts);
   if(retVal < 0)
   {
      fprintf(f,"SingleDIO: _PdDioEnableOutput failed\n");
      system ("dialog --title 'ERROR: DIO Test' --msgbox 'Output not enabled error!\n\nPress enter to continue.' 10 25");
      system("clear");
      exit(EXIT_FAILURE);
   }
   
   pDioData->state = configured;

   
   pDioData->state = running;

   //write 0 to all 4 DIO lines to make sure all DO ports are off (start at 0)

   for(j=0;j<pDioData->OutPorts;j++)
   {
      retVal = _PdDIOWrite(pDioData->handle, j, 0);
   }

   for(i=0; i<pDioData->nbOfSamplesPerPort; i++)
   {
      for(j=0; j<pDioData->OutPorts; j++)
      {
         if(count < pDioData->nbOfSamplesPerPort)
         {
            writeVal = count;
            retVal = _PdDIOWrite(pDioData->handle, j, writeVal);
            if(retVal < 0)
            {
               fprintf(f,"SingleDIO: _PdDIOWrite error: %d\n", retVal);
               system ("dialog --title 'ERROR: DIO Test' --msgbox 'Write error!\n\nPress enter to continue.' 10 25");
               system("clear");
               exit(EXIT_FAILURE);
            }
         }
         else
         {
            count=0;
         }
         usleep(3000);     //Microseconds between each IO stream set
         retVal = _PdDIORead(pDioData->handle, j, &readVal);
         if(retVal < 0)
         {
            fprintf(f,"SingleDIO: _PdDIORead error: %d\n", retVal);
            system ("dialog --title 'ERROR: DIO Test' --msgbox 'Read error!\n\nPress enter to continue.' 10 25");
            system("clear");
            exit(EXIT_FAILURE);
         }

     /*Normalize read/write values to remove extraneous bits*/
            readVal = 0xff0000 | readVal;
            writeVal = 0xff0000 | writeVal;

           if((readVal != writeVal))         
           {
            fprintf(f,"FAIL - Port %d, Value Wrote: 0x%x, Value Read: 0x%x\n", j, writeVal, readVal);
            failFlag = 1;
           }

      }

      count++;

      usleep(3.0E3/pDioData->scanRate);
   }

    if (failFlag == 0) 
    {
       fprintf(f,"\nPASSED ALL TESTS\n");
    }

   /*Turn all LEDs off after test*/

   system ("dialog --title 'PD2-DIO-128I/MT Test' --infobox 'Running DIO test\n\nPlease wait for LEDs to turn off...' 10 25");

   for(j=0;j<pDioData->OutPorts;j++)
   {
      retVal = _PdDIOWrite(pDioData->handle, j, 0);
   }

   return retVal;
}


void CleanUpSingleDIO(tSingleDioData *pDioData)
{
   int retVal;
      
   if(pDioData->state == running)
   {
      pDioData->state = configured;
   }

   if(pDioData->state == configured)
   {
      retVal = _PdDIOReset(pDioData->handle);
      if (retVal < 0)
         //fprintf(f,"SingleDIO: PdDInReset error %d\n", retVal);
         //system ("dialog --title 'ERROR: DIO Test' --msgbox 'Adapter reset error!\n\nPress enter to continue.' 10 25");
         //system("clear");

      pDioData->state = unconfigured;
   }

   if(pDioData->handle > 0 && pDioData->state == unconfigured)
   {
      retVal = PdAcquireSubsystem(pDioData->handle, DigitalIn, 0);
      if (retVal < 0)
         fprintf(f,"SingleDI: PdReleaseSubsystem error %d\n", retVal);
         system ("dialog --title 'ERROR: DIO Test' --msgbox 'Can't close adapter error!\n\nPress enter to continue.' 10 25");
         system("clear");
   }

   pDioData->state = closed;
}


int main(int argc, char *argv[])
{
   int i,k;
   FILE* fp;                        //Temp file for device number entry
   char* readFile = NULL, end;
   size_t len;
   long deviceNumber;

//Default parameters
//   PD_PARAMS params = {0, 4, {0,1,2,3}, 5.0, 0};
  

//   int statusStart = system ("dialog --title 'PD2-DIO-128I/MT Test' --yesno 'Enter DIO test?' 10 25");

   int statusStart = system ("dialog --title 'PD2-DIO-128I/MT Test' --nocancel --inputbox 'To begin, please enter the board number of your PD2-DIO device (0-4): ' 10 25 2> deviceInput.txt");

   fp = fopen("deviceInput.txt", "r");              //Temp file for device number entry

   getline(&readFile, &len, fp);  //Read user input from temp file

   fclose(fp);

   system("rm deviceInput.txt");               //Remove temp file

   deviceNumber = strtol(readFile, &end, 10);    //Convert device number entry to long

   PD_PARAMS params = {deviceNumber, 4, {0,1,2,3}, 5.0, 0};

   ParseParameters(argc, argv, &params);

if(!statusStart) {            //if valid entry to start test

   // initializes acquisition session parameters
   G_DioData.board = params.board;
   G_DioData.nbOfPorts = params.numChannels;
   for(i=0; i<params.numChannels; i++)
   {
       G_DioData.portList[i] = params.channels[i];
   }

   G_DioData.handle = 0;
   G_DioData.OutPorts = params.numChannels; // use ports 1 and 2 for output
   G_DioData.nbOfSamplesPerPort = 65536;
   G_DioData.scanRate = params.frequency;
   G_DioData.state = closed;

   // setup exit handler that will clean-up the acquisition session
   // if an error occurs
   on_exit(SingleDIOExitHandler, &G_DioData);

   f = fopen("/home/mtsnlinux/Desktop/DIO_test_logs/Log_DIO_Test.txt","w");

   InitSingleDIO(&G_DioData);

for(k=0;k<2;k++) {

   // run the acquisition
   SingleDIO(&G_DioData);

   // Cleanup acquisition
   CleanUpSingleDIO(&G_DioData);

   usleep(1000000); //!!!!!!!!!!!!!!!!!!!!

}

   fclose(f);

   system ("dialog --title 'DIO Test' --msgbox 'Test complete.\n\nLog generated: Log_DIO_Test.txt' 10 25");

}
 
   else if(statusStart!=0) {
       system ("dialog --title 'DIO Test' --msgbox 'Test not run.\n\nNo log generated.\n\nPress enter to quit.' 10 25");
}
   system("clear");
   return 0;
}

