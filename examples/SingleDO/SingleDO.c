/*****************************************************************************/
/*                    Single digital output example                          */
/*                                                                           */
/*  This example shows how to use the powerdaq API to perform a single       */
/*  digital update. The update is performed in a software timed fashion      */
/*  that is appropriate for slow speed pattern generation (up to 500Hz).     */
/*  This example only works on PDx-MFx and PD2-AO boards.                    */
/*  For the PD2-DIO boards look at the SingleDIO example.                    */
/*                                                                           */
/*---------------------------------------------------------------------------*/
/*      Copyright (C) 2001 United Electronic Industries, Inc.                */
/*      All rights reserved.                                                 */
/*---------------------------------------------------------------------------*/
/*                                                                           */ 
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
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

typedef struct _singleDoData
{
   int board;                    // board number to be used for the AI operation
   int handle;                   // board handle
   int nbOfPorts;                // number of digital lines to use
   int nbOfSamplesPerPort;       // number of samples per port
   double updateRate;            // update frequency on each channel
   tState state;                 // state of the acquisition session
} tSingleDoData;


int InitSingleDO(tSingleDoData *pDoData);
int SingleDO(tSingleDoData *pDoData);
void CleanUpSingleDO(tSingleDoData *pDoData);


static tSingleDoData G_DoData;

// exit handler
void SingleDOExitHandler(int status, void *arg)
{
   CleanUpSingleDO((tSingleDoData *)arg);
}


int InitSingleDO(tSingleDoData *pDoData)
{
   int retVal = 0;

   pDoData->handle = PdAcquireSubsystem(pDoData->board, DigitalOut, 1);
   if(pDoData->handle < 0)
   {
      printf("SingleDO: PdAcquireSubsystem failed\n");
      exit(EXIT_FAILURE);
   }

   pDoData->state = unconfigured;

   retVal = _PdDOutReset(pDoData->handle);
   if (retVal < 0)
   {
      printf("SingleDO: PdDInReset error %d\n", retVal);
      exit(EXIT_FAILURE);
   }

   return 0;
}


int SingleDO(tSingleDoData *pDoData)
{
   int retVal;
   int i;
   DWORD value;
      
   pDoData->state = configured;
   
   pDoData->state = running;

   // Periodically turn on and off all the lines of the digital output port
   for(i=0; i<pDoData->nbOfSamplesPerPort; i++)
   {
      value = i % 255;
      
      retVal = _PdDOutWrite(pDoData->handle, 0xFFFF);
      if(retVal < 0)
      {
         printf("SingleDO: _PdDOutWrite error: %d\n", retVal);
         exit(EXIT_FAILURE);
      }

      usleep((1.0E6/pDoData->updateRate)/2);

      retVal = _PdDOutWrite(pDoData->handle, 0x0000);
      if(retVal < 0)
      {
         printf("SingleDO: _PdDOutWrite error: %d\n", retVal);
         exit(EXIT_FAILURE);
      }

      usleep((1.0E6/pDoData->updateRate)/2);

      printf(".");
      fflush(stdout);
   }
   printf("\n");

   return retVal;
}


void CleanUpSingleDO(tSingleDoData *pDoData)
{
   int retVal;
      
   if(pDoData->state == running)
   {
      pDoData->state = configured;
   }

   if(pDoData->state == configured)
   {
      retVal = _PdDOutReset(pDoData->handle);
      if (retVal < 0)
         printf("SingleDO: _PdDOutReset error %d\n", retVal);

      pDoData->state = unconfigured;
   }

   if(pDoData->handle > 0 && pDoData->state == unconfigured)
   {
      retVal = PdAcquireSubsystem(pDoData->handle, DigitalOut, 0);
      if (retVal < 0)
         printf("SingleDI: PdReleaseSubsystem error %d\n", retVal);
   }

   pDoData->state = closed;
}


int main(int argc, char *argv[])
{
   PD_PARAMS params = {0, 1, {0}, 100.0, 0, 100};
   
   ParseParameters(argc, argv, &params);

   // initializes acquisition session parameters
   G_DoData.board = params.board;;
   G_DoData.handle = 0;
   G_DoData.nbOfPorts = params.numChannels;
   
   G_DoData.nbOfSamplesPerPort = params.numSamplesPerChannel;;
   G_DoData.updateRate = params.frequency;
   G_DoData.state = closed;

   // setup exit handler that will clean-up the acquisition session
   // if an error occurs
   on_exit(SingleDOExitHandler, &G_DoData);

   // initializes acquisition session
   InitSingleDO(&G_DoData);

   // run the acquisition
   SingleDO(&G_DoData);

   // Cleanup acquisition
   CleanUpSingleDO(&G_DoData);

   return 0;
}

