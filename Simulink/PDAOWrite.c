/*
* File: PDAIRead.c
*
*
*
*   --- THIS FILE GENERATED BY S-FUNCTION BUILDER: 2.0 ---
*
*   This file is an S-function produced by the S-Function
*   Builder which only recognizes certain fields.  Changes made
*   outside these fields will be lost the next time the block is
*   used to load, edit, and resave this file. This file will be overwritten
*   by the S-function Builder block. If you want to edit this file by hand, 
*   you must change it only in the area defined as:  
*
*        %%%-SFUNWIZ_defines_Changes_BEGIN
*        #define NAME 'replacement text' 
*        %%% SFUNWIZ_defines_Changes_END
*
*   DO NOT change NAME--Change the 'replacement text' only.
*
*   For better compatibility with the Real-Time Workshop, the
*   "wrapper" S-function technique is used.  This is discussed
*   in the Real-Time Workshop User's Manual in the Chapter titled,
*   "Wrapper S-functions".
*
*  -------------------------------------------------------------------------
* | See matlabroot/simulink/src/sfuntmpl_doc.c for a more detailed template |
*  ------------------------------------------------------------------------- 
* Created: Wed Oct 12 12:15:11 2005
* 
*
*/
#include <math.h>
#include "win_sdk_types.h"
#if defined(_WIN32)
   #include "pwrdaq32.h"
   #include "pwrdaq.h"
#else
   #include <signal.h>
   #include "powerdaq.h"
   #include "powerdaq32.h"
#endif
#include "pdfw_def.h"

#if defined(_WIN32) && defined(MATLAB_MEX_FILE)  
void __stdcall Sleep(DWORD dwMilliseconds);
#endif

#if defined(_WIN32)
#define errorChk(functionCall) { BOOL bResult; DWORD error=0;\
                                 if(!(bResult=functionCall)) { \
	                                 sprintf(errorMsg, "Error %d at line %d in function call %s\n", error, __LINE__, #functionCall); \
	                                 ssSetErrorStatus(S, errorMsg); \
                                    return; \
                               }}
#else
#define errorChk(functionCall) { int error=0;\
                                 if((error=functionCall) < 0) { \
	                                 sprintf(errorMsg, "Error %d at line %d in function call %s\n", error, __LINE__, #functionCall); \
	                                 ssSetErrorStatus(S, errorMsg); \
                                    return; \
                               }}
#endif                               

typedef enum _state
{
   closed,
   unconfigured,
   configured,
   running
} tState;

typedef struct _singleAoData
{
   int board;                    /* board number to be used for the AI operation */
   int nbOfBoards;               /* number of boards installed */
   HANDLE handle;                /* board handle */
   HANDLE driver;
   int nbOfChannels;             /* number of channels */
   int nbOfSamplesPerChannel;    /* number of samples per channel */
   unsigned long channelList[64];
   unsigned long aoCfg;
   double scanRate;              /* sampling frequency on each channel */
   tState state;                 /* state of the acquisition session */
   int adapterType;
} tSingleAoData;


#define S_FUNCTION_NAME PDAOWrite
#define S_FUNCTION_LEVEL 2
/*<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<*/
/* %%%-SFUNWIZ_defines_Changes_BEGIN --- EDIT HERE TO _END */
/* Output Port  0 */
#define OUT_PORT_0_NAME      y0
#define OUTPUT_0_WIDTH       1
#define OUTPUT_DIMS_0_COL    1
#define OUTPUT_0_DTYPE       real_T
#define OUTPUT_0_COMPLEX     COMPLEX_NO
#define OUT_0_FRAME_BASED    FRAME_NO
#define OUT_0_DIMS           1-D

#define NUM_PARAMS              3 

#include "simstruc.h"
#define PD_BOARD_PARAM(S)     ssGetSFcnParam(S, 0)
#define PD_CHANNEL_LIST_PARAM(S) ssGetSFcnParam(S, 1)
#define PD_SAMPLE_TIME_PARAM(S)  ssGetSFcnParam(S, 2)

#define IS_PARAM_INT32(pVal) (mxIsNumeric(pVal) && !mxIsLogical(pVal) &&\
!mxIsEmpty(pVal) && !mxIsSparse(pVal) && !mxIsComplex(pVal) && mxIsInt32(pVal))

#define IS_PARAM_DOUBLE(pVal) (mxIsNumeric(pVal) && !mxIsLogical(pVal) &&\
!mxIsEmpty(pVal) && !mxIsSparse(pVal) && !mxIsComplex(pVal) && mxIsDouble(pVal))


/*====================*
* S-function methods *
*====================*/
#define MDL_CHECK_PARAMETERS
#if defined(MDL_CHECK_PARAMETERS) && defined(MATLAB_MEX_FILE)
/* Function: mdlCheckParameters =============================================
* Abstract:
*    Validate our parameters to verify they are okay.
*/
static void mdlCheckParameters(SimStruct *S)
{
#define PrmNumPos 46
   int paramIndex = 0;
   bool validParam = false;
   char paramVector[] ={'1','2','3'};
   static char parameterErrorMsg[] ="The data type and/or complexity of parameter    does not match the information "
      "specified in the S-function Builder dialog. For non-double parameters you will need to cast them using int8, int16,"
      "int32, uint8, uint16, uint32 or boolean."; 
   
   /* All parameters must match the S-function Builder Dialog */
   
   
   {
      const mxArray *pVal = PD_BOARD_PARAM(S);
      if (!IS_PARAM_DOUBLE(pVal)) 
      {
         validParam = true;
         paramIndex = 0;
         goto EXIT_POINT;
      }
   }
   
   {
      const mxArray *pVal = PD_CHANNEL_LIST_PARAM(S);
      if (!IS_PARAM_DOUBLE(pVal)) 
      {
         validParam = true;
         paramIndex = 1;
         goto EXIT_POINT;
      }
   }

   {
      const mxArray *pVal = PD_SAMPLE_TIME_PARAM(S);
      if (!IS_PARAM_DOUBLE(pVal)) 
      {
         validParam = true;
         paramIndex = 2;
         goto EXIT_POINT;
      }
   }
EXIT_POINT:
   if (validParam) 
   {
      parameterErrorMsg[PrmNumPos] = paramVector[paramIndex];
      ssSetErrorStatus(S,parameterErrorMsg);
   }
   return;
}
#endif /* MDL_CHECK_PARAMETERS */
/* Function: mdlInitializeSizes ===============================================
* Abstract:
*   Setup sizes of the various vectors.
*/
static void mdlInitializeSizes(SimStruct *S)
{
   int i;

   ssSetNumSFcnParams(S, NUM_PARAMS);  /* Number of expected parameters */
#if defined(MATLAB_MEX_FILE)
   if (ssGetNumSFcnParams(S) == ssGetSFcnParamsCount(S)) 
   {
      mdlCheckParameters(S);
      if (ssGetErrorStatus(S) != NULL) 
      {
         return;
      }
   } 
   else 
   {
      return; /* Parameter mismatch will be reported by Simulink */
   }
#endif
   
   ssSetNumContStates(S, 0);
   ssSetNumDiscStates(S, 0);
   
   if (!ssSetNumOutputPorts(S, 0)) 
      return;

   ssSetNumInputPorts(S, mxGetN(PD_CHANNEL_LIST_PARAM(S)));
   for (i=0;i<mxGetN(PD_CHANNEL_LIST_PARAM(S));i++) 
   {
      ssSetInputPortWidth(S, i, 1);
      ssSetInputPortDataType(S, i, SS_DOUBLE);
      ssSetInputPortComplexSignal(S, i, COMPLEX_NO);
      ssSetInputPortDirectFeedThrough(S, i, 1);
   }

   ssSetNumSampleTimes(S, 1);
   ssSetNumRWork(S, 0);
   ssSetNumIWork(S, 0);
   ssSetNumPWork(S, 1);
   ssSetNumModes(S, 0);
   ssSetNumNonsampledZCs(S, 0);
   
   /* Take care when specifying exception free code - see sfuntmpl_doc.c */
   ssSetOptions(S, (SS_OPTION_EXCEPTION_FREE_CODE |
      /*SS_OPTION_RUNTIME_EXCEPTION_FREE_CODE |*/
      SS_OPTION_USE_TLC_WITH_ACCELERATOR /*|
      SS_OPTION_WORKS_WITH_CODE_REUSE*/));
}

/* Function: mdlInitializeSampleTimes =========================================
* Abstract:
*    Specifiy  the sample time.
*/
static void mdlInitializeSampleTimes(SimStruct *S)
{
   ssSetSampleTime(S, 0, (time_T)mxGetPr(PD_SAMPLE_TIME_PARAM(S))[0]/*INHERITED_SAMPLE_TIME*/);
   ssSetOffsetTime(S, 0, 0.0);
}


#define MDL_START  /* Change to #undef to remove function */
#if defined(MDL_START) 
/* Function: mdlStart =======================================================
* Abstract:
*    This function is called once at start of model execution. If you
*    have states that should be initialized once, this is the place
*    to do it.
*/
static void mdlStart(SimStruct *S)
{
   Adapter_Info adaptInfo;
   static char errorMsg[256];
   tSingleAoData* pAoData;
   int i; 

   if(ssGetSimMode(S) != SS_SIMMODE_NORMAL)
   {
      return;
   }

   pAoData = (tSingleAoData*)mxCalloc(1, sizeof(tSingleAoData));
   if(!pAoData)
   {
      sprintf(errorMsg, "Error allocating memory\n");
      ssSetErrorStatus(S, errorMsg);
      return;
   }

   pAoData->board = (int)(mxGetPr(PD_BOARD_PARAM(S))[0]);
   pAoData->handle = 0;
   pAoData->scanRate = 1.0/(real_T)(mxGetPr(PD_SAMPLE_TIME_PARAM(S))[0]);
   pAoData->nbOfChannels = mxGetN(PD_CHANNEL_LIST_PARAM(S));
   pAoData->nbOfSamplesPerChannel = 1;
   pAoData->state = closed;
   for(i=0; i<pAoData->nbOfChannels; i++)
   {
      pAoData->channelList[i] = (int_T)mxGetPr(PD_CHANNEL_LIST_PARAM(S))[i];
   }

   pAoData->aoCfg = 0; 

#if defined(_WIN32)
   errorChk(PdDriverOpen(&pAoData->driver, &error, &pAoData->nbOfBoards));
   errorChk(_PdAdapterOpen(pAoData->board, &error, &pAoData->handle));   
   errorChk(PdAdapterAcquireSubsystem(pAoData->handle, &error, AnalogOut, 1));
#else  
   errorChk(_PdGetAdapterInfo(pAoData->board, &adaptInfo));   
   pAoData->adapterType = adaptInfo.atType;

   printf("adapter name is %s\n", adaptInfo.lpBoardName);

   pAoData->handle = PdAcquireSubsystem(pAoData->board, AnalogOut, 1);
   if(pAoData->handle < 0) 
   { 
      sprintf(errorMsg, "Error acquiring AnalogOut subsystem\n");
      ssSetErrorStatus(S, errorMsg);
      return;
   }
#endif

   if(!(pAoData->adapterType & atPD2AO) && !(pAoData->adapterType & atMF))
   {
      sprintf(errorMsg, "The specified board doesn't have any analog output channels\n");
	  ssSetErrorStatus(S, errorMsg);
      return;
   }

   if(pAoData->nbOfChannels > adaptInfo.SSI[AnalogOut].dwChannels)
   {
      sprintf(errorMsg, "The specified board doesn't have enough analog output channels\n");
	  ssSetErrorStatus(S, errorMsg);
      return;
   }

   pAoData->state = unconfigured;

#if defined(_WIN32)
   errorChk(_PdAOutReset(pAoData->handle, &error));   
   /* need also to call this function if the board is a PD2-AO-xx */
   if(pAoData->adapterType & atPD2AO)
   {
      errorChk(_PdAO32Reset(pAoData->handle, &error));
   }
#else      
   errorChk(_PdAOutReset(pAoData->handle));  
   /* need also to call this function if the board is a PD2-AO-xx */
   if(pAoData->adapterType & atPD2AO)
   {
      errorChk(_PdAO32Reset(pAoData->handle));
   }
#endif

   pAoData->state = configured;

#if defined(_WIN32)
   errorChk(_PdAOutSetCfg(pAoData->handle, &error, pAoData->aoCfg, 0));
   errorChk(_PdAOutSwStartTrig(pAoData->handle, &error));
#else
   errorChk(_PdAOutSetCfg(pAoData->handle, pAoData->aoCfg, 0));
   errorChk(_PdAOutSwStartTrig(pAoData->handle));
#endif

   pAoData->state = running;

      
   ssGetPWork(S)[0] = (void*)pAoData;
}
#endif /*  MDL_START */

#define MDL_SET_INPUT_PORT_DATA_TYPE
static void mdlSetInputPortDataType(SimStruct *S, int port, DTypeId dType)
{
   int i;

   for (i=0;i<mxGetN(PD_CHANNEL_LIST_PARAM(S));i++) 
   {
      ssSetInputPortDataType(S, i, dType);
   }
}

#define MDL_SET_DEFAULT_PORT_DATA_TYPES
static void mdlSetDefaultPortDataTypes(SimStruct *S)
{
   int i;

   for (i=0;i<mxGetN(PD_CHANNEL_LIST_PARAM(S));i++) 
   {
      ssSetInputPortDataType(S, i, SS_DOUBLE);
   }

}
/* Function: mdlOutputs =======================================================
*
*/
static void mdlOutputs(SimStruct *S, int_T tid)
{
   static char errorMsg[256];
   tSingleAoData* pAoData = (tSingleAoData*)ssGetPWork(S)[0];
   
   InputRealPtrsType uPtrs;
   unsigned int i, k;
   DWORD value;

   if(ssGetSimMode(S) != SS_SIMMODE_NORMAL)
   {
      return;
   }
                  
   for(i=0; i<pAoData->nbOfSamplesPerChannel; i++)
   {
      value = 0;
      
      /* write update value to the board */
      if(pAoData->adapterType & atMF)
      {
         for(k=0; k<pAoData->nbOfChannels; k++)
         {
            uPtrs=ssGetInputPortRealSignalPtrs(S,k);
            /* special case for PD2-MFx boards */
            if(k==0)
            {
               value = (*uPtrs[0] + 10.0) / 20.0 * 0xFFF;
            }
            else
            {
               value = value | ((int)((*uPtrs[0] + 10.0) / 20.0 * 0xFFF) << 12);
            }
         }
#if defined(_WIN32)
         errorChk(_PdAOutPutValue(pAoData->handle, &error, value));
#else
         /*printf("Writing %f volts (0x%x)\n", *uPtrs[0], value); */
         errorChk(_PdAOutPutValue(pAoData->handle, value));
#endif
      }
      else if (pAoData->adapterType & atPD2AO)
      {
         /* special case for PD2-AO boards */
         for(k=0; k<pAoData->nbOfChannels; k++)
         {
            uPtrs=ssGetInputPortRealSignalPtrs(S,k);
            value = (*uPtrs[0] + 10.0) / 20.0 * 0xFFFF;
#if defined(_WIN32)
            errorChk(_PdAO32Write(pAoData->handle, &error, pAoData->channelList[k], value));
#else
            errorChk(_PdAO32Write(pAoData->handle, pAoData->channelList[k], value));
#endif
         }
      }
   }
   
/* When running in simulation mode in simulink */
/* simulate real-time */
#if defined(MATLAB_MEX_FILE)
   #if defined(_WIN32) 
      Sleep((DWORD)floor(1.0E3/pAoData->scanRate));
   #else
      usleep((useconds_t)floor(1.0E6/pAoData->scanRate));
   #endif
#endif

}



/* Function: mdlTerminate =====================================================
* Abstract:
*    In this function, you should perform any actions that are necessary
*    at the termination of a simulation.  For example, if memory was
*    allocated in mdlStart, this is the place to free it.
*/
static void mdlTerminate(SimStruct *S)
{
   static char errorMsg[256];
   tSingleAoData* pAoData = (tSingleAoData*)ssGetPWork(S)[0];

   if(ssGetSimMode(S) != SS_SIMMODE_NORMAL)
   {
      return;
   }

   if(pAoData->state == running)
   {
      pAoData->state = configured;
   }
   
   if(pAoData->state == configured)
   {
#if defined(_WIN32)
      errorChk(_PdAOutReset(pAoData->handle, &error));   
      /* need also to call this function if the board is a PD2-AO-xx */
      if(pAoData->adapterType & atPD2AO)
      {
         errorChk(_PdAO32Reset(pAoData->handle, &error));
      }
#else      
      errorChk(_PdAOutReset(pAoData->handle));  
      /* need also to call this function if the board is a PD2-AO-xx */
      if(pAoData->adapterType & atPD2AO)
      {
         errorChk(_PdAO32Reset(pAoData->handle));
      }
#endif
      pAoData->state = unconfigured;
   }
   
   if(pAoData->handle > 0 && pAoData->state == unconfigured)
   {
#if defined(_WIN32)
      errorChk(PdAdapterAcquireSubsystem(pAoData->handle, &error, AnalogOut, 0));
      errorChk(_PdAdapterClose(pAoData->handle, &error));
      errorChk(PdDriverClose(pAoData->driver, &error));
#else
      errorChk(PdAcquireSubsystem(pAoData->handle, AnalogOut, 0));
#endif
   }
   
   pAoData->state = closed;

   mxFree(pAoData);
}

#ifdef  MATLAB_MEX_FILE    /* Is this file being compiled as a MEX-file? */
#include "simulink.c"      /* MEX-file interface mechanism */
#else
#include "cg_sfun.h"       /* Code generation registration function */
#endif


