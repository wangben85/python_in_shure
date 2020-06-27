#include "EmbeddedTypes.h"
#include "zigbee.h"
#include "BeeStack_Globals.h"
#include "BeeStackConfiguration.h"
#include "AppZdoInterface.h"
#include "TS_Interface.h"
#include "TMR_Interface.h"
#include "AppAfInterface.h"
#include "FunctionLib.h"
#include "EndPointConfig.h"
#include "BeeApp.h"
#include "ZDOStateMachineHandler.h"
#include "ZdoApsInterface.h"
#include "BeeAppInit.h"
#include "NVM_Interface.h"
#include "ZtcInterface.h"
#include "ZdpManager.h"
#include "ASL_ZdpInterface.h"
#include "ASL_UserInterface.h"
#include "ZdpFrequencyAgility.h"
#include "ZigbeeTask.h"
#define ledEventDisplay_c (1 << 0)  
#define ledClusterHighByte_c  0x11  
#define ledClusterData_c      0x01  
void BeeAppTask(tsEvent_t events);
void BeeAppDataIndication(void);
void BeeAppDataConfirm(void);
void BeeAppZdpCallBack(zdpToAppMessage_t *pMsg, zbCounter_t counter);
#if gInterPanCommunicationEnabled_c
  void BeeAppInterPanDataConfirm(void);
  void BeeAppInterPanDataIndication(void);
#endif 
void LedReportData(void);
void LedDisplayData(void);
void LedTimerCallBack ( zbTmrTimerID_t timerId );
void lederometerDisplayModeCallBack ( zbTmrTimerID_t timerId );
void GenericApp_LCDInitComplete(bool_t status);
#if(gInstantiableStackEnabled_d == 0)
bool_t           ledDemoValue = FALSE;           
zbNwkAddr_t      gaLedDstAddr;
zbEndPoint_t     gledDstEndPoint;
bool_t           gfLedFoundDst;
bool_t           gfLedIsDisplay;
uint8_t          appGenericState = mStateIdle_c;
uint8_t          comboDeviceStart=gZdoStartMode_Zc_c;
zbClusterId_t    appDataCluster;
#else
bool_t           ledDemoValueInst[2] = {FALSE,FALSE};            
zbNwkAddr_t      gaLedDstAddrInst[2];
zbEndPoint_t     gledDstEndPointInst[2];
bool_t           gfLedFoundDstInst[2];
bool_t           gfLedIsDisplayInst[2];
uint8_t          appGenericStateInst[2]  = {mStateIdle_c,mStateIdle_c};
uint8_t          comboDeviceStartInst[2] = {gZdoStartMode_Zc_c,gZdoStartMode_Zc_c};
zbClusterId_t    appDataClusterInst[2];
 #define ledDemoValue            ledDemoValueInst[zbProInstance]
#define gaLedDstAddr            gaLedDstAddrInst[zbProInstance]
#define gledDstEndPoint         gledDstEndPointInst[zbProInstance]
#define gfLedFoundDst           gfLedFoundDstInst[zbProInstance]
#define gfLedIsDisplay          gfLedIsDisplayInst[zbProInstance]
#define appGenericState         appGenericStateInst[zbProInstance]
#define comboDeviceStart        comboDeviceStartInst[zbProInstance]
#define appDataCluster          appDataClusterInst[zbProInstance]
#endif 
void BeeAppInit
  (
  void
  )
{
  index_t i;
  Zdp_AppRegisterCallBack(BeeAppZdpCallBack);
   #if(gInstantiableStackEnabled_d == 0) 
  LED_TurnOffAllLeds();
  LED_SetLed(LED1, gLedFlashing_c);
  #else
  if(zbProInstance == 0)
  {
    LED_TurnOffAllLeds();
    LED_SetLed(LED1, gLedFlashing_c);
  }
  else
  {
    LED_SetLed(LED3, gLedFlashing_c);
  }
  #endif
  LCD_WriteString(2, "lederometer");
  #if gInstantiableStackEnabled_d
    for(i=0; i<EndPointConfigData(gNum_EndPoints); ++i) {
  #else
    for(i=0; i<gNum_EndPoints_c; ++i) {  
  #endif
    (void)AF_RegisterEndPoint(EndPointConfigData(endPointList[i].pEndpointDesc));
  }
  BeeAppDataInit(appEndPoint) = EndPointConfigData(endPointList[0].pEndpointDesc->pSimpleDesc->endPoint);
  Copy2Bytes(appDataCluster, EndPointConfigData(endPointList[0].pEndpointDesc->pSimpleDesc->pAppInClusterList));
  #if(gInstantiableStackEnabled_d == 1)    
    ZbBeeStackGlobalsParams(gaEndPointDesc[ 0 ].pDescription) = (endPointDesc_t *)pEndPointData->endPoint0Desc;
    ZbBeeStackGlobalsParams(gaEndPointDesc[ 1 ].pDescription) = (endPointDesc_t *)pEndPointData->broadcastEndPointDesc;  
  #endif  
  FA_StateMachineInit();
}
void BeeAppTask
  (
  tsEvent_t events    
  )
{
  if(events & gAppEvtDataConfirm_c)
    BeeAppDataConfirm();
  if(events & gAppEvtDataIndication_c)
    BeeAppDataIndication();
  if(events & gAppEvtSyncReq_c)
    ASL_Nlme_Sync_req(FALSE);  
#if gInterPanCommunicationEnabled_c
  if(events & gInterPanAppEvtDataConfirm_c)
    BeeAppInterPanDataConfirm();
  if(events & gInterPanAppEvtDataIndication_c)
    BeeAppInterPanDataIndication();
#endif
    app specific events (bits 0-11) go here...
  if(events & ledEventDisplay_c) 
  {
    LedDisplayData();
  }
}
void BeeAppHandleKeys
  (
  key_event_t keyEvent  
  )
{
  switch(keyEvent) {
    case gKBD_EventSW1_c:
      if (appGenericState == mStateIdle_c) 
      {
        /* indicate looking for the network to form(ZC) or join(ZED, ZR) */
        #if(gInstantiableStackEnabled_d == 0) 
        LED_StartSerialFlash(LED1);
        #else
        if(zbProInstance == 0)
        {
          LED_SetLed(LED2, gLedFlashing_c);
        }
        else
        {
          LED_SetLed(LED4, gLedFlashing_c);
        }
        #endif
        ZDO_Start(comboDeviceStart|gStartWithOutNvm_c);
      }
      break;
    case gKBD_EventSW2_c:
      if(appGenericState == mStateIdle_c)
      {
        comboDeviceStart = gZdoStartMode_Zr_c;
        break;
      }
      if(ledDemoValue == TRUE)
      {
        ledDemoValue = FALSE;
      }
      else
      {
        ledDemoValue = TRUE;
      }
      LedReportData();
      TS_SendEvent(gAppTaskID, ledEventDisplay_c);
      break;
    case gKBD_EventLongSW1_c:
	ZDO_Leave();
        gfLedIsDisplay = FALSE;
        comboDeviceStart=gZdoStartMode_Zc_c;
        appGenericState = mStateIdle_c;
         #if(gInstantiableStackEnabled_d == 0) 
         LED_TurnOffAllLeds();
         LED_SetLed(LED1, gLedFlashing_c);
         #else
         if(zbProInstance == 0)
         {
            LED_SetLed(LED1, gLedFlashing_c);
            LED_SetLed(LED2, gLedOff_c);
         }
         else
         {
            LED_SetLed(LED3, gLedFlashing_c);
            LED_SetLed(LED4, gLedOff_c);
         }
         #endif
         break;
    /* Find led using match descriptor */
    case gKBD_EventLongSW2_c:
      if(!gfLedIsDisplay)
      ASL_MatchDescriptor_req( NULL, (uint8_t *) gaBroadcastRxOnIdle, AF_FindEndPointDesc(BeeAppDataInit(appEndPoint)) );
      break;
   
     default:
       break;
  }
}
/****************************************************************************
* BeeAppDataIndication
*
* Process incoming ZigBee over-the-air messages.
*****************************************************************************/
void BeeAppDataIndication
  (
  void
  )
{
  apsdeToAfMessage_t *pMsg;
  zbApsdeDataIndication_t *pIndication;
  while(MSG_Pending(&BeeAppDataInit(gAppDataIndicationQueue)))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &BeeAppDataInit(gAppDataIndicationQueue) );
    /* ask ZCL to handle the frame */
    pIndication = &(pMsg->msgData.dataIndication);
    /*
      Note: if multiple endpoints are supported by this app, insert 
      endpoint filtering here...
      This app assumes only 1 active endpoint. APS layer has already
      filtered by endpoint and profile.
      Note: all multi-byte over-the-air fields are little endian.
      That is 0x1101 would come in byte order 0x01 0x11.
    */
    /* is the cluster for lederometer? */
    if(pIndication->aClusterId[1] != appDataCluster[1]) {
      AF_FreeDataIndicationMsg(pMsg); /* no, free it and we're done */
      continue;
    }
    /* handle the command */
    if(pIndication->aClusterId[0] == appDataCluster[0]) {
      /* indicate we're the display */
      gfLedIsDisplay = TRUE;
      /* get the new lederometer readings */
      FLib_MemCpy(&ledDemoValue, pIndication->pAsdu, sizeof(ledDemoValue));
      /* update display with new data */
      TS_SendEvent(gAppTaskID, ledEventDisplay_c);
    }
    /* Free memory allocated by data indication */
    AF_FreeDataIndicationMsg(pMsg);
  }
}
/*****************************************************************************
  BeeAppDataConfirm
  Process incoming ZigBee over-the-air data confirms.
*****************************************************************************/
void BeeAppDataConfirm
  (
  void
  )
{
  apsdeToAfMessage_t *pMsg;
  zbApsdeDataConfirm_t *pConfirm;
  while(MSG_Pending(&BeeAppDataInit(gAppDataConfirmQueue)))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &BeeAppDataInit(gAppDataConfirmQueue) );
    pConfirm = &(pMsg->msgData.dataConfirm);
    /* Action taken when confirmation is received. */
    if( pConfirm->status == gSuccess_c )
    {
      /* successful confirm */
    }
    
    /* Free memory allocated in Call Back function */
    MSG_Free(pMsg);
  }
}
#if gInterPanCommunicationEnabled_c
/*****************************************************************************
  BeeAppInterPanDataIndication
  Process InterPan incoming ZigBee over-the-air messages.
*****************************************************************************/
void BeeAppInterPanDataIndication(void)
{
  InterPanMessage_t *pMsg;
  zbInterPanDataIndication_t *pIndication;
  zbStatus_t status = gZclMfgSpecific_c;
  while(MSG_Pending(&BeeAppDataInit(gInterPanAppDataIndicationQueue)))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &BeeAppDataInit(gInterPanAppDataIndicationQueue) );
    /* ask ZCL to handle the frame */
    pIndication = &(pMsg->msgData.InterPandataIndication );
    /* Handle the Indication here */
    /* Free memory allocated by data indication */
    MSG_Free(pMsg);
  }
}

/*****************************************************************************
  BeeAppDataConfirm
  Process InterPan incoming ZigBee over-the-air data confirms.
*****************************************************************************/
void BeeAppInterPanDataConfirm
(
void
)
{
  InterPanMessage_t *pMsg;
  zbInterPanDataConfirm_t *pConfirm;
  
  while(MSG_Pending(&BeeAppDataInit(gInterPanAppDataConfirmQueue)))
  {
    /* Get a message from a queue */
    pMsg = MSG_DeQueue( &BeeAppDataInit(gInterPanAppDataConfirmQueue) );
    pConfirm = &(pMsg->msgData.InterPandataConf);
    
    /* Action taken when confirmation is received. */
    if( pConfirm->status != gZbSuccess_c )
    {
      /* The data wasn't delivered -- Handle error code here */
    }
    
    /* Free memory allocated in Call Back function */
    MSG_Free(pMsg);
  }
}
#endif 
/******************************************************************************
*******************************************************************************
* Private Functions
*******************************************************************************
*******************************************************************************/

/****************************************************************************
* BeeAppZdpCallBack
*
* ZDP calls this function when it receives a response to a request.
* For example, ASL_MatchDescriptor_req will return whether it worked or not.
*****************************************************************************/
void BeeAppZdpCallBack
  (
  zdpToAppMessage_t *pMsg, 
  zbCounter_t counter
  )
{
  uint8_t event;
  zbMatchDescriptorResponse_t * pMatchRsp;
  (void)counter;
  /* get the event from ZDP */
  event = pMsg->msgType;
  if(event == gzdo2AppEventInd_c) /* many possible application events */
    event = pMsg->msgData.zdo2AppEvent;
  /* got a response to match descriptor */
  switch(event) {
    case gMatch_Desc_rsp_c:
      pMatchRsp = &(pMsg->msgData.matchDescriptorResponse);
      if (pMatchRsp->status == gZbSuccess_c) 
      {
        #if(gInstantiableStackEnabled_d == 0) 
        /* indicate matched */
        LED_SetLed(LED1, gLedOff_c);
        #else
        if(zbProInstance == 0)
        {
          LED_SetLed(LED1, gLedOff_c);
        }
        else
        {
          LED_SetLed(LED3, gLedOff_c);
        }
        #endif
        /* remember destination (nwkaddr + endpoint) */
        gledDstEndPoint = pMatchRsp->matchList[0];  /* match to first endpoint */
        Copy2Bytes(gaLedDstAddr, pMatchRsp->aNwkAddrOfInterest);
        gfLedFoundDst = TRUE;
      }
      /* couldn't find match, give up */
      else
      {
     	#if(gInstantiableStackEnabled_d == 0) 
        LED_SetLed(LED1, gLedOn_c);
        #else
        if(zbProInstance == 0)
        {
          LED_SetLed(LED1, gLedOn_c);
        }
        else
        {
          LED_SetLed(LED3, gLedOn_c);
        }
        #endif
      }
      break;
    /* network has been started */
    case gZDOToAppMgmtZCRunning_c:
    case gZDOToAppMgmtZRRunning_c:
    case gZDOToAppMgmtZEDRunning_c:
      if (appGenericState == mStateIdle_c) {
        appGenericState = mStateZDO_device_running_c;
        
        #if(gInstantiableStackEnabled_d == 0)
        /* stop the flashing and indicate the device is running (has joined the network) */
        LED_SetLed(LED1, gLedOn_c);
        #else
        if(zbProInstance == 0)
        {
          LED_SetLed(LED2, gLedOff_c);
          LED_SetLed(LED1, gLedOn_c);
        }
        else
        {
          LED_SetLed(LED4, gLedOff_c);
          LED_SetLed(LED3, gLedOn_c);
        }
        #endif
        LCD_WriteString(1,(uint8_t *)"Running Device");
        }
      break;
    case gNlmeTxReport_c:
      FA_ProcessNlmeTxReport(&pMsg->msgData.nlmeNwkTxReport);
      break;
    case gMgmt_NWK_Update_notify_c:
      /* Already handle in ZDP. */
      break;
    case gNlmeEnergyScanConfirm_c:
      FA_ProcessEnergyScanCnf(&pMsg->msgData.energyScanConf);
      break;
    case gChannelMasterReport_c:
        /*
          The process wanted to change the channel but too little time has
          passed since the last FA channel change.
        */
      break;
  }
  if (pMsg->msgType == gNlmeEnergyScanConfirm_c)
    MSG_Free(pMsg->msgData.energyScanConf.resList.pEnergyDetectList);
  /* free the message from ZDP */
  MSG_Free(pMsg);
}
/*****************************************************************************
* LedDisplayData
*
*
*****************************************************************************/
void LedDisplayData
  (
  void
  )
{
#if (gLEDSupported_d)
  
  LedState_t ledState = gLedOff_c;
  if(ledDemoValue == TRUE)
  {
    ledState = gLedOn_c;
  }
 
  #if(gInstantiableStackEnabled_d == 0)     
    LED_SetLed(LED2, ledState);
  #else
    if(zbProInstance == 0)
    {
      LED_SetLed(LED2, ledState);
    }
    else
    {
      LED_SetLed(LED4, ledState);
    }
    #endif  
  
#endif  
}
/*****************************************************************************
* LedReportData
*
* Report led value data over-the-air
*
*****************************************************************************/
void LedReportData
  (
  void
  )
{
  afAddrInfo_t  addrInfo;
  /* don't have a place to send data to, give up */
  if(!gfLedFoundDst)
    return;
  /* set up address information */
  addrInfo.dstAddrMode = gZbAddrMode16Bit_c;
  Copy2Bytes(addrInfo.dstAddr.aNwkAddr, gaLedDstAddr);
  addrInfo.dstEndPoint = gledDstEndPoint;
  addrInfo.srcEndPoint = BeeAppDataInit(appEndPoint);
  addrInfo.txOptions = gApsTxOptionNone_c;
  addrInfo.radiusCounter = afDefaultRadius_c;
  /* set up cluster */
  Copy2Bytes(addrInfo.aClusterId, appDataCluster);
  /* send the data request */
  (void)AF_DataRequest(&addrInfo, sizeof(ledDemoValue), &ledDemoValue, NULL);
}
/******************************************************************************
* BeeAppInit.c
*
* Initialization common to all applications. The very start of the program,
* main(), is found here.
*
* Copyright (c) 2008, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor
.
*******************************************************************************/
#include "EmbeddedTypes.h"
#include "MsgSystem.h"
#include "ZtcInterface.h"
#include "BeestackFunctionality.h"
#include "BeeStackInterface.h"
#if gBeeStackIncluded_d
  #include "BeeStackInit.h"
  #include "BeeStackUtil.h"
  #include "BeeStackConfiguration.h"
  #include "ZdoApsInterface.h"
  #include "ZdoMain.h"
#endif /* gBeeStackIncluded_d */
#include "BeeApp.h"
#ifndef gHostApp_d
  //#include "AppAspInterface.h"
#endif
#include "NwkMacInterface.h"
#include "BeeAppInit.h"
#include "BeeApp.h"
#include "EndPointConfig.h"
#include "ZdoNwkInterface.h"
/* For the Nlme_Sync_req */
#include "ASL_ZdpInterface.h"
#include "BeeStackInit.h"
#if gLpmIncluded_d 
//#include "pwr_interface.h"
//#include "pwr_configuration.h"
#endif
#include "Nwkcommon.h"
#include "BeeStack_Globals.h"
#include "NwkVariables.h"
#include "ApsVariables.h"
#include "AfVariables.h"
#include "ZDOVariables.h"
#include "BeeStackRamAlloc.h" 
#ifdef gHostApp_d
#include "ZtcHandler.h"
#endif
/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/
#define mMsgTypeForMSGInd                       0xFC
#ifdef PROCESSOR_KINETIS
  #define InterruptInit()
#endif
/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/
void APP_ZDPJoinPermitReq(uint8_t);
void BeeAppInit(void);
void AppResetApplicationQueues(void);
/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
void Include_Symbols(void);
/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/
#if(gInstantiableStackEnabled_d == 1)
  beeAppDataInit_t* pBeeAppInitData;
#else
  anchor_t gAppDataConfirmQueue;
  anchor_t gAppDataIndicationQueue;
  anchor_t gInterPanAppDataConfirmQueue;
  anchor_t gInterPanAppDataIndicationQueue;
  
  #if (gLpmIncluded_d || gComboDeviceCapability_d)
    uint16_t PollTimeoutBackup; /*Stores orginal pollrate during binding.*/
  #endif
   
  zbEndPoint_t appEndPoint;
  zbTmrTimerID_t gAppGenericTimerId; 
#endif /* gInstantiableStackEnabled_d */ 

tsTaskID_t gAppTaskID;
 	
/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
/*****************************************************************************
* Permit Join Request through ZDO
*****************************************************************************/

/*****************************************************************************
* Callback for Idle timer
*
*****************************************************************************/
#define gIdleTaskNVIntervalEvent_c  ( 1 << 0 )
void IdleTaskNvTimerCallback(zbTmrTimerID_t timerID) {
  (void) timerID;
  //TS_SendEvent(gIdleTaskID, gIdleTaskNVIntervalEvent_c);
}
/***************************************************************************/
/* NOTE: NEED ALOT OF COMMENTS */
uint8_t InterPan_APP_SapHandler
(
  InterPanMessage_t *pMsg /*pointer from Intra Pan to APP*/
)
{
  zbInterPanDataIndication_t *pDataInd;
  uint8_t index;
  zbSimpleDescriptor_t *pSimpleDescriptor;
  uint8_t fMatch = FALSE;
  
  /* For Host application , all received SAP messages are forward 
     to Host(if Host uart communication is enabled) by ZTC */  
#ifndef gHostApp_d
  ZTC_TaskEventMonitor(gInterPanApp_SAPHandlerId_c, (uint8_t *) pMsg, gZbSuccess_c);
#endif
  if(pMsg->msgType == gInterPanDataCnf_c)
  {
    MSG_Queue( &BeeAppDataInit(gInterPanAppDataConfirmQueue), pMsg );
    TS_SendEvent(gAppTaskID, gInterPanAppEvtDataConfirm_c);
  }
  else if(pMsg->msgType == gInterPanDataInd_c)
  {
    /* filter the profile */
    pDataInd =(zbInterPanDataIndication_t *)(&pMsg->msgData.InterPandataIndication);
/* look for matching endpoint
            iIndex starts as 2 in order to ignore broadcast and ZDP endpoint. */
    #if TestProfileApp_d
	for(index = 0; index < ZbStackTablesSizes(gNoOfEndPoints) ; ++index)
	#else		    
    for(index = 2; index < ZbStackTablesSizes(gNoOfEndPoints) ; ++index) 
    #endif
	{
      /* is this a registered endpoint? */
      if(ZbBeeStackGlobalsParams(gaEndPointDesc[index].pDescription))
      {
        zbProfileId_t zllProfileId = {gZllProfileIdentifier_d};
        /* does profile ID match? */
        pSimpleDescriptor = (zbSimpleDescriptor_t *)ZbBeeStackGlobalsParams(gaEndPointDesc[index].pDescription->pSimpleDesc);
        if(IsEqual2Bytes(pSimpleDescriptor->aAppProfId, pDataInd->aProfileId) ||
           IsEqual2Bytes(zllProfileId, pDataInd->aProfileId))
        {
          fMatch = TRUE;
          break;
        }
      }
    } /* end for(...) */
    if(fMatch){
      MSG_Queue( &BeeAppDataInit(gInterPanAppDataIndicationQueue), pMsg );
      TS_SendEvent(gAppTaskID, gInterPanAppEvtDataIndication_c);
    }
    else{
      MSG_Free(pMsg);
    }
  }
  return gZbSuccess_c;
}
/***************************************************************************/
/*
  AppMsgCallBack
  Received a message.
*/
void AppMsgCallBack(apsdeToAfMessage_t *pMsg)
{
  MSG_Queue( &BeeAppDataInit(gAppDataIndicationQueue), pMsg );
  TS_SendEvent(gAppTaskID, gAppEvtDataIndication_c);
}
/***************************************************************************/
/*
  AppCnfCallBack
  Received a confirm.
*/
void AppCnfCallBack(apsdeToAfMessage_t *pMsg)
{
  MSG_Queue( &BeeAppDataInit(gAppDataConfirmQueue), pMsg );
  TS_SendEvent(gAppTaskID, gAppEvtDataConfirm_c);
}
/*
  DeQueue (but don't free) the application message queues.
  
*/
void AppResetApplicationQueues(void)
{
  List_ClearAnchor( &BeeAppDataInit(gAppDataIndicationQueue) );
  List_ClearAnchor( &BeeAppDataInit(gAppDataConfirmQueue) );
  /* Also Cleat all the events to the application. */
  TS_ClearEvent(gAppTaskID, 0xFFFF);
}
/*****************************************************************************
  AF_GetEndPointDevice
  Based on an endpoint number, get the pointer to the device data. Device data
  is unique per endpoint, and only for application endpoints (not ZDO or
  Broadcast).
  Returns pointer to endpoint device or NULL if doesn't exist.
*****************************************************************************/
afDeviceDef_t *AF_GetEndPointDevice
  (
  zbEndPoint_t endPoint /* IN: endpoint # (1-240) */
  )
{
#if gNum_EndPoints_c != 0
  index_t i;
  zbSimpleDescriptor_t *pSimpleDesc;
  if(!endPoint)
    return NULL;
  #if gInstantiableStackEnabled_d 
  for(i=0; i < EndPointConfigData(gNum_EndPoints); ++i) {
  #else
  for(i=0; i < gNum_EndPoints_c; ++i) {
  #endif  
    if(EndPointConfigData(endPointList[i].pEndpointDesc)) {
      pSimpleDesc = EndPointConfigData(endPointList[i].pEndpointDesc->pSimpleDesc);
      if(pSimpleDesc->endPoint == endPoint)
        return (afDeviceDef_t *)EndPointConfigData(endPointList[i].pDevice);
    }
  }
  return NULL;
#else
  (void) endPoint;
  return NULL;
#endif  
}
/*****************************************************************************
* AF_DeviceDefToEndPoint
*
* Look through endpoint descriptors for this device definition. 
* Returns endpoint #.
*****************************************************************************/
zbEndPoint_t AF_DeviceDefToEndPoint
  (
  afDeviceDef_t *pDeviceDef
  )
{
#if gNum_EndPoints_c != 0  
  index_t i;
    #if gInstantiableStackEnabled_d 
    for(i=0; i < EndPointConfigData(gNum_EndPoints); ++i) {
    #else
    for(i=0; i < gNum_EndPoints_c; ++i) {
    #endif 
    if(EndPointConfigData(endPointList[i].pDevice) == pDeviceDef)
      return EndPointConfigData(endPointList[i].pEndpointDesc->pSimpleDesc->endPoint);
  }
  return gInvalidAppEndPoint_c; /* not found */
#else
  (void) pDeviceDef;
  return gInvalidAppEndPoint_c;
#endif  
  
}
/*****************************************************************************
* Get the simple descriptor for this index #
*****************************************************************************/
static zbSimpleDescriptor_t *AF_GetSimpleDescriptor
  (
  uint8_t index
  )
{  
  return (ZbBeeStackGlobalsParams(gaEndPointDesc[index].pDescription) ? ZbBeeStackGlobalsParams(gaEndPointDesc[index].pDescription->pSimpleDesc) : NULL);
}
/*****************************************************************************
* TS_AppBroadcastMsgCallBack
*
* Common routine called whenever a message on the broadcast endpoint (0xff)
* is received.
*
*****************************************************************************/
void AppBroadcastMsgCallBack
  (
  apsdeToAfMessage_t *pMsg  /* IN: broadcast message coming in */
  )
{
  zbSimpleDescriptor_t *pSimpleDescriptor;
  zbApsdeDataIndication_t *pIndication;
  zbEndPoint_t endPoint;
	index_t iIndex;
  apsdeToAfMessage_t *pPrevEpMsg;
  uint8_t prevEndPoint=gZbBroadcastEndPoint_c;
  /* get indication */
  pIndication = &(pMsg->msgData.dataIndication);
	/* look for matching endpoint
            iIndex starts as 2 in order to ignore broadcast and ZDP endpoint. */
	for(iIndex = 2; iIndex < ZbStackTablesSizes(gNoOfEndPoints); ++iIndex) {
	  /* for now, just try the first application endpoint */
  	pSimpleDescriptor = AF_GetSimpleDescriptor(iIndex);
  	if(!pSimpleDescriptor)
			continue;
	  endPoint = pSimpleDescriptor->endPoint;
	  /* profile filter */
  	if(!IsEqual2Bytes(pIndication->aProfileId, pSimpleDescriptor->aAppProfId))
			continue;
	  /* group filter  */
  	if(pIndication->dstAddrMode == gZbAddrModeGroup_c) {
    	if(!ApsGroupIsMemberOfEndpoint(pIndication->aDstAddr,endPoint))
				continue;
			}
    if(prevEndPoint  !=  gZbBroadcastEndPoint_c)
    {
      /* copy all message in order to queue it */
      pPrevEpMsg = AF_MsgAlloc();
      if (pPrevEpMsg)
      {
        FLib_MemCpy(pPrevEpMsg, pMsg, gMaxRxTxDataLength_c);
        pPrevEpMsg->msgData.dataIndication.pAsdu = ((uint8_t *)pMsg->msgData.dataIndication.pAsdu - (uint8_t *)pMsg) + (uint8_t *)pPrevEpMsg;
        /* Copy EndPoint number */
        pPrevEpMsg->msgData.dataIndication.dstEndPoint = prevEndPoint;    /* set endpoint to found application endpoint */
        /* tell ZTC about the message */
#ifndef gHostApp_d        
        ZTC_TaskEventMonitor(gAFDEAppSAPHandlerId_c, (uint8_t *) pPrevEpMsg, mMsgTypeForMSGInd);
#else
        ZTC_TaskEventMonitor(gpHostAppUart, gAFDEAppSAPHandlerId_c, (uint8_t *) pPrevEpMsg, mMsgTypeForMSGInd);
#endif 
        /* pass it on to the app */
        MSG_Queue( &BeeAppDataInit(gAppDataIndicationQueue), pPrevEpMsg);
        TS_SendEvent(gAppTaskID, gAppEvtDataIndication_c);
      }
    }
    /* found a new end point save its number to send it later */
    prevEndPoint = endPoint;
	} /* end of for loop */
	/* no endpoints matched, throw out msg */
	if(prevEndPoint == gZbBroadcastEndPoint_c)
  {
    MSG_Free(pMsg);
		return;
	}
  /* Copy EndPoint Number*/
  pIndication->dstEndPoint = prevEndPoint;    /* set endpoint to found application endpoint */
  /* tell ZTC about the message */
#ifndef gHostApp_d   
  ZTC_TaskEventMonitor(gAFDEAppSAPHandlerId_c, (uint8_t *) pMsg, mMsgTypeForMSGInd);
#else
  ZTC_TaskEventMonitor(gpHostAppUart, gAFDEAppSAPHandlerId_c, (uint8_t *) pMsg, mMsgTypeForMSGInd);
#endif
  /* pass it on to the app */
  MSG_Queue( &BeeAppDataInit(gAppDataIndicationQueue), pMsg );
  TS_SendEvent(gAppTaskID, gAppEvtDataIndication_c);
}
/*****************************************************************************
* AppStartPolling
* 
* This fucntion change the Poll rate to 1 sec and start the interval timer to 
* make polling every 1sec until it is stop calling function AppStopPolling.
*
******************************************************************************/
void AppStartPolling
  (
  void
  )
{  
#if (gLpmIncluded_d || gComboDeviceCapability_d)
#if gComboDeviceCapability_d
  if(ZbBeeStackNwkGlobals(gLpmIncluded))
#endif
  {
 	  uint16_t BindingPollTimeout = 1000; /* 1sec */
 	  /*Only set the polltimeout if lowpower is enabled*/
    if (ZbBeeStackGlobalsParams(gBeeStackConfig.lpmStatus) == 1) {      
      BeeAppDataInit(PollTimeoutBackup) = NlmeGetRequest(gNwkIndirectPollRate_c);
      NlmeSetRequest(gNwkIndirectPollRate_c, BindingPollTimeout);
#ifndef gHostApp_d    
 	    NWK_MNGTSync_ChangePollRate(BindingPollTimeout); 
#endif    
    }
  }
#endif
}
  
/*****************************************************************************
* AppStopPolling
* 
* This fucntion change the Poll rate to 0 so it just make polling when need it.
*
******************************************************************************/
void AppStopPolling
  (
  void
  )
{
#if (gLpmIncluded_d || gComboDeviceCapability_d)
#if gComboDeviceCapability_d
  if(ZbBeeStackNwkGlobals(gLpmIncluded))
#endif
  {
    /*Only restore orginal poll timeout if low power is enabled*/
    if (ZbBeeStackGlobalsParams(gBeeStackConfig.lpmStatus) == 1) {      
     NlmeSetRequest(gNwkIndirectPollRate_c, BeeAppDataInit(PollTimeoutBackup));
#ifndef gHostApp_d    
     NWK_MNGTSync_ChangePollRate(NlmeGetRequest(gNwkIndirectPollRate_c));
#endif     
    }
  }
#endif
}
/*****************************************************************************
* BeeAppDataInitVariables
* 
* 
*
******************************************************************************/
#if(gInstantiableStackEnabled_d == 1)
void BeeAppDataInitVariables(void)
{
  FLib_MemSet(pBeeAppInitData,0,sizeof(beeAppDataInit_t));
}
#endif
#ifdef FRDM_KW24D512
/* Protection for FRDM_KW24D512 at start-up.
The combo sensor is connected to KW24D512 NMI pin trough R117 */
void NMI_Handler(void)
{
    __asm volatile("NOP");
}
#endif
void DBG_VECT_HWfaultISR
(
    void
)
{
   __asm("BKPT #0\n") ; /* cause the debugger to stop */
}
/*! *********************************************************************************
* \file   AppInit.c
* This is a source file for the Application Initialization File.
*
* Copyright (c) 2013, Freescale Semiconductor, Inc.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* o Redistributions of source code must retain the above copyright notice, this list
*   of conditions and the following disclaimer.
*
* o Redistributions in binary form must reproduce the above copyright notice, this
*   list of conditions and the following disclaimer in the documentation and/or
*   other materials provided with the distribution.
*
* o Neither the name of Freescale Semiconductor, Inc. nor the names of its
*   contributors may be used to endorse or promote products derived from this
*   software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
* ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
* ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************************** */

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
************************************************************************************/
#include "EmbeddedTypes.h"
#include "fsl_osa_ext.h"
#include "AppInit.h"
#include "PhyInterface.h"
#include "MacInterface.h"
#include "SerialManager.h"
#include "FsciInterface.h"
#include "MsgSystem.h"
#include "RNG_Interface.h"
#include "MemManager.h"
#include "TimersManager.h"
#include "PWR_Interface.h"
#include "NVM_Interface.h"
#include "LED.h"
#include "ZigbeeTask.h"
#include "board.h"
/*! *********************************************************************************
*************************************************************************************
* Private macros
*************************************************************************************
************************************************************************************/
/* NONE */
/*! *********************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
#if (gLpmIncluded_d || gNvStorageIncluded_d) 
static void AppIdleHandler(void const *argument);
#endif
/*! *********************************************************************************
*************************************************************************************
* Private type definitions
*************************************************************************************
************************************************************************************/
/* NONE */
/*! *********************************************************************************
*************************************************************************************
* Private memory declarations
*************************************************************************************
************************************************************************************/
#if gFsciIncluded_c
#ifndef gInterfaceChannel_c
#define gInterfaceChannel_c 1
#endif
/* FSCI Interface Configuration structure */
static const gFsciSerialConfig_t mFsciSerials[] = {
    /* Baudrate,            interface type,   channel No, virtual interface */
    #if gSerialMgrUseUSB_c
       {gUARTBaudRate115200_c, gSerialMgrUSB_c, 1,           0}
      #if (gFsciMaxInterfaces_c > 1)
      ,{gUARTBaudRate115200_c, gSerialMgrUSB_c, 1,           1}
      #endif
    #elif gSerialMgrUseUart_c
       {gUARTBaudRate115200_c, APP_SERIAL_INTERFACE_TYPE, APP_SERIAL_INTERFACE_INSTANCE,           0}
       #if (gFsciMaxInterfaces_c > 1)
       ,{gUARTBaudRate115200_c, APP_SERIAL_INTERFACE_TYPE, APP_SERIAL_INTERFACE_INSTANCE,           1}
       #endif
    #else
      #warning Please configure the serial used for FSCI 
    #endif
};
#endif

/*! *********************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
/* NONE */
/*! *********************************************************************************
*************************************************************************************
* Private functions
*************************************************************************************
************************************************************************************/
/*! *********************************************************************************
* \brief  Cmsis main thread - init fwk modules, starts conectivity stacks
*         Used for low power module and non volatile module asynchronous operations
* \param[in]  main task param 
*
* \return  None.
*
* \pre
*
* \post
*
* \remarks
*
************************************************************************************/
void main_task(uint32_t param)
{
#if FSL_RTOS_FREE_RTOS
    /* Initialize framework and platform drivers */
    hardware_init();
#endif  
  /* Init memory blocks manager */
  MEM_Init();
  
  /* Init  timers module */
  TMR_Init();
  
  /* Init Led module */
  LED_Init();
  
  /* Init serial manager module*/
  SerialManager_Init();
  
  /* Init phy module */  
  Phy_Init();
  
  /* RNG must be initialized after the PHY is Initialized */
  RNG_Init();
  
  /* Init Keyboard module  */
  KBD_Init(ZbKeyboardCallback);
  
  /* Init mac module */
  MAC_Init();
  /* Init FSCI module */
#if gFsciIncluded_c    
  FSCI_Init( (void*)mFsciSerials );
#endif
  
  /* Init zigbeepro module */
  ZbPro_Init();
  
  /* Call after ZbPro_Init - because of the dynamic dataset registration */
  /* Non volatile memory module init */
  NvModuleInit();
    
  /*All LED's are switched OFF*/
  Led1Off();
  Led2Off();
  Led3Off();
  Led4Off();
#if TestProfileApp_d  
  Led1On();
#endif     
  
  /* Call after Nvm module init - to be able to restore NVM data */
  ZbApplicationInit();
  
#if (gLpmIncluded_d || gNvStorageIncluded_d) 
  AppIdleHandler(&param);
#else  
  /* The main thread is not used anymore*/
  OSA_EXT_TaskDestroy(OSA_EXT_TaskGetId());
#endif  
}
/*! *********************************************************************************
* \brief  Used for Low power module and non volatile module operation
*         
* \param[in]  main task param 
*
* \return  None.
*
* \pre
*
* \post
*
* \remarks
*
************************************************************************************/
#if (gLpmIncluded_d || gNvStorageIncluded_d)
static void AppIdleHandler(void const *argument)
{
    
    while(1)
    {
      #if gNvStorageIncluded_d
        /* Process NV Storage save-on-idle, save-on-count and save-on-interval requests */
        NvIdle();
      #endif
      
      #if gLpmIncluded_d  
      if( PWR_CheckIfDeviceCanGoToSleep() )
      {
            //PWRLib_WakeupReason_t wakeupReason;
            //wakeupReason = PWR_EnterLowPower();
            (void)PWR_EnterLowPower();
            PWR_DisallowDeviceToSleep();
            #if 0
            if( wakeupReason.Bits.FromKeyBoard )
            {
                App_HandleKeys( gKBD_EventSW1_c );
            }
            #endif
      }
      #endif
    }
}
#endif
/*************************************************************************************/
/******************************************************************************
* This is the Source file for device End point
*
* (c) Copyright 2014, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*
******************************************************************************/
#include "EmbeddedTypes.h"
#include "BeeStack_Globals.h"
#include "BeeStackConfiguration.h"
#include "EndPoint.h"
/******************************************************************************
*******************************************************************************
* Private Macros
*******************************************************************************
******************************************************************************/
/* None */
/******************************************************************************
*******************************************************************************
* Private Prototypes
*******************************************************************************
******************************************************************************/
/* None */
/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
/* None */
/******************************************************************************
*******************************************************************************
* Private Memory Declarations
*******************************************************************************
******************************************************************************/

/* Simple Discriptor for 0xFF(Broadcast)*/
const zbSimpleDescriptor_t BroadcastEp =
{
  /*End Point(1Byte), Device Description(2Bytes), Profile ID (2 Bytes),
  AppDeviceVersionAndFlag(1Byte), NumOfInputClusters(1Byte),
  PointerToInputClusterList(1Byte), NumOfOutputClusters(1Byte),
  PointerToOutputClusterList(1Byte) */
  0xFF,
  0x00,0x00,    /* profile ID */
  0x00,0x00,    /* device ID */
  0x00,         /* flags - no user or complex descriptor */
  0x00,         /* # of input clusters */
  NULL,         /* ptr to input clusters */
  0x00,         /* # of output clusters */
  NULL          /* ptr to output clusters */
};

/* End point 0xFF(Broadcast) description */
endPointDesc_t broadcastEndPointDesc =
{
  (zbSimpleDescriptor_t *)&BroadcastEp,   /* simple descriptor */
  AppBroadcastMsgCallBack,                /* broadcast MSG callback */
  NULL,                                   /* no broadcast confirm callback */
  0x00                                    /* Values 1 - 8, 0 if fragmentation is not supported. */
};

/******************************************************************************
*******************************************************************************
* Public Functions
*******************************************************************************
******************************************************************************/
/* None */
/*****************************************************************************/
/************************************************************************************
*
* Copyright (c) 2012, Freescale, Inc.  All rights reserved.
*
* No part of this document may be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
************************************************************************************/
#include "EmbeddedTypes.h"
//#include "AppToPlatformConfig.h"
#if gMAC_PHY_INCLUDED_c
  #include "NwkMacInterface.h"
#endif
/************************************************************************************
*************************************************************************************
* Public memory declarations
*************************************************************************************
************************************************************************************/
#if gMAC_PHY_INCLUDED_c
#ifndef gMAC2006_d
  /* Application allocated space for MAC PIB ACL Entry descriptors. */
#if gNumAclEntryDescriptors_c > 0
  aclEntryDescriptor_t gPIBaclEntryDescriptorSet[gNumAclEntryDescriptors_c];
    /* Set number of ACL entries. Used by the MAC. */
  const uint8_t gNumAclEntryDescriptors = gNumAclEntryDescriptors_c;
  #endif /* gNumAclEntryDescriptors_c */
#else
  
  #if gNumKeyTableEntries_c > 0
    KeyDescriptor_t gPIBKeyTable[gNumKeyTableEntries_c];
    const uint8_t gNumKeyTableEntries = gNumKeyTableEntries_c;
    
    /* Allocate KeyIdLookupDescriptor_t, KeyDeviceDescriptor_t, KeyUsageDescriptor_t */ 
    /* These arrays are part of KeyDescriptor_t structure */
    /* Allocate a continuous space for each array based on the gNumKeyTableEntries_c */
    /* The MAC PIB will initialize the pointers accordingly */
    
    #if gNumKeyIdLookupEntries_c > 0
       KeyIdLookupDescriptor_t gPIBKeyIdLookupDescriptorTable[gNumKeyIdLookupEntries_c * gNumKeyTableEntries_c];
       const uint8_t gNumKeyIdLookupEntries = gNumKeyIdLookupEntries_c; /* The number of elements in each virtual array inside the gPIBKeyIdLookupDescriptorTable */
    #endif //gNumKeyIdLookupEntries_c
    
    #if gNumKeyDeviceListEntries_c > 0
       KeyDeviceDescriptor_t  gPIBKeyDeviceDescriptorTable[gNumKeyDeviceListEntries_c * gNumKeyTableEntries_c];
       const uint8_t gNumKeyDeviceListEntries = gNumKeyDeviceListEntries_c; /* The number of elements for each virtual array inside the gPIBKeyDeviceDescriptorTable */
    #endif //gNumKeyDeviceListEntries_c
    
    #if gNumKeyUsageListEntries_c > 0
       KeyUsageDescriptor_t  gPIBKeyUsageDescriptorTable[gNumKeyUsageListEntries_c * gNumKeyTableEntries_c];
       const uint8_t gNumKeyUsageListEntries = gNumKeyUsageListEntries_c; /* The number of elements for each virtual array inside the gPIBKeyDeviceDescriptorTable */
    #endif //gNumKeyUsageListEntries_c
  #endif //gNumKeyTableEntries_c
  
  #if gNumDeviceTableEntries_c > 0
    DeviceDescriptor_t gPIBDeviceTable[gNumDeviceTableEntries_c];
    const uint8_t gNumDeviceTableEntries = gNumDeviceTableEntries_c;    
  #endif //gNumDeviceTableEntries_c
  #if gNumSecurityLevelTableEntries_c > 0
    SecurityLevelDescriptor_t gPIBSecurityLevelTable[gNumSecurityLevelTableEntries_c];
    const uint8_t gNumSecurityLevelTableEntries = gNumSecurityLevelTableEntries_c;    
  #endif //gNumKeyTableEntries_d

#endif  /*gMAC2006_d*/
#endif /*gMAC_PHY_INCLUDED_c*/
/************************************************************************************
*************************************************************************************
* Private prototypes
*************************************************************************************
************************************************************************************/
/************************************************************************************
*************************************************************************************
* Public functions
*************************************************************************************
************************************************************************************/
/****************************************************************************
* ZigBee Application.
*
* Copyright (c) 2007, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/
#include "EmbeddedTypes.h"
#include "ZigBee.h"
#include "BeeAppInit.h"
#include "AppAfInterface.h"
#include "EndPointConfig.h"
#include "AppZdoInterface.h"
#include "ZdpManager.h"
#include "ZDOStateMachineHandler.h"
#include "BeeStackInterface.h"
#include "ASL_ZdpInterface.h"
#include "ASL_UserInterface.h"
#include "ASL_ZCLInterface.h"
#include "ApsMgmtInterface.h"
#include "keyboard.h"
//#include "Sound.h"
#include "TMR_Interface.h"
#include "beeapp.h"
#include "BeeStackInit.h"
#include "ZdpFrequencyAgility.h"
#include "EzCommissioning.h"
#if gLpmIncluded_d 
#include "PWR_Interface.h"
#endif
#include "ZclSe.h"
#ifdef gHcGenericApp_d    
#if gHcGenericApp_d    
#include "Oep11073.h"
#include "ZclProtocInterf.h"
#endif
#endif
#if gASL_PrintToTestClient_d
#include "ZtcInterface.h"
#endif
#if gZclEnableOTAClient_d 
#include "ZclOta.h"
#endif
#if gZclEnablePwrProfileClusterServer_d
#include "ZclEnergyHome.h"
#endif

#include "ZigbeeTask.h"

/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/
/*Temperature limits for turning on the Leds on the temperature display*/
#define Limit0   -500 
#define Limit1   1000
#define Limit2   2000
#define Limit3   3000
#define Limit4   4000
#define mSecDay_c ((uint32_t)24 * 60 * 60)
#define mPrintStringEvStatus_c TRUE
#define mStartLine  3

#ifdef SmartEnergyApplication_d
#define SmartEnergyApp_d TRUE
#else
#define SmartEnergyApp_d FALSE
#endif
/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/
/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
void ASL_TurnOnLed(LedState_t Led);
void ASL_TurnOffLed(LedState_t Led);
void ASL_FlashLed(LedState_t Led);
void ASL_StopFlashLed(LedState_t Led);
void ZdoStoppedAction(void);
#if (gMC1323xMatrixKBD_d == 1)
void HandleKeysMC1323xMatrixKBD( uint8_t events, uint8_t pressedKey );
#endif
void BeeAppHandleKeys( key_event_t events );
void ASL_ChangeLedsStateOnMode(LED_t LEDNr,LedState_t state, UIMode_t mode);
#if gLCDSupported_d || gASL_PrintToTestClient_d
void ASL_DisplayLine2OnLCD(void);
void ASL_DisplayAppName(void);
void ASL_Dec2Str(char *pStr,uint16_t decValue,index_t digits);
#else
#define ASL_DisplayLine2OnLCD()   /* empty function if no LCD */
#define ASL_DisplayAppName()      /* empty function if no LCD */
#define ASL_Dec2Str(pStr,decValue,digits)
#endif
/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
/* None */
/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/
const uint8_t gsASL_LeaveNetwork[] = "Leave network";
#if (gCoordinatorCapability_d)
  const uint8_t gsASL_StartingNwk[] = "Starting Network";
#else
  const uint8_t gsASL_StartingNwk[] = "Joining Network";
#endif
/* LCD Strings use for displaing diferente messages */
const uint8_t gsASL_ChannelSelect[]=      "Select channel";
const uint8_t gsASL_Running[]=            "Running Device";
const uint8_t gsASL_PermitJoinEnabled[]=  "Permit Join (E)";
const uint8_t gsASL_PermitJoinDisabled[]= "Permit Join (D)";
#ifndef SmartEnergyApplication_d
const uint8_t gsASL_Binding[]=            "Binding";
const uint8_t gsASL_BindingFail[]=        "Binding Fail";
const uint8_t gsASL_BindingSuccess[]=     "Binding Success";
const uint8_t gsASL_UnBinding[]=          "UnBinding";
const uint8_t gsASL_UnBindingFail[]=      "UnBinding Fail";
const uint8_t gsASL_UnBindingSuccess[]=   "UnBinding Success";
const uint8_t gsASL_RemoveBind[]=         "Remove Binding";
#endif
const uint8_t gsASL_ResetNode[]=          "ResetNode";
const uint8_t gsASL_IdentifyEnabled[]=    "Identify Enabled";
const uint8_t gsASL_IdentifyDisabled[]=   "Identify Disabled";
#if gMatch_Desc_req_d
#ifndef SmartEnergyApplication_d
const uint8_t gsASL_Matching[]=           "Matching";
const uint8_t gsASL_MatchFound[]=         "Match Found";
const uint8_t gsASL_MatchFail[]=          "Match Fail";
const uint8_t gsASL_MatchNotFound[]=      "No Match Found";
#endif
#endif
#ifndef gHaCombinedInterface_d
#ifndef SmartEnergyApplication_d
const uint8_t gsASL_SwitchTypeMomentary[]="Momentary SW";
const uint8_t gsASL_SwitchTypeToggle[]=   "Toggle SW";
const uint8_t gsASL_SwitchActionOn[]=     "On Action";
const uint8_t gsASL_SwitchActionOff[]=    "Off Action";
const uint8_t gsASL_SwitchActionToggle[]= "Toggle Action";
const uint8_t gsASL_DispModeCelsius[]=    "Celsius  ";
const uint8_t gsASL_DispModeFahrenheit[]= "Fahrnheit";
#endif
#endif

uint8_t *gpszAslAppName;
const uint8_t gszAslCfgMode[] = "Cfg";
const uint8_t gszAslAppMode[] = "App";
const uint8_t gszAslNoNwkAddr[] = "----";
#if gLCDSupported_d
const uint8_t gProgressBarBitmapPixels[]=
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Empty                 */
  0xFF, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, /* Left empty progress   */
  0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, /* Middle empty progress */
  0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xFF, /* Right empty progress  */
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF  /* Filled progress       */
};
char gLevelLcdString[16] = "Level =        ";
uint8_t gProgressBarIndex[11];
#ifdef gHaThermostat_d
const uint8_t gFanIconBitmapPixels[]=
{
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x00 - Empty         */
  0x00, 0x00, 0x00, 0x00, 0xFE, 0xFE, 0xFC, 0xF8, /* 0x01 - Fan 1-2       */
  0xF0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x02 - Fan 1-3       */
  0x00, 0x18, 0x1C, 0x1E, 0x1F, 0x1F, 0x1F, 0x1F, /* 0x03 - Fan 2-1       */
  0x1E, 0x9E, 0xDC, 0xF8, 0xEF, 0x3F, 0x77, 0xF3, /* 0x04 - Fan 2-2       */
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0x70, 0x30, /* 0x05 - Fan 2-3       */
  0x1E, 0x3F, 0x7F, 0xFF, 0xFF, 0x00, 0x00, 0x00, /* 0x06 - Fan 3-2       */
  0x00, 0x01, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, /* 0x07 - Fan 3-3       */
  0x00, 0x00, 0x02, 0x06, 0x0E, 0x1E, 0x3E, 0x7E, /* 0x08 - ArrowDown 2-1 */
  0xFE, 0x7E, 0x3E, 0x1E, 0x0E, 0x06, 0x02, 0x00, /* 0x09 - ArrowDown 2-2 */
  0x00, 0x00, 0x80, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC, /* 0x0A - ArrowUp   2-1 */
  0xFE, 0xFC, 0xF8, 0xF0, 0xE0, 0xC0, 0x80, 0x00, /* 0x0B - ArrowUp   2-2 */
  
  0x00, 0x00, 0x00, 0xE0, 0xC0, 0xC0, 0xF0, 0x00, /* 0x0C - Snow 1-1 */
  0x08, 0x0C, 0x18, 0xFE, 0xFE, 0x10, 0x18, 0x0C, /* 0x0D - Snow 1-2 */  
  0x00, 0xF0, 0x90, 0xC0, 0xC0, 0x40, 0x00, 0x00, /* 0x0E - Snow 1-3 */  
  0x00, 0x00, 0x00, 0xB6, 0xB2, 0xE3, 0xE1, 0x63, /* 0x0F - Snow 2-1 */    
  0x63, 0x32, 0x36, 0xFF, 0xFF, 0x1C, 0x36, 0x36, /* 0x10 - Snow 2-2 */    
  0xE3, 0xE1, 0xE3, 0xA2, 0xB6, 0x02, 0x00, 0x00, /* 0x11 - Snow 2-3 */    
  0x00, 0x00, 0x01, 0x03, 0x01, 0x00, 0x07, 0x00, /* 0x12 - Snow 3-1 */    
  0x00, 0x18, 0x0C, 0x37, 0x3F, 0x04, 0x0C, 0x18, /* 0x13 - Snow 3-2 */    
  0x01, 0x07, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00, /* 0x14 - Snow 3-3 */
  
  0x00, 0x00, 0x0C, 0x1C, 0x38, 0x70, 0xE0, 0x40, /* 0x15 - Sun 1-1 */
  0x00, 0x80, 0x80, 0xDF, 0xDF, 0x80, 0x80, 0x00, /* 0x16 - Sun 1-2 */  
  0x00, 0xC0, 0xE0, 0x70, 0x38, 0x1C, 0x0C, 0x00, /* 0x17 - Sun 1-3 */  
  0x18, 0x18, 0x18, 0x18, 0x18, 0x00, 0x18, 0x7E, /* 0x18 - Sun 2-1 */    
  0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* 0x19 - Sun 2-2 */    
  0xFE, 0x3C, 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, /* 0x1A - Sun 2-3 */    
  0x00, 0x00, 0x30, 0x38, 0x1C, 0x0E, 0x07, 0x02, /* 0x1B - Sun 3-1 */    
  0x00, 0x01, 0x03, 0xFB, 0xFB, 0x03, 0x01, 0x00, /* 0x1C - Sun 3-2 */    
  0x00, 0x02, 0x07, 0x0E, 0x1C, 0x38, 0x30, 0x00, /* 0x1D - Sun 3-3 */  
  
  0x00, 0x04, 0x04, 0x08, 0x08, 0x10, 0x10, 0x10, /* 0x1E - Wave 1 2-1 */
  0x10, 0x08, 0x08, 0x08, 0x04, 0x04, 0x02, 0x02, /* 0x1F - Wave 1 2-2 */
  0x02, 0x02, 0x02, 0x02, 0x04, 0x04, 0x08, 0x08, /* 0x20 - Wave 1 2-3 */  
  
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x80, /* 0x21 - Wave 2 1-2 */
  0x80, 0x80, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, /* 0x22 - Wave 2 1-3 */  
  0x00, 0x21, 0x21, 0x42, 0x42, 0x84, 0x84, 0x84, /* 0x23 - Wave 2 2-1 */    
  0x84, 0x42, 0x42, 0x42, 0x21, 0x21, 0x10, 0x10, /* 0x24 - Wave 2 2-2 */      
  0x10, 0x10, 0x10, 0x10, 0x21, 0x21, 0x42, 0x42, /* 0x25 - Wave 2 2-3 */        
  
  0x00, 0x40, 0x40, 0x80, 0x80, 0x00, 0x00, 0x00, /* 0x26 - Wave 3 1-1 */
  0x00, 0x80, 0x80, 0x80, 0x40, 0x40, 0x20, 0x20, /* 0x27 - Wave 3 1-2 */  
  0x20, 0x20, 0x20, 0x20, 0x40, 0x40, 0x80, 0x80, /* 0x28 - Wave 3 1-3 */    
  0x00, 0x08, 0x08, 0x10, 0x10, 0x21, 0x21, 0x21, /* 0x29 - Wave 3 2-1 */      
  0x21, 0x10, 0x10, 0x10, 0x08, 0x08, 0x84, 0x84, /* 0x2A - Wave 3 2-2 */          
  0x84, 0x84, 0x84, 0x84, 0x08, 0x08, 0x10, 0x10, /* 0x2B - Wave 3 2-2 */            
  0x00, 0x01, 0x01, 0x02, 0x02, 0x04, 0x04, 0x04, /* 0x2C - Wave 3 3-1 */            
  0x04, 0x02, 0x02, 0x02, 0x01, 0x01, 0x00, 0x00, /* 0x2D - Wave 3 3-2 */            
  0x00, 0x00, 0x00, 0x00, 0x01, 0x01, 0x02, 0x02  /* 0x2E - Wave 3 3-3 */              
};
#define gFanIconY_d 0
const uint8_t gFanLine1[] = {0x00, 0x00, 0x01, 0x02, 0x00};
const uint8_t gFanLine2[] = {0x00, 0x03, 0x04, 0x05, 0x00};
const uint8_t gFanLine3[] = {0x00, 0x00, 0x06, 0x07, 0x00};
#define gHeatStatusIconY_d 5
const uint8_t gThermostatOffLine1[] = {0x00, 0x00, 0x00, 0x00};
const uint8_t gThermostatOffLine2[] = {0x00, 0x00, 0x00, 0x00};
const uint8_t gThermostatOffLine3[] = {0x00, 0x00, 0x00, 0x00};
const uint8_t gThermostatHeatLine1[] = {0x15, 0x16, 0x17, 0x00};
const uint8_t gThermostatHeatLine2[] = {0x18, 0x19, 0x1A, 0x00};
const uint8_t gThermostatHeatLine3[] = {0x1B, 0x1C, 0x1D, 0x00};
const uint8_t gThermostatCoolLine1[] = {0x0C, 0x0D, 0x0E, 0x00};
const uint8_t gThermostatCoolLine2[] = {0x0F, 0x10, 0x11, 0x00};
const uint8_t gThermostatCoolLine3[] = {0x12, 0x13, 0x14, 0x00};
#define gFanStatusIconY_d 9
const uint8_t gWave1Line1[] = {0x00, 0x00, 0x00, 0x00};
const uint8_t gWave1Line2[] = {0x1E, 0x1F, 0x20, 0x00};
const uint8_t gWave1Line3[] = {0x00, 0x00, 0x00, 0x00};
const uint8_t gWave2Line1[] = {0x00, 0x21, 0x22, 0x00};
const uint8_t gWave2Line2[] = {0x23, 0x24, 0x25, 0x00};
const uint8_t gWave2Line3[] = {0x00, 0x00, 0x00, 0x00};
const uint8_t gWave3Line1[] = {0x26, 0x27, 0x28, 0x00};
const uint8_t gWave3Line2[] = {0x29, 0x2A, 0x2B, 0x00};
const uint8_t gWave3Line3[] = {0x2C, 0x2D, 0x2E, 0x00};
uint8_t gThermostatLine1[13];
uint8_t gThermostatLine2[13];
uint8_t gThermostatLine3[13];
#endif /* gHaThermostat_d */
#endif /* gLCDSupported_d */
#if gASL_PrintToTestClient_d 
const uint8_t gaHexValue[] = "0123456789ABCDEF";
#else
extern const uint8_t gaHexValue[];
#endif

/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/
/* Variables to keep track of the status of the Display on the AppMode or ConfigMode */
// ASL_DisplayStatus_t gAppModeDisplay;
// ASL_DisplayStatus_t gConfigModeDisplay;
// uint8_t gmUserInterfaceMode;
  
#if !gInstantiableStackEnabled_d    
  ASL_Data_t gAslData;
  uint8_t gmUserInterfaceMode;
#else
   ASL_Data_t gAslData[2];
   uint8_t gmUserInterfaceMode[2];
   #define gmUserInterfaceMode gmUserInterfaceMode[zbProInstance] 
#endif
/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
/******************************************************************************
* Initialize the user interface for the Application Support Library
******************************************************************************/
void ASL_InitUserInterface
  (
  char *pApplicationName
  )
{
  ASL_SendingNwkData_t mSendingNwkDataInit ={gZbAddrModeIndirect_c,{BeeKitGroupDefault_d},BeeKitSceneDefault_d,{0x00,0x00}};
  /* initialize the global variables */
  PermitJoinStatusFlag = 0xff;
  FLib_MemCpy(&gSendingNwkData,&mSendingNwkDataInit,sizeof(mSendingNwkDataInit));
  
  /* initialize LEDs if available (all boards) */
  //LED_Init();
  /* initialize keyboard if available (all boards) */
#if (gMC1323xMatrixKBD_d == 1)
  //KBD_Init(HandleKeysMC1323xMatrixKBD);
#else  
  //KBD_Init(BeeAppHandleKeys);
#endif  
  /* initialize LCD if available (NCB only) */
  //LCD_Init();
  /* initialize sound and beep if enabled (SRC & NCB only) */
  //BuzzerInit();
  //BuzzerBeep();
  /* register to receive ZDO and ZDP notifications */
  Zdp_AppRegisterCallBack (ASL_ZdoCallBack);
  /* initialize ZigBee Cluster Library (always after registering endpoints) */
  ASL_ZclInit();
  /* start out in config mode */
  ASL_ChangeUserInterfaceModeTo(gConfigureMode_c);
  /* indicate not on network */
#if !gInstantiableStackEnabled_d  
  ASL_ConfigSetLed(LED1,gLedFlashing_c);
#else
  if(0 == zbProInstance)
  {
    ASL_ConfigSetLed(LED1,gLedFlashing_c);
  }
  else
  {
    ASL_ConfigSetLed(LED3,gLedFlashing_c);
  }
#endif  
  /* write both node type and application name */
  gpszAslAppName = (uint8_t*)pApplicationName;
  ASL_DisplayAppName();
  ASL_DisplayLine2OnLCD();
  /* Get the statemachine for FA into start state. */
#if gFrequencyAgilityCapability_d && (!gEndDevCapability_d && !gComboDeviceCapability_d)
  /*
    The combo device should init the FA State Machine when it joins or forms the
    NWK when it gets to the Running State not in here.
  */
  FA_StateMachineInit();
#endif  
}
/******************************************************************************
* Write a string to the LCD line 1, but only for application mode.
******************************************************************************/
#if gLCDSupported_d
void ASL_LCDWriteString
  (
  char *pstr
  )
{
  if (gmUserInterfaceMode == gApplicationMode_c)
    LCD_WriteString(1, (uint8_t*) pstr);
  (void)pstr; /* remove compiler warning in case no LCD */
}
#endif
/*****************************************************************************
* ASL_TurnOffAllLeds
*
* Set LED to on/off or flashing
*****************************************************************************/
void ASL_ToggleLed
  (
  LED_t LEDNr  
  )
{
  LED_ToggleLed(LEDNr);
  (void) LEDNr;
}
/*****************************************************************************
* ASL_TurnOffAllLeds
*
* Set LED to on/off or flashing
*****************************************************************************/
void ASL_TurnOffAllLeds
  (
  void
  )
{
  /* turn off all LEDs, both in display and in the logical display */
  #if !gInstantiableStackEnabled_d
    LED_SetLed(LED_ALL, gLedOff_c);
  #else
      if(0 == zbProInstance)
      {
        LED_SetLed(LED1, gLedOff_c);
        LED_SetLed(LED2, gLedOff_c);
      }
      else
      {
        LED_SetLed(LED3, gLedOff_c);
        LED_SetLed(LED4, gLedOff_c);
      }
  #endif  
  if(gmUserInterfaceMode == gConfigureMode_c)
    gConfigModeDisplay.Leds = 0;
  else
    gAppModeDisplay.Leds = 0;
}
/*****************************************************************************
* ASL_SetLed
*
* Set LED to on/off or flashing
*****************************************************************************/
void ASL_SerialLeds
  (
    uint8_t LedStartPosition
  )
{
  LED_StartSerialFlash(LedStartPosition);
}

/*****************************************************************************
* ASL_UserSetLed
*
* Set LED to on/off or flashing in the Application Mode
*****************************************************************************/
void ASL_AppSetLed
  (
  LED_t LEDNr,
  LedState_t state
  )
{
  
  if (gmUserInterfaceMode == gApplicationMode_c)
     LED_SetLed(LEDNr, state);
  ASL_ChangeLedsStateOnMode(LEDNr,state,gApplicationMode_c);
}
/*****************************************************************************/
void ASL_DisplayAppSettingsLeds(void)
{
  LED_SetLed(LED_FLASH_ALL, gLedStopFlashing_c);
  LED_SetLed(LED_ALL, gLedOff_c);
  LED_SetLed(gAppSettingDisplay.Leds, gLedOn_c); /**keep this order... on first, toggle second */
  LED_SetLed((uint8_t)(gAppSettingDisplay.Leds>>4), gLedFlashing_c );
}
/*****************************************************************************
* ASL_AppSettingsSetLed
*
* Set LED to on/off or flashing in the Application Mode or for Application Settings 
*****************************************************************************/
void ASL_AppSettingsSetLed
  (
  LED_t LEDNr,
  LedState_t state
  )
{
  if ((gmUserInterfaceMode == gApplicationMode_c)||
      (gmUserInterfaceMode == gAppSetting_c))
  {
    ASL_ChangeLedsStateOnMode(LEDNr,state,gAppSetting_c);
    ASL_DisplayAppSettingsLeds();
  }
}
/*****************************************************************************
* ASL_ChangeUserInterfaceModeTo
*****************************************************************************/
void ASL_ChangeUserInterfaceModeTo
  (
  UIMode_t DeviceMode
  )
{
#if !gInstantiableStackEnabled_d    
    LED_StopFlashingAllLeds();
#else
  if(0 == zbProInstance)
  {
    LED_SetLed(LED1|LED2, gLedOff_c);;
  }
  else
  {
    LED_SetLed(LED3|LED4, gLedOff_c);;
  }
#endif    
    gmUserInterfaceMode = DeviceMode;    
}
/*****************************************************************************
* ASL_ChangeUserInterfaceModeTo
*****************************************************************************/
void ASL_DisplayChangeToCurrentMode
  (
  UIMode_t DeviceMode
  )
{
  uint8_t currentLed1, currentLed2; 
  uint8_t currentLed1FLash, currentLed2Flash;
#if (gLEDSupported_d)
  ASL_DisplayStatus_t *pCurrentMode;
  uint8_t Leds;  
#endif
  
  /* update current leds*/
#if gInstantiableStackEnabled_d  
  currentLed1 = (0 == zbProInstance)?LED1:LED3;
  currentLed2 = (0 == zbProInstance)?LED2:LED4;  
  currentLed1FLash = (0 == zbProInstance)?LED1_FLASH:LED3_FLASH;
  currentLed2Flash = (0 == zbProInstance)?LED2_FLASH:LED4_FLASH;
#else
  currentLed1 = LED1;
  currentLed2 = LED2;
  currentLed1FLash = LED1_FLASH;
  currentLed2Flash = LED2_FLASH;
#endif  
  
  /* Clean the Display */
  LCD_ClearDisplay();  
  ASL_DisplayAppName();
  
#if !gInstantiableStackEnabled_d    
  LED_SetLed(LED_ALL,gLedOff_c); 
#else
  LED_SetLed(currentLed1|currentLed2,gLedOff_c); 
#endif 
  /* change to new mode on display and LEDs */
  if (DeviceMode == gApplicationMode_c) 
  {
    ASL_DisplayLine2OnLCD();
#if (gLEDSupported_d)  
    pCurrentMode = &gAppModeDisplay;
#endif
  }
  else
  {
    if (DeviceMode == gConfigureMode_c)
    {
        ASL_DisplayLine2OnLCD();
#if (gLEDSupported_d)		
	pCurrentMode = &gConfigModeDisplay;
#endif
    }
#if (gLEDSupported_d)
    else
      pCurrentMode = &gAppSettingDisplay;
#endif
  }
  
#if (gLEDSupported_d)
  Leds = pCurrentMode->Leds;
  if (Leds & currentLed1FLash) 
  {
    LED_SetLed(currentLed1,gLedFlashing_c);
  }
  else 
  {
    if (Leds & currentLed1)
      LED_SetLed(currentLed1,gLedOn_c);
  }
  if (Leds & currentLed2Flash) 
  {
    LED_SetLed(currentLed2,gLedFlashing_c);
  }
  else 
  {
    if (Leds & currentLed2)
      LED_SetLed(currentLed2,gLedOn_c);
  }
#if !gInstantiableStackEnabled_d   
  if (Leds & LED3_FLASH)
  {
    LED_SetLed(LED3,gLedFlashing_c);
  }
  else 
  {
    if (Leds & LED3)
      LED_SetLed(LED3,gLedOn_c);
  }
  if (Leds & LED4_FLASH) 
  {
    LED_SetLed(LED4,gLedFlashing_c);
  }
  else 
  {
    if (Leds & LED4)
      LED_SetLed(LED4,gLedOn_c);
  }
#endif /* !gInstantiableStackEnabled_d */  
#endif    
}
/*****************************************************************************
* ASL_UpdateDevice
*
* Update the visual display on the device. Also updates the state (for example
* idle or running). Used by ZCL and by ASL to update the display.
*****************************************************************************/
void ASL_UpdateDevice
  (
  zbEndPoint_t ep,              /* IN: endpoint update happend on */
  SystemEvents_t event          /* IN: state to update */
  )
{
#if gInstantiableStackEnabled_d    
  uint8_t currentLed1, currentLed2;
#endif  
  
#if gASL_EnableEZCommissioning_d  
  uint8_t identifyCommissioningState = 0;
  zbClusterId_t identifyClusterId = {gaZclClusterIdentify_c};
  /* Get CommissioningState */
  if(ep != DummyEndPoint)
    (void)ZCL_GetAttribute(ep, identifyClusterId, gZclAttrIdentify_CommissioningState_c, gZclServerAttr_c, &identifyCommissioningState, NULL);
#endif  
#if gInstantiableStackEnabled_d  
  /* update current leds*/
  currentLed1 = (0 == zbProInstance)?LED1:LED3;
  currentLed2 = (0 == zbProInstance)?LED2:LED4;  
#endif  
  
  /* handle the UI event (blink LED, etc...) */
  switch (event) 
  {
    /* Formed/joined network */
    case gZDOToAppMgmtZCRunning_c:
    case gZDOToAppMgmtZRRunning_c:
    case gZDOToAppMgmtZEDRunning_c:
      appState = mStateZDO_device_running_c;
#if (!defined(SmartEnergyApplication_d)) && (!gASL_EnableEZCommissioning_d) && (gZclEnableOTAClient_d && gZclOTADiscoveryServerProcess_d)        
      if(event != gZDOToAppMgmtZCRunning_c)
      { 	         
        (void)OtaClient_StartServerDiscoveryProcess(gNoOfOtaClusterInstances_d);
      }
#endif     
      
#if (gCoordinatorCapability_d || gRouterCapability_d || gComboDeviceCapability_d)&&(!gASL_EnableEZCommissioning_d)
      /* check Permit Join Flag*/
#if gComboDeviceCapability_d
      if (NlmeGetRequest(gDevType_c) != gEndDevice_c)
      {
#endif 
      if (NlmeGetRequest(gNwkPermitJoiningFlag_c))
      {
         PermitJoinStatusFlag = PermitJoinOn;
         ASL_ConfigSetLed(PERMIT_JOIN_LED,gLedOn_c);
      }
      else
      {
         PermitJoinStatusFlag = PermitJoinOff;
      }
#if gComboDeviceCapability_d
      }
#endif      
#endif /* gCoordinatorCapability_d || gRouterCapability_d || gComboDeviceCapability_d */
      /* update Leds */
#if gInstantiableStackEnabled_d       
      ASL_ConfigSetLed(currentLed1,gLedStopFlashing_c);
      ASL_ConfigSetLed(currentLed1,gLedOn_c);
#else     
      ASL_ConfigSetLed(LED1,gLedStopFlashing_c);
      ASL_ConfigSetLed(LED1,gLedOn_c);
#endif /* gInstantiableStackEnabled_d */
      
#if gASL_EnableEZCommissioning_d       
      if(identifyCommissioningState & gZclCommissioningState_OperationalState_d)
      {
          ASL_DisplayChangeToCurrentMode(gmUserInterfaceMode);
      } 
#endif /* gASL_EnableEZCommissioning_d */          	  
      
      /* update display */
      LCD_WriteString(1,(uint8_t *)gsASL_Running);
      ASL_DisplayLine2OnLCD();
      break;
    /* Leave the Network event*/
    case gZDOToAppMgmtStopped_c:
    case gLeaveNetwork_c:
      if ((appState == mStateZDO_device_running_c ) ||(appState == mStateBindSuccess_c))
        ZdoStoppedAction();
      break;
    case gStartNetwork_c:
#if !gInstantiableStackEnabled_d   
      ASL_SerialLeds(LED1);
#else
      ASL_ConfigSetLed(currentLed1,gLedFlashing_c);
      ASL_ConfigSetLed(currentLed2,gLedOff_c);
#endif  
      LCD_WriteString(1,(uint8_t *)gsASL_StartingNwk);
      break;
      
#if gZclEnableOTAProgressReport_d      
    case gOTAProgressReportEvent_c:
      switch(OtaClient_GetProgressReport())
      {
        case otaStartProgress_c:  
          ASL_SetLed(LED2, gLedToggle_c);
          ASL_SetLed(LED3|LED4, gLedOff_c);
          break;
        case otaProgress33_c:
          ASL_SetLed(LED2, gLedOn_c);
          ASL_SetLed(LED3, gLedToggle_c);
          ASL_SetLed(LED4, gLedOff_c);
          break;
        case otaProgress66_c:
          ASL_SetLed(LED2|LED3, gLedOn_c);
          ASL_SetLed(LED4, gLedToggle_c);
          break;  
        case otaProgress100_c:
          ASL_SetLed(LED2|LED3|LED4, gLedOn_c);
          break;   
      }
      break;
#endif      
      
#if !gASL_EnableEZCommissioning_d  && (!defined(SmartEnergyApplication_d))    
    /* Binding Request event */
    case gBind_Device_c:
      if (appState != mStateIdle_c )
      {	
	  AppStartPolling();
          appState = mStateBindRequest_c;
#if !gInstantiableStackEnabled_d  
          ASL_ConfigSetLed(BINDING_LED,gLedFlashing_c);
#endif
          LCD_WriteString(1,(uint8_t *)gsASL_Binding); 
      }
      break;
    /* Binding Success event */
    case gBindingSuccess_c:
      if (appState == mStateBindRequest_c)
      {
	AppStopPolling();
        appState = mStateBindSuccess_c;
#if !gInstantiableStackEnabled_d          
        ASL_ConfigSetLed(BINDING_LED,gLedStopFlashing_c);
        ASL_ConfigSetLed(BINDING_LED,gLedOn_c);
        LCD_WriteString(1,(uint8_t *)gsASL_BindingSuccess);
#endif        
        gSendingNwkData.gAddressMode = gZbAddrModeIndirect_c;
        gSendingNwkData.endPoint = 0x00;
      }
      break;
    /* Unbinding worked */
    case gUnBindingSuccess_c:
      AppStopPolling();
      appState = mStateZDO_device_running_c;
#if !gInstantiableStackEnabled_d        
      ASL_ConfigSetLed(BINDING_LED,gLedOff_c);
      /* "UnBinding success" */
      LCD_WriteString(1,(uint8_t *)gsASL_UnBindingSuccess);
#endif      
      break;
    /* Binding Failure Event */
    case gBindingFailure_c:
      if (appState == mStateBindRequest_c)
      {
	  AppStopPolling();
          appState = mStateZDO_device_running_c;
#if !gInstantiableStackEnabled_d            
          ASL_ConfigSetLed(BINDING_LED,gLedStopFlashing_c);
          ASL_ConfigSetLed(BINDING_LED,gLedOff_c);
          /*"Binding fault" */
          LCD_WriteString(1,(uint8_t *)gsASL_BindingFail);
#endif          
      }
      break;
    /* unbind failed */
    case gUnBindingFailure_c:
      appState = mStateZDO_device_running_c;
#if !gInstantiableStackEnabled_d        
      ASL_ConfigSetLed(BINDING_LED,gLedOn_c);
      /* "UnBinding fault" */
      LCD_WriteString(1,(uint8_t *)gsASL_UnBindingFail);
#endif      
      break;
#if gMatch_Desc_req_d
    case gMatch_Desc_req_c:
      if (appState != mStateIdle_c)
      {
        appState = mStateMatchDescRequest_c;
#if !gInstantiableStackEnabled_d          
        ASL_ConfigSetLed(BINDING_LED,gLedFlashing_c);
        LCD_WriteString(1,(uint8_t *)gsASL_Matching);
#endif        
      }
      break;
    case gMatchDescriptorSuccess_c:
      if (appState == mStateMatchDescRequest_c)
      {
        appState = mStateMatchDescSuccess_c;
        gSendingNwkData.gAddressMode = gZbAddrMode16Bit_c;
 #if !gInstantiableStackEnabled_d         
        ASL_ConfigSetLed(BINDING_LED,gLedStopFlashing_c);
        ASL_ConfigSetLed(BINDING_LED,gLedOn_c);
        /* Match Found */
        LCD_WriteString(1,(uint8_t *)gsASL_MatchFound);
#endif       
      }
      break;
    case gMatchNotFound_c:
      if (appState == mStateMatchDescRequest_c)
      {
        appState = mStateZDO_device_running_c;
#if !gInstantiableStackEnabled_d        
        ASL_ConfigSetLed(BINDING_LED,gLedStopFlashing_c);
        ASL_ConfigSetLed(BINDING_LED,gLedOff_c);
        /* Match Descriptor Not Found  */
        LCD_WriteString(1,(uint8_t *)gsASL_MatchNotFound);
#endif        
      }
      break;
    case gMatchFailure_c:
      if (appState == mStateMatchDescRequest_c)
      {
        appState = mStateZDO_device_running_c;
#if !gInstantiableStackEnabled_d         
        ASL_ConfigSetLed(BINDING_LED,gLedStopFlashing_c);
        ASL_ConfigSetLed(BINDING_LED,gLedOff_c);
        /* Match Descriptor fault */
        LCD_WriteString(1,(uint8_t *)gsASL_MatchFail);
#endif        
      }
      break;
#endif /*  gMatch_Desc_req_d */  
#endif /* !gASL_EnableEZCommissioning_d  &&  !smart energy application*/     
         
    /* permit join toggle */
    case gPermitJoinToggle_c:
#if (gCoordinatorCapability_d || gRouterCapability_d || gComboDeviceCapability_d) && (!gInstantiableStackEnabled_d)
#if gComboDeviceCapability_d
      if (NlmeGetRequest(gDevType_c) == gEndDevice_c)
      {
        break;
      }
#endif /* gComboDeviceCapability_d */
      if (appState == mStateZDO_device_running_c ) 
      {
        if(!PermitJoinStatusFlag) 
        {
          LCD_WriteString( 1,(uint8_t *)gsASL_PermitJoinDisabled );
          ASL_ConfigSetLed(PERMIT_JOIN_LED,gLedOff_c);
        }
        else 
        {
          LCD_WriteString( 1,(uint8_t *)gsASL_PermitJoinEnabled);
          ASL_ConfigSetLed(PERMIT_JOIN_LED,gLedOn_c);
        }
        
      }
#endif /*gCoordinatorCapability_d || gRouterCapability_d || gComboDeviceCapability_d && (!gInstantiableStackEnabled_d)*/
      break;
      
#if (!gInstantiableStackEnabled_d)   
    case gScanningChannels_c:  
      if (appState == mStateIdle_c) 
      {
        LCD_WriteString(1,(uint8_t *)gsASL_ChannelSelect);
        ASL_DisplayLine2OnLCD();
        LED_TurnOffAllLeds();
        LED_SetLed((gmAslChannelNumber - dLowestChannelVal), gLedOn_c);
      }      
      break;
    /* Identify on */
    case gIdentifyOn_c:
      if (appState != mStateIdle_c ) 
      {
        LCD_WriteString( 1,(uint8_t *)gsASL_IdentifyEnabled);
        ASL_SetLed(LED3,gLedFlashing_c);
#if gASL_EnableZllTouchlinkCommissioning_d        
        ASL_ConfigSetLed(LED3, gLedFlashing_c); 
#endif        
      }
      break;
    /* Identify off */
    case gIdentifyOff_c:
      if(appState != mStateIdle_c) 
      {
        LCD_WriteString( 1,(uint8_t *)gsASL_IdentifyDisabled);
        ASL_SetLed(LED3,gLedStopFlashing_c);
        ASL_SetLed(LED3,gLedOff_c);       
#if gASL_EnableZllTouchlinkCommissioning_d        
        ASL_ConfigSetLed(LED3, gLedStopFlashing_c); 
        ASL_ConfigSetLed(LED3,gLedOff_c);
#endif        
      }
      break;
#endif /*(!gInstantiableStackEnabled_d) */
#if gLCDSupported_d && (!(defined(gHaCombinedInterface_d) || defined(gHaOnOffLight_d) || defined(SmartEnergyApplication_d)))
    case gSwitchTypeMomentary:
      if(appState != mStateIdle_c)
        LCD_WriteString( 1,(uint8_t *)gsASL_SwitchTypeMomentary);
      break;
    case gSwitchTypeToggle:
      if(appState != mStateIdle_c)
        LCD_WriteString( 1,(uint8_t *)gsASL_SwitchTypeToggle);
      break;
    case gSwitchActionOn:
      if(appState != mStateIdle_c)
        LCD_WriteString( 1,(uint8_t *)gsASL_SwitchActionOn);
      break;
    case gSwitchActionOff:
      if(appState != mStateIdle_c)
        LCD_WriteString( 1,(uint8_t *)gsASL_SwitchActionOff);
      break;
    case gSwitchActionToggle:
      if(appState != mStateIdle_c)
        LCD_WriteString( 1,(uint8_t *)gsASL_SwitchActionToggle);
      break;
    case gThermostatFahrenheit:
      if(appState != mStateIdle_c) 
        LCD_WriteString( 1,(uint8_t *)gsASL_DispModeFahrenheit);
      break;
    case gThermostatCelsius:
      if(appState != mStateIdle_c)
        LCD_WriteString( 1,(uint8_t *)gsASL_DispModeCelsius);
      break;   
#endif      
 
      
#if gASL_EnableEZCommissioning_d      
    case gZclUI_EZCommissioning_FindingAndBinding_c:
      {
        uint8_t indexDevice = GetIndexFromEZModeDeviceTable(BeeAppDataInit(appEndPoint)); 
        if(indexDevice != gEZModeInvalidIndex_d)
        {
 #if !gInstantiableStackEnabled_d 
          if(EZCommissioningConfigData(gEZModeDeviceInf[indexDevice].isInitiator) == TRUE)
          { 
            ASL_SerialLeds(LED2);     
          }
          else
            ASL_ConfigSetLed(LED4,gLedFlashing_c);
#else
          ASL_ConfigSetLed(currentLed2,gLedFlashing_c);
#endif
        }    
        break;
      }
    case gZclUI_EZCommissioning_Succesfull_c:  
#if !gInstantiableStackEnabled_d 
      ASL_ConfigSetLed(LED1|LED3, gLedOn_c);
      ASL_ConfigSetLed(LED4, gLedStopFlashing_c);
      ASL_ConfigSetLed(LED4, gLedOff_c);       
#else
      ASL_ConfigSetLed(currentLed2, gLedStopFlashing_c);
      ASL_ConfigSetLed(currentLed1|currentLed2,gLedOn_c);
#endif
#if gZclEnableOTAClient_d  &&   gZclOTADiscoveryServerProcess_d   
        (void)OtaClient_StartServerDiscoveryProcess(gNoOfOtaClusterInstances_d);
#endif	/* gZclEnableOTAClient_d && gZclOTADiscoveryServerProcess_d  */    
      break;
#endif /* gASL_EnableEZCommissioning_d */      
  } /* end of switch */
  /* to prevent compiler warnings */
  (void)ep;
}
 
/*****************************************************************************
* ASL_HandleKeys
*
* Default key handling. Handles all config mode keys.
*****************************************************************************/
void ASL_HandleKeys
  (
  key_event_t events  /*IN: Events from keyboard modul */
  )
{
  uint16_t time;
  /* common application mode switches */
  if (gmUserInterfaceMode == gApplicationMode_c) 
  {
    switch (events)
    {
      case gKBD_EventSW3_c:
        /* identify Mode for 10 Seconds */
        if(!ZCL_IdentifyTimeLeft(BeeAppDataInit(appEndPoint)))
          time = gIdentifyTimeSecs_d;
        else
          time = 0;
        ZCL_SetIdentifyMode(BeeAppDataInit(appEndPoint), Native2OTA16(time));
        break;
      case gKBD_EventLongSW1_c:
          /* Change the User Interface Mode to Configmode */
          /* Change the User Interface Display to the current  Mode */
        ASL_ChangeUserInterfaceModeTo(gConfigureMode_c);
        ASL_DisplayChangeToCurrentMode(gmUserInterfaceMode);
        break;
      case gKBD_EventSW4_c: /* Recalls the data for Scene*/
#if gASL_ZclRecallSceneReq_d      
#ifndef SmartEnergyApplication_d        
        ASL_LCDWriteString( "Recall Scene" );
        ASL_ZclRecallSceneUI(gSendingNwkData.aGroupId, gSendingNwkData.aSceneId);
#endif        
#endif        
        break;
      /* Sends an Add Group to the Devices by air */
      case gKBD_EventLongSW3_c:
#if gASL_ZclGroupAddGroupReq_d
#ifndef SmartEnergyApplication_d        
        ASL_LCDWriteString("Add Group" );
        ASL_SetLed(LED3,gLedOn_c);
        if (ASL_ZclAddGroupIfIdentifyUI(gASL_Zcl_InitState_c,gSendingNwkData.aGroupId)!=gZbSuccess_c)
          ASL_SetLed(LED3,gLedOff_c);
        else{
          gSendingNwkData.gAddressMode = gZbAddrModeGroup_c;
					gSendingNwkData.endPoint = 0xff;
					/* save app data into nvm */
          ZCL_SaveNvmZclData();
          }
#endif        
#endif
       break;
      /* Set up the data for Scene to the SW4*/
      case gKBD_EventLongSW4_c:
#if gASL_ZclStoreSceneReq_d
#ifndef SmartEnergyApplication_d        
        ASL_LCDWriteString("Store Scene" );
        ASL_SetLed(LED4,gLedOn_c);
        if(ASL_ZclStoreSceneUI(gASL_Zcl_InitState_c, gSendingNwkData.aGroupId, gSendingNwkData.aSceneId) != gZbSuccess_c)
          ASL_SetLed(LED4,gLedOff_c);
#endif        
#endif        
       break;
      }
    }
  /* config mode configures the node, including joining the network */
  else {  
#if gASL_EnableEZCommissioning_d
    ASL_EZModeCommissioning_ConfigHandleKeys(events);
#else	  
    switch ( events ) 
    {
    /* SW1 joins with NVM. That is, after a reset, it will be back on the network */
   case gKBD_EventSW1_c:
      if (appState == mStateIdle_c) {
        ASL_UpdateDevice(BeeAppDataInit(appEndPoint),gStartNetwork_c);
        /* ZDO_Start:
        gStartAssociationRejoinWithNvm_c
        gStartOrphanRejoinWithNvm_c
        gStartNwkRejoinWithNvm_c
        gStartSilentRejoinWithNvm_c
        */
        ZDO_Start(gStartSilentRejoinWithNvm_c);
      	}
      break;
    case PERMIT_JOIN_SW:        /* SW3 on MC1322x-LPN, SW2 on other boards */
      if (appState != mStateIdle_c) {
#if gCoordinatorCapability_d || gRouterCapability_d || gComboDeviceCapability_d
#if gComboDeviceCapability_d
      if (NlmeGetRequest(gDevType_c) == gEndDevice_c)
      {
        break;
      }
#endif
      if(PermitJoinStatusFlag)
        PermitJoinStatusFlag = PermitJoinOff;
      else
        PermitJoinStatusFlag = PermitJoinOn;
      ASL_UpdateDevice(DummyEndPoint,gPermitJoinToggle_c);
      APP_ZDP_PermitJoinRequest(PermitJoinStatusFlag);
#endif
      }
      break;
    case BINDING_SW:        /* SW2 on MC1322x-LPN, SW3 on other boards */
#ifndef SmartEnergyApplication_d 
#if !defined(gBlackBoxApp_d) && !defined(gBlackBox_d)     
      #if gMatchDescEnable_d
	  if (appState != mStateIdle_c)
          {
              ASL_UpdateDevice(BeeAppDataInit(appEndPoint),gMatch_Desc_req_d);
              ASL_MatchDescriptor_req( NULL, gaBroadcastRxOnIdle, AF_FindEndPointDesc(BeeAppDataInit(appEndPoint))appEndPoint));
	  }
      #else
          if (appState != mStateIdle_c) 
          {
              zbNwkAddr_t  aDestAddress = {0x00,0x00};
              ASL_UpdateDevice(BeeAppDataInit(appEndPoint),gBind_Device_c);       
              ASL_EndDeviceBindRequest(NULL,aDestAddress,EndPointConfigData(endPointList[0].pEndpointDesc->pSimpleDesc));
          }
      #endif
#endif      
#endif	  
      break;
    /* cfg.SW4 - Walk through all the channels (only when idle) */
    case gKBD_EventSW4_c:
      /* only allow selecting if not on network */
      if(appState == mStateIdle_c) {
        zbChannels_t aChannelMask = {0x00, 0x00, 0x00, 0x00};
        LED_SetLed(LED1,gLedOff_c);
        /* set the logical channel */
        if(!gmAslChannelNumber)
          gmAslChannelNumber = NlmeGetRequest(gNwkLogicalChannel_c);
        ++gmAslChannelNumber;
        if(gmAslChannelNumber < dLowestChannelVal || gmAslChannelNumber > dMaxChannelVal)
            gmAslChannelNumber = dLowestChannelVal;
        /* turn it into a bit mask */
        BeeUtilSetIndexedBit(aChannelMask, gmAslChannelNumber);
        ApsmeSetRequest(gApsChannelMask_c, aChannelMask);
        
        NlmeSetRequest(gNwkLogicalChannel_c, &gmAslChannelNumber);   
    
      }
      /* display current channel on LEDs (0x0=channel 11, 0x1=channel 12, 0xf=channel 26) */
      ASL_UpdateDevice(DummyEndPoint,gScanningChannels_c);
      break;
    case gKBD_EventLongSW1_c:
      ASL_ChangeUserInterfaceModeTo(gApplicationMode_c);
      ASL_DisplayChangeToCurrentMode(gmUserInterfaceMode);
      break;
    /* cfg.LSW2 - Leave the network */
    case gKBD_EventLongSW2_c:
      if (appState != mStateIdle_c){
          ASL_TurnOffAllLeds();
          ASL_ConfigSetLed(LED1, gLedFlashing_c);
	        ASL_DisplayAppName();
          ZDO_Leave();
          appState = mStateIdle_c;
  	  }
      break;
    /* cfg.LSW3 - clear binding table */
    case gKBD_EventLongSW3_c:
#ifndef SmartEnergyApplication_d      
      if (appState == mStateBindSuccess_c)
      {
  #if gBindCapability_d
          APS_ClearBindingTable();
  #endif              
          ASL_UpdateDevice(BeeAppDataInit(appEndPoint),gUnBindingSuccess_c);
      }
      else
          ASL_UpdateDevice(BeeAppDataInit(appEndPoint),gUnBindingFailure_c);
#endif            
      break;
    /* start network without NVM */
    case gKBD_EventLongSW4_c:
      if (appState == mStateIdle_c){
	      ASL_UpdateDevice(BeeAppDataInit(appEndPoint),gStartNetwork_c);
        ZDO_Start(gStartWithOutNvm_c);
      }
      //LCD_WriteString(2,(uint8_t *)gsASL_ResetNode);
      break;
    } /* end of switch */
#endif  
  } /* end of if */
}
/*****************************************************************************
* Common code used by both thermostat and temp sensor.
* Only linked in if Thermostat or temp sensor endpoint is used.
*****************************************************************************/
void ASL_DisplayTemperature
(
uint8_t TypeOfTemperature,
int16_t Temperature,
uint8_t displayModeFlag,
uint8_t HeatCoolUnitStatus
)
{
#if gLCDSupported_d 
  char aString[] = "DT=             ";
  int16_t NewTemp=0;
  uint8_t NewTempLength;
   
  if (TypeOfTemperature == gASL_LocalTemperature_c)
     aString[0] = 'L';     
  
  if (displayModeFlag == gZclDisplayMode_TempCelsius_c){
  	NewTemp = Temperature/100;          /* The Real temperatures is = Temperature/100 */
	  aString[6]='C';
	  NewTempLength=2;
  }
  else
  {
    NewTemp = ((9*Temperature)/500)+32; /* Converts the displayed temperature into Farenheit */
    aString[7]='F';
    NewTempLength=3;
	}
  
  if (Temperature < 0)
  {
		aString[3] = '-';
		ASL_Dec2Str(&aString[4],(-1*(NewTemp)), NewTempLength);
  }
  else
		ASL_Dec2Str(&aString[3],NewTemp, NewTempLength);
  switch (HeatCoolUnitStatus){
    case gASL_HCUHeatOn_c:
	    FLib_MemCpy(&aString[9],"HeatOn",sizeof("HeatOn"));
		break;
    case gASL_HCUCoolOn_c:
        FLib_MemCpy(&aString[9],"CoolOn",sizeof("CoolOn"));
		break;
  	}
  #if gTargetMC1322xNCB
    ASL_LCDWriteString(aString);
  #else
    LCD_WriteString(2, (uint8_t*) aString);
  #endif  
#endif

  /*Turn OFF all Led's in app mode*/
  	ASL_SetLed(LED2,gLedOff_c);
  	ASL_SetLed(LED3,gLedOff_c);
  	ASL_SetLed(LED4,gLedOff_c);

  if (Temperature <= Limit0)
  	ASL_SetLed(LED2,gLedFlashing_c);
                   
  if (Temperature >= Limit1)
  	ASL_SetLed(LED2, gLedOn_c);
  if (Temperature >= Limit2)
  	ASL_SetLed(LED3, gLedOn_c);
  
  if (Temperature >= Limit3)
  	ASL_SetLed(LED4, gLedOn_c);
  
  
  if (Temperature >= Limit4)
  	ASL_SetLed(LED4,gLedFlashing_c);
  
  (void) TypeOfTemperature;
  (void) displayModeFlag;
  (void) HeatCoolUnitStatus;
  
}
/******************************************************************************
 ASL_DisplayDutyCycle
*******************************************************************************/
void ASL_DisplayDutyCycle(uint8_t dutyCycle)
{
  
#if gLCDSupported_d 
  char pString[] = "DutyCycle=   %ON";
  ASL_Dec2Str(&pString[10], dutyCycle, 3);
#if gTargetMC1322xNCB  
  ASL_LCDWriteString(pString);
#else
  LCD_WriteString(2, (uint8_t*) pString);
#endif  
#else
  (void) dutyCycle;
#endif
  
 // ASL_DisplayFanSpeed(dutyCycle/25);
  
}
/******************************************************************************
 ASL_DisplayFanSpeed
*******************************************************************************/
void ASL_DisplayFanSpeed(uint8_t speed)
{
  uint8_t led = 1, i;
#if gLCDSupported_d 
  char pString[] = "Speed=    (   %)";  
  ASL_Dec2Str(&pString[7], speed, 1);
  ASL_Dec2Str(&pString[11], 25 * speed, 3);
#if gTargetMC1322xNCB
  ASL_LCDWriteString(pString);
#else
  LCD_WriteString(2, (uint8_t*) pString);
#endif  
#endif
  /* Turn OFF all Led's in app mode*/
  ASL_SetLed(LED1,gLedOff_c);
  ASL_SetLed(LED2,gLedOff_c);
  ASL_SetLed(LED3,gLedOff_c);
  ASL_SetLed(LED4,gLedOff_c);
  for(i = 0; i < speed; i++)
    ASL_SetLed(((led<<i)), gLedOn_c);
  
}
/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/
/*****************************************************************************
* Changes state of LEDs to state for this mode
*****************************************************************************/
void ASL_ChangeLedsStateOnMode
(
LED_t LEDNr,
LedState_t state,
UIMode_t mode
)
{
 ASL_DisplayStatus_t *pModeDisplay;
  if (mode == gApplicationMode_c)
    pModeDisplay = &gAppModeDisplay;
  else
    if (mode == gConfigureMode_c)
       pModeDisplay = &gConfigModeDisplay;
    else
       pModeDisplay = &gAppSettingDisplay;
/* set logical state of LEDs */
  if ((state == gLedOn_c) || (state == gLedFlashing_c)){
    pModeDisplay->Leds |= LEDNr;
    /* In case that the LED is set to Flashing mode, the Led variable of the Current Mode is set too
    0000 0001 => LED1 on, 0001 0001 => LED1 On/flashing */
    if (state == gLedFlashing_c)
      pModeDisplay->Leds |= (LEDNr << 4);
    }
  if ((state == gLedOff_c) || (state == gLedStopFlashing_c)){
    pModeDisplay->Leds  &= ~LEDNr;
	/* In case that the LED i was set to Flashing mode, then to turn of the Led variable of the Current Mode is set too
	     0000 0001 => LED1 on, 0001 0001 => LED1 On/flashing */
  if (state == gLedStopFlashing_c)
    pModeDisplay->Leds &= ~(LEDNr << 4);
  }
  
   if (state == gLedToggle_c)
   {
     if((pModeDisplay->Leds & LEDNr) == 0x00)
      pModeDisplay->Leds |= LEDNr;
     else
      pModeDisplay->Leds &= ~LEDNr ;
   }
  
}
/*****************************************************************************
* ASL_ConfigSetLed
*
* Set LED to on/off or flashing in the Config Mode
*****************************************************************************/
void ASL_ConfigSetLed
  (
  LED_t LEDNr,
  LedState_t state
  )
{
  /* set the LEDs for the proper mode */
  if (gmUserInterfaceMode == gConfigureMode_c)
    /* physically set the LEDs */
    LED_SetLed(LEDNr,state);
  ASL_ChangeLedsStateOnMode(LEDNr,state,gConfigureMode_c);
}
/*****************************************************************************
* zdoStopAction
*****************************************************************************/
void ZdoStoppedAction(void)
{
  LCD_WriteString(1,(void *)gsASL_LeaveNetwork);
  LCD_WriteString(2,"SW1 Restarts");
  appState = mStateIdle_c;
  ASL_ChangeUserInterfaceModeTo(gConfigureMode_c);
  ASL_DisplayLine2OnLCD();
  ASL_TurnOffAllLeds();
  ASL_ConfigSetLed(LED1,gLedFlashing_c);
}
#ifdef gHaThermostat_d
#if gHaThermostat_d
/*****************************************************************************
* Special case for thermostat: make a reverse binding than the one created
* by enddevice bind for the temperature measurement cluster  as reporting needs
* to happen from temp sensor to thermostat
*****************************************************************************/
#ifndef gHostApp_d
void CreateTempMeasurementBinding(void) 
{
  uint8_t i, j;
  uint8_t tempCluster[] = { gaZclClusterTemperatureSensor_c };
  apsBindingTable_Ptr_t  *pBindEntry;
  zbBindUnbindRequest_t bindRequest;
  zbAddressMap_t addrMap;
  uint8_t aNwkAddrLocalCpy[2];
  
  for (i = 0; i < gApsMaxBindingEntries; i++)
  {
    pBindEntry = (void *)&gpBindingTable[i];
    /* If the bind entry is not valid then don't process it */
    if (!pBindEntry->srcEndPoint)
    {
      continue;
    }
    
    for (j = 0; j < pBindEntry->iClusterCount; j++) {
      if (IsEqual2Bytes(pBindEntry->aClusterList[j], tempCluster)) {
         (void)AddrMap_GetTableEntry(pBindEntry->dstAddr.index, &addrMap);
         Copy2Bytes(bindRequest.aClusterId, tempCluster);
         Copy8Bytes(bindRequest.aSrcAddress, addrMap.aIeeeAddr);
         bindRequest.srcEndPoint = pBindEntry->dstEndPoint;
         bindRequest.addressMode = gZbAddrMode64Bit_c;
         Copy8Bytes(bindRequest.destData.extendedMode.aDstAddress, NlmeGetRequest(gNwkIeeeAddress_c));
         bindRequest.destData.extendedMode.dstEndPoint = pBindEntry->srcEndPoint;     
         
         APP_ZDP_BindUnbindRequest(NULL,  APS_GetNwkAddress(addrMap.aIeeeAddr,aNwkAddrLocalCpy), gBind_req_c, &bindRequest);        
         
         break;
      }
    }
  }
}
#else
extern zbBindUnbindRequest_t storedBindReq;
void CreateTempMeasurementBinding(void) 
{
  uint8_t tempCluster[] = { gaZclClusterTemperatureSensor_c };
  zbBindUnbindRequest_t bindRequest;
  uint8_t aWhereToCpyNwkAddr[2];
  Copy2Bytes(bindRequest.aClusterId, tempCluster);
  Copy8Bytes(bindRequest.aSrcAddress, storedBindReq.destData.extendedMode.aDstAddress);
  bindRequest.srcEndPoint = storedBindReq.destData.extendedMode.dstEndPoint;
  bindRequest.addressMode = gZbAddrMode64Bit_c;
  Copy8Bytes(bindRequest.destData.extendedMode.aDstAddress, NlmeGetRequest(gNwkIeeeAddress_c));
  bindRequest.destData.extendedMode.dstEndPoint = storedBindReq.srcEndPoint;     
  APP_ZDP_BindUnbindRequest(NULL, 
                            APS_GetNwkAddress(storedBindReq.destData.extendedMode.aDstAddress, aWhereToCpyNwkAddr), 
                            gBind_req_c,
                            &bindRequest);        

}
#endif /* gHostApp_d */
#endif
#endif


/******************************************************************************
*******************************************************************************
* Private Debug stuff
*******************************************************************************
******************************************************************************/
#if gLCDSupported_d || gASL_PrintToTestClient_d
/*******************************************************************************
* Converts from a hex number into a 4 digit string (does not terminate).
*******************************************************************************/
void ASL_Hex2Str
  (
  char *pStr,
  uint16_t hexValue
  )
{
  index_t digits;
  digits = 4;
  while(digits)
  {
    --digits;
    pStr[digits] = gaHexValue[hexValue & 0xf];
    hexValue = hexValue >> 4;
  }
}
/*******************************************************************************
* Converts from decimal number into a string (does not terminate)
*******************************************************************************/
void ASL_Dec2Str
  (
  char *pStr,
  uint16_t decValue,
  index_t digits
  )
{
  if (digits == 0)
    digits = 2;
  while(digits)
  {
    --digits;
    pStr[digits] = gaHexValue[decValue % 10];
    decValue /= 10;
  }
}
/********************************************************************************
  ASL_DisplayLine2OnLCD
  This function will display the common line2 on the LCD screen (for those
  nodes with LCDs). Line2 includes Keyboard mode (Config mode or Application
  mode), PAN ID (may be 0xffff if not yet on a PAN), Channel, and Nwk (short)
  address, or ---- if not yet on a PAN.
  0123456789012345
  Cfg 1AAA 26 ----
  App 1AAA 11 0001
********************************************************************************/
void ASL_DisplayLine2OnLCD
  (
  void
  )
{
  char aString[17];
  zbNwkAddr_t aNwkAddr;
  zbPanId_t aPanId;
  uint8_t channel;
  /* start with all spaces */
  FLib_MemSet(aString, ' ', sizeof(aString));
  aString[sizeof(aString)-1] = 0;
  /* current mode */
  FLib_MemCpy(aString, gmUserInterfaceMode == gConfigureMode_c ?
    (void *)gszAslCfgMode : (void *)gszAslAppMode, sizeof(gszAslCfgMode)-1);
  /* copy in PAN ID */
  Copy2Bytes(aPanId, NlmeGetRequest(gNwkPanId_c));
  ASL_Hex2Str(&aString[4], OTA2Native16(*((uint16_t *)aPanId)));
  /* copy in channel, first look in NIB */
  /* then in our button-pressed channel */
  /* then convert hard-coded channel */
  channel = gmAslChannelNumber;
  if(!channel)
    channel = NlmeGetRequest(gNwkLogicalChannel_c);
  if(!channel)
    channel = BeeUtilBitToIndex(ApsmeGetRequest(gApsChannelMask_c), 4);
  ASL_Dec2Str(&aString[9], channel, 2);
  /* copy in NWK address */
  Copy2Bytes(aNwkAddr, NlmeGetRequest(gNwkShortAddress_c));
  if(TwoBytesToUint16(aNwkAddr) == 0xffff)
    FLib_MemCpy(&aString[12], (void *)gszAslNoNwkAddr, sizeof(gszAslNoNwkAddr)-1);
  else
    ASL_Hex2Str(&aString[12], OTA2Native16(*((uint16_t *)aNwkAddr)));
  /* write it to line 2 on display */
  LCD_WriteString(2, (uint8_t*)aString);
}
/********************************************************************************
  ASL_DisplayAppName
  Display application name.
********************************************************************************/
void ASL_DisplayAppName
  (
  void
  )
{
    LCD_WriteString(1, gpszAslAppName);
}
#endif

#ifdef gHaDimmableLight_d
#if gHaDimmableLight_d
#if gTargetMC1322xNCB && gLCDSupported_d
/********************************************************************************
  ASL_DisplayLevelBarMC1322xNCB
  Display the level bar
********************************************************************************/
void ASL_DisplayLevelBarMC1322xNCB
  (
    zclLevelValue_t Level,
    zclCmd_t OnOffStatus
  )
{
    uint16_t percent;
    uint16_t displayUnits;
    index_t digits = 2;
    
    (void) OnOffStatus;
    
    gProgressBarIndex[0] = 0;
    FLib_MemSet(&gProgressBarIndex[1], 0x02, 10);
    gProgressBarIndex[1] = 0x01;
    gProgressBarIndex[10] = 0x03;
    
    if (Level == gZclLevelMaxValue)
    {
      percent = 100;
      digits = 3;
    }
    else
      percent = (uint16_t)(Level * 0.3937);
    if (percent < 10)
      digits = 1;
    displayUnits = percent / 10;
    FLib_MemSet(&gProgressBarIndex[1], 0x04, (uint8_t)displayUnits);      
    FLib_MemSet(&gLevelLcdString[8], ' ', 5);
    ASL_Dec2Str(&gLevelLcdString[8], percent, digits);      
    gLevelLcdString[8 + digits] = '%';      
     
    LCD_WriteBitmap(gProgressBarIndex, 11, 4, (unsigned char*)gProgressBarBitmapPixels);    
   
}

/********************************************************************************
  ASL_DisplayLevelPercentageMC1322xNCB
  Display the level percentage
********************************************************************************/
void ASL_DisplayLevelPercentageMC1322xNCB
  (
    zclLevelValue_t Level,
    zclCmd_t OnOffStatus
  )
{
    uint16_t percent;
    uint16_t displayUnits;
    index_t digits = 2;    
    gProgressBarIndex[0] = 0;
    FLib_MemSet(&gProgressBarIndex[1], 0x02, 10);
    gProgressBarIndex[1] = 0x01;
    gProgressBarIndex[10] = 0x03;
    
    if (OnOffStatus == gZclCmdOnOff_Off_c) 
    {
      ASL_LCDWriteString("Light Off");
    }
    else
    {
      if (Level == gZclLevelMaxValue)
      {
        percent = 100;
        digits = 3;
      }
      else
        percent = (uint16_t)(Level * 0.3937);
      if (percent < 10)
        digits = 1;
      displayUnits = percent / 10;
      FLib_MemSet(&gProgressBarIndex[1], 0x04, (uint8_t)displayUnits);      
      FLib_MemSet(&gLevelLcdString[8], ' ', 5);
      ASL_Dec2Str(&gLevelLcdString[8], percent, digits);      
      gLevelLcdString[8 + digits] = '%';      
      
      ASL_LCDWriteString(gLevelLcdString);
    }
       
}

#endif
#endif /* #if  gHaDimmableLight_d */ 
#endif /* #ifdef gHaDimmableLight_d */ 
#ifdef gHaThermostat_d
#if gHaThermostat_d
#if gTargetMC1322xNCB
void ASL_DisplayThermostatStatusMC1322xNCB
  (
    uint8_t heatStatus,
    uint8_t fanStatus
  )
{ 
  FLib_MemCpy(gThermostatLine1, (void *)gFanLine1, sizeof(gFanLine1));
  FLib_MemCpy(gThermostatLine2, (void *)gFanLine2, sizeof(gFanLine2));
  FLib_MemCpy(gThermostatLine3, (void *)gFanLine3, sizeof(gFanLine3));  
  if (heatStatus == gASL_HCUOff_c)
  {
    FLib_MemCpy(&gThermostatLine1[gHeatStatusIconY_d], (void *)gThermostatOffLine1, sizeof(gThermostatOffLine1));
    FLib_MemCpy(&gThermostatLine2[gHeatStatusIconY_d], (void *)gThermostatOffLine2, sizeof(gThermostatOffLine2));
    FLib_MemCpy(&gThermostatLine3[gHeatStatusIconY_d], (void *)gThermostatOffLine3, sizeof(gThermostatOffLine3));    
  }
  else if (heatStatus == gASL_HCUHeatOn_c)
  {
    FLib_MemCpy(&gThermostatLine1[gHeatStatusIconY_d], (void *)gThermostatHeatLine1, sizeof(gThermostatHeatLine1));
    FLib_MemCpy(&gThermostatLine2[gHeatStatusIconY_d], (void *)gThermostatHeatLine2, sizeof(gThermostatHeatLine2));
    FLib_MemCpy(&gThermostatLine3[gHeatStatusIconY_d], (void *)gThermostatHeatLine3, sizeof(gThermostatHeatLine3));
  }
  else if (heatStatus == gASL_HCUCoolOn_c)
  {
    FLib_MemCpy(&gThermostatLine1[gHeatStatusIconY_d], (void *)gThermostatCoolLine1, sizeof(gThermostatCoolLine1));
    FLib_MemCpy(&gThermostatLine2[gHeatStatusIconY_d], (void *)gThermostatCoolLine2, sizeof(gThermostatCoolLine2));
    FLib_MemCpy(&gThermostatLine3[gHeatStatusIconY_d], (void *)gThermostatCoolLine3, sizeof(gThermostatCoolLine3));
  }  
  
  if (fanStatus == gZcl_FanMode_Off_c)
  {
    FLib_MemCpy(&gThermostatLine1[gFanStatusIconY_d], (void *)gThermostatOffLine1, sizeof(gThermostatOffLine1));
    FLib_MemCpy(&gThermostatLine2[gFanStatusIconY_d], (void *)gThermostatOffLine2, sizeof(gThermostatOffLine2));
    FLib_MemCpy(&gThermostatLine3[gFanStatusIconY_d], (void *)gThermostatOffLine3, sizeof(gThermostatOffLine3));    
  }
  else if (fanStatus == gZcl_FanMode_Low_c)
  {
    FLib_MemCpy(&gThermostatLine1[gFanStatusIconY_d], (void *)gWave1Line1, sizeof(gWave1Line1));
    FLib_MemCpy(&gThermostatLine2[gFanStatusIconY_d], (void *)gWave1Line2, sizeof(gWave1Line2));
    FLib_MemCpy(&gThermostatLine3[gFanStatusIconY_d], (void *)gWave1Line3, sizeof(gWave1Line3));        
  }
  else if (fanStatus == gZcl_FanMode_Medium_c)
  {
    FLib_MemCpy(&gThermostatLine1[gFanStatusIconY_d], (void *)gWave2Line1, sizeof(gWave2Line1));
    FLib_MemCpy(&gThermostatLine2[gFanStatusIconY_d], (void *)gWave2Line2, sizeof(gWave2Line2));
    FLib_MemCpy(&gThermostatLine3[gFanStatusIconY_d], (void *)gWave2Line3, sizeof(gWave2Line3));        
  }
  else
  {
    FLib_MemCpy(&gThermostatLine1[gFanStatusIconY_d], (void *)gWave3Line1, sizeof(gWave3Line1));
    FLib_MemCpy(&gThermostatLine2[gFanStatusIconY_d], (void *)gWave3Line2, sizeof(gWave3Line2));
    FLib_MemCpy(&gThermostatLine3[gFanStatusIconY_d], (void *)gWave3Line3, sizeof(gWave3Line3));        
  }  
  
  LCD_WriteBitmap(gThermostatLine1, sizeof(gThermostatLine1), 4, (unsigned char*)gFanIconBitmapPixels);      
  LCD_WriteBitmap(gThermostatLine2, sizeof(gThermostatLine2), 5, (unsigned char*)gFanIconBitmapPixels);      
  LCD_WriteBitmap(gThermostatLine3, sizeof(gThermostatLine3), 6, (unsigned char*)gFanIconBitmapPixels);        
}
#endif
#endif /* #if gHaThermostat_d  */
#endif /* #ifdef gHaThermostat_d  */
#ifdef SmartEnergyApplication_d
/*****************************************************************************
  ASL_PrintUTCTime
  Converts UTC Time in HH:MM:SS
*****************************************************************************/
void ASL_PrintUTCTime
  (
  void
  )
{
#if gLCDSupported_d	
   uint32_t time; 
   uint32_t val;
   uint32_t daySec;
   zbClusterId_t clusterId = {gaZclClusterTime_c};  
   char aStr[14] = "Time= ";
   uint8_t endpoint = ZCL_GetEndPointForSpecificCluster(clusterId, TRUE, 0, NULL)  
   time = ZCL_GetUTCTime(endpoint);
   
   daySec = time % mSecDay_c;
   val = daySec / 3600;
   ASL_Dec2Str(&aStr[6], (uint16_t)val, 2);
   aStr[8]=':';
   val = (daySec % 3600) / 60;
   ASL_Dec2Str(&aStr[9], (uint16_t)val, 2);
   aStr[11]=':';
   val = daySec % 60;
 
   ASL_Dec2Str(&aStr[12], (uint16_t)val, 2);
   
   #if gTargetMC1322xNCB
   LCD_WriteString(7, (uint8_t *)&aStr[0]);
   #elif gTargetQE128EVB_d || gTargetMC1323xRCM_d
   LCD_WriteString(2, (uint8_t *)&aStr[0]);
   #endif
#endif
}
#endif
/*****************************************************************************
* ASL_PrintEvent
*
* Print event strings (e.g. msg rcvd, cancel cnf, etc)
*
*****************************************************************************/
void ASL_PrintEvent(char *eventStr)
{  
  
#if gTargetMC1322xNCB
  LCD_WriteString(2, (uint8_t*) eventStr);
#elif gTargetQE128EVB_d || gTargetMC1323xRCM_d
  LCD_WriteString(1, eventStr);
#else
  (void) eventStr;  
#endif
}

#ifdef SmartEnergyApplication_d
void ASL_PrintMessage(uint8_t *aStr) 
{
#if gTargetMC1322xNCB
  uint8_t i, line = mStartLine;
  
  for(i=0; i<gMaxRcvdMsgToDisplay; i+= gMAX_LCD_CHARS_c)
  {
    LCD_WriteString(line++, aStr);
    aStr+= gMAX_LCD_CHARS_c;
  }
#elif gTargetQE128EVB_d || gTargetMC1323xRCM_d
  LCD_WriteString(2, aStr);
#else
(void) aStr;  
#endif
}
#endif
#if SmartEnergyApp_d || gZclEnablePwrProfileClusterServer_d
void ASL_PrintField(uint8_t fieldNr, void *pData)
{
#if gLCDSupported_d || gASL_PrintToTestClient_d
  uint8_t aStrEvent[16];
  uint32_t priceVal;
  uint8_t pointPlace, printDigit;
  uint8_t  i;
  uint16_t duration;
  
#ifdef SmartEnergyApplication_d 
  const uint8_t *pUnit[10]={"kW","m^3","ft3","ccf","US gl","IMP gl","BTUs","L","kPA(G)","kPA(A)" };
  zclCmdPrice_PublishPriceRsp_t  *pPrice = (zclCmdPrice_PublishPriceRsp_t*) pData;
#endif
 
  
  FLib_MemCpy(aStrEvent, "                ", 16);
  
  switch (fieldNr)  
  {
#ifdef SmartEnergyApplication_d     
  case gASL_FieldPriceTier_c: 
  {    
    /* Print Price Tier number */
    FLib_MemCpy(aStrEvent, (uint8_t*)"PriceTier", 9);
    ASL_Dec2Str((char*)(aStrEvent+10), (pPrice->PriceTrailingDigitAndPriceTier & 0x0F), 1);
  }
  break;
  case gASL_FieldLabel_c: 
  {
    /* Print the Label */
    FLib_MemCpy(aStrEvent, (uint8_t*) "Label=", 6);    
    for(i= pPrice->RateLabel.length + 6; i < gMAX_LCD_CHARS_c; i++)
      *(aStrEvent+i)=' ';
    if(pPrice->RateLabel.length + 6 < gMAX_LCD_CHARS_c)
      FLib_MemCpy((aStrEvent+6), &pPrice->RateLabel.aStr[0], pPrice->RateLabel.length);
    else
      FLib_MemCpy((aStrEvent+6), &pPrice->RateLabel.aStr[0], gMAX_LCD_CHARS_c);
  }    
  break;
        
  case gASL_FieldUom_c: 
  {        
    /* Print the unit Of Measure*/
    FLib_MemCpy(aStrEvent, (uint8_t *)"UM=", 3);
    FLib_MemCpy((aStrEvent+3), (uint8_t*) pUnit[(pPrice->UnitOfMeasure&0x0F)], sizeof(*pUnit[(pPrice->UnitOfMeasure&0x0F)]));
    if((pPrice->UnitOfMeasure&0x0F) < 0x0A)
      if(pPrice->UnitOfMeasure & 0x80)
        FLib_MemCpy((aStrEvent+9), (uint8_t*)"BCD", 3);
      else
        FLib_MemCpy((aStrEvent+9), (uint8_t*)"BIN", 3);
    else
      FLib_MemCpy((aStrEvent+3), (uint8_t*) "NotDefined", 10);
  }
  break;
#endif    
  case gASL_FieldPrice_c: 
  {    
#if gZclEnablePwrProfileClusterServer_d  
  zclCmdPwrProfile_GetPwrProfilePriceRsp_t  *pPrice = (zclCmdPwrProfile_GetPwrProfilePriceRsp_t*) pData;
#endif    
    
    /* Print the Price */
    FLib_MemCpy(aStrEvent, (uint8_t*)"Price=", 6);
#if !gZclEnablePwrProfileClusterServer_d      
    priceVal = FourBytesToUint32(pPrice->Price);
    pointPlace = (pPrice->PriceTrailingDigitAndPriceTier >> 4);
#else    
    priceVal = pPrice->priceInf.price;
    pointPlace = (pPrice->priceInf.priceTrailingDigit);
#endif  
    priceVal = OTA2Native32(priceVal);
    
    printDigit = 15;
    do
    {      
      if(pointPlace == 15 - printDigit )
      {
        *(aStrEvent+printDigit) = '.';
        --printDigit;
      }
      
      *(aStrEvent+printDigit)= (uint8_t)(priceVal%10)+'0';
      --printDigit;
      priceVal = priceVal / 10;
      
    }
    while((priceVal != 0) ||  (pointPlace >= 15 - printDigit));
#if gZclEnablePwrProfileClusterServer_d     
    /* update the buffer */
    for(i=0;i<15-printDigit;i++)
    {
      aStrEvent[6+i] = aStrEvent[16-(15-printDigit)+i];
      aStrEvent[16-(15-printDigit)+i] = ' ';
    }
    
    if(6+(15-printDigit)<16)
      i = 6+(15-printDigit)+1;
    else
      i= 16;
    /* add the currency */
    if(pPrice->priceInf.currency == gISO4217Currency_EUR_c)
      aStrEvent[i] = 'E';
    else
      aStrEvent[i] =  '$';
#endif    
  }
  break;
  
  case gASL_FieldDuration_c: 
  {    
    /* Print the duration */
    FLib_MemCpy(aStrEvent, (uint8_t*)"Duration=", 9);
#ifdef SmartEnergyApplication_d     
    duration = OTA2Native16(pPrice->DurationInMinutes);
    ASL_Dec2Str((char*)(aStrEvent+ 10), duration, 4);
#else
    {
      uint16_t hour = 0;
      uint16_t minute = 0;
      duration = (uint16_t)(*(uint32_t *)(pData));  
      duration = OTA2Native16(duration);
      hour = duration/60;
      minute = duration%60;  
      ASL_Dec2Str((char*)(aStrEvent+ 10), hour, 2);
      aStrEvent[12] = ':';
      ASL_Dec2Str((char*)(aStrEvent+ 13), minute, 2);
    }
#endif    
    
  }
  break;
#if gTargetQE128EVB_d || gTargetMC1323xRCM_d
#ifdef gSeInPremiseDisplay_d
  case gASL_FieldMessage_c: 
  {
    /* Display message on QE128 and exit */
    ASL_PrintMessage((uint8_t*) pData);
    return;
  }
  break;
  
  case gASL_FieldTime_c:
  {
    /* Display UTC time on QE128 board */
    ASL_PrintUTCTime();
    return;
  }
  break;
#endif      
#endif /* gTargetQE128EVB || gTargetMC1323xRCM_d*/  
  } /* switch */
          
  /* Print the field */
  #if gTargetMC1322xNCB
#if gZclEnablePwrProfileClusterServer_d  
  if(fieldNr == gASL_FieldDuration_c)  
    LCD_WriteString(5, (uint8_t*)&aStrEvent[0]);
  if(fieldNr == gASL_FieldPrice_c)
    LCD_WriteString(6, (uint8_t*)&aStrEvent[0]);
#else  
  LCD_WriteString(5, (uint8_t*)&aStrEvent[0]);
#endif  
  #elif gTargetQE128EVB_d || gTargetMC1323xRCM_d
  LCD_WriteString(2, (uint8_t*)&aStrEvent[0]);  
  #endif
    
#if gASL_PrintToTestClient_d
  ASL_PrintToTestClient((uint8_t*)&aStrEvent[0]);
#endif  
#endif /* gLCDSupported_d || gASL_PrintToTestClient_d */  
}

#ifdef SmartEnergyApplication_d
/*****************************************************************************/
void ASL_PrintPriceEvtStatus(uint8_t status) 
{
#if gLCDSupported_d || gASL_PrintToTestClient_d
#if mPrintStringEvStatus_c  
  char *pMsg;
  
  /* Print the Status of the Price event on line 6 */
  switch(status)
  {
  case gPriceReceivedStatus_c:
    pMsg = "PriceReceived   ";
    break;
  case gPriceStartedStatus_c: 
    pMsg = "PriceStarted    ";
    break;
  case gPriceUpdateStatus_c:
    pMsg = "PriceUpdated    ";
    break;
  case gPriceCompletedStatus_c:
    pMsg = "PriceCompleted  ";           
    break;		
  default:
    {
      pMsg = "NoCurrentPrice  ";
    }
    break;
  }
  
  /* Print Status */
  #if gTargetMC1322xNCB
    LCD_WriteString(6, (uint8_t*) pMsg);
  #endif
  #if gTargetQE128EVB_d || gTargetMC1323xRCM_d
    ASL_PrintEvent(pMsg);
  #endif
    
#if gASL_PrintToTestClient_d
    ASL_PrintToTestClient((uint8_t*)pMsg);
#endif    
#else /* mPrintStringEvStatus_c == FALSE */

  uint8_t aStrStatus[] = "Status=         ";
  ASL_Hex2Str((char *)(aStrStatus+7), (uint16_t)status);
  LCD_WriteString(6, &aStrStatus[0]);  
#endif /* #if mPrintStringEvStatus_c */
#endif
}


/*****************************************************************************/

void ASL_PrintLdCtrlEvtStatus(uint8_t status) 
{
#if gLCDSupported_d || gASL_PrintToTestClient_d
#if mPrintStringEvStatus_c
  uint8_t *pStr = "";
  
  switch(status)
  {
  case gSELCDR_LdCtrlEvtCode_CmdRcvd_c:
    pStr = "Received        ";
    break;
  case gSELCDR_LdCtrlEvtCode_Started_c:
    pStr = "Started         ";
    break;
  case gSELCDR_LdCtrlEvtCode_UserHaveToChooseOptOut_c:
    pStr = "HaveOptOut      ";
    break;
  case gSELCDR_LdCtrlEvtCode_UserHaveToChooseOptIn_c:
    pStr = "HaveOptIn       ";
    break;
  case gSELCDR_LdCtrlEvtCode_EvtCancelled_c:
    pStr = "Cancelled       ";
    break;
  case gSELCDR_LdCtrlEvtCode_EvtSuperseded_c:
    pStr = "Superseded      ";
    break;
  case gSELCDR_LdCtrlEvtCode_Completed_c:
    pStr = "Completed       ";
    break;
  case gSELCDR_LdCtrlEvtCode_EvtPrtlCompletedWithUserOptOut_c:
    pStr = "CompletedOptOut ";
    break;
  case gSELCDR_LdCtrlEvtCode_EvtPrtlCompletedWithUserOptIn_c:
    pStr = "CompletedOptIn  ";
    break;
  case gSELCDR_LdCtrlEvtCode_EvtCompletedWithNoUser_c:
    pStr = "CompletedNoUser ";    
    break;
  default:
    pStr = "LDCtrlNotDefined";
    break;
    
  }
  
  #if gTargetMC1322xNCB
  LCD_WriteString(6, &pStr[0]);
  #else
  LCD_WriteString(1, &pStr[0]);  
  #endif
  
  
#else
  uint8_t aStrStatus[]="Status=         "; 
  ASL_Hex2Str((char *)(aStrStatus+7), (uint16_t)status);
  #if gTargetMC1322xNCB
  LCD_WriteString(6, &aStrStatus[0]);
  #else
  LCD_WriteString(1, &aStrStatus[0]);
  #endif
#endif /* #if mPrintStringEvStatus_c */
#endif  
}
const uint8_t aStrEv[]= "Ev    Min    %ON";
const uint8_t aStrHeatSP[]= "HeatingSP=     C";
const uint8_t aStrCoolSP[]= "CoolingSP=     C";
const uint8_t aStrHeatOFS[]= "HeatingOFS=    C";
const uint8_t aStrCoolOFS[]=  "CoolingOFS=    C";
/*****************************************************************************/
void ASL_PrintLdCtrlEvt(uint8_t fieldNr, zclCmdDmndRspLdCtrl_LdCtrlEvtReq_t *pEvt) 
{
#if gLCDSupported_d	
  uint16_t val;
  uint8_t aStrEvent[16];
  
  /* Print duration and duty cycle */
  if(0 == fieldNr)
  {
    FLib_MemCpy(aStrEvent, (uint8_t*)&aStrEv[0], 16);
    val= OTA2Native16(pEvt->DurationInMinutes);
    ASL_Dec2Str((char*)(aStrEvent+3), val, 3);
    ASL_Dec2Str((char*)(aStrEvent+10), pEvt->DutyCycle, 3);
  }
  else
    if(1 == fieldNr)
    {
      /* Print Heating Temperature set point */
      FLib_MemCpy(aStrEvent, (uint8_t*)&aStrHeatSP[0], 16);
      
      val= OTA2Native16(pEvt->HeatingTempSetPoint)/ 100;
      if ((int16_t)val < 0)
  	FLib_MemCpy((aStrEvent+11), "-", 1);
      ASL_Dec2Str((char*)(aStrEvent+12), val, 3);
    }
    else
      if(2 == fieldNr)
      {
        /* Print Cooling Temperature set point */
        FLib_MemCpy(aStrEvent, (uint8_t*)&aStrCoolSP[0], 16);
        val = OTA2Native16(pEvt->CoolingTempSetPoint)/ 100;
        if ((int16_t)val < 0)
          FLib_MemCpy((aStrEvent+11), "-", 1);
        ASL_Dec2Str((char*)(aStrEvent+12), val, 3);
      }
      else
        if(3 == fieldNr)
        {
          /* Print Heating Temperature offset */
          FLib_MemCpy(aStrEvent,(uint8_t*)&aStrHeatOFS[0], 16);
          val = pEvt->HeatingTempOffset/10;
          ASL_Dec2Str((char*)(aStrEvent+12), val, 2);
        }
        else
          if(4 == fieldNr)
          {
            /* Print Cooling Temperature offset */
            FLib_MemCpy(aStrEvent, (uint8_t*)&aStrCoolOFS[0], 16);
            val = pEvt->CoolingTempOffset/10;
            ASL_Dec2Str((char*)(aStrEvent+12), val, 2);
          }
  
  #if gTargetMC1322xNCB
  LCD_WriteString(5, (uint8_t*)&aStrEvent[0]);
  #else
  LCD_WriteString(2, (uint8_t*)&aStrEvent[0]);
  #endif
#endif  
}
#endif /* #ifndef SmartEnergyApplication_d */

#if gZclEnablePwrProfileClusterServer_d
void ASL_PrintPwrProfileEvtStatus(uint8_t status) 
{
  
  uint8_t *pStr = "";
  
  switch(status)
  {
   case gApplianceStatus_Idle_c:    
   case gApplianceStatus_StandBy_c:
    pStr = "State=Idle";
    break;
   case gApplianceStatus_ProgrammedWaitToStart_c: 
    pStr = "State=Wait2Start";
    break;
   case gApplianceStatus_Running_c:
    pStr = "State=Running";
    break;
   case gApplianceStatus_ProgrammedInterrupted_c:
   case gApplianceStatus_Pause_c: 
    pStr = "State=Pause";
    break;
   case gApplianceStatus_EndProgrammed_c:
    pStr = "State=End";
    break; 
  default:
    pStr = "State=Programmed";
    break;
    
  }
  
#if gTargetMC1322xNCB
  LCD_WriteString(4, &pStr[0]);
#else
  LCD_WriteString(1, &pStr[0]);  
#endif  
}
#endif /* gZclEnablePwrProfileClusterServer_d */
#endif /* gLCDSupported_d || gASL_PrintToTestClient_d*/

#if (gMC1323xMatrixKBD_d == 1)
void HandleKeysMC1323xMatrixKBD
( 
  uint8_t events, 
  uint8_t pressedKey 
)
{
  if (events == gKBD_EventLong_c)
    pressedKey +=  gKBD_EventLongSW1_c - gKBD_EventSW1_c;
  BeeAppHandleKeys(pressedKey);
}
#endif
#if gASL_PrintToTestClient_d
void ASL_PrintToTestClient(uint8_t *pStr)
{
#ifndef gHostApp_d  
  ZTCQueue_QueueToTestClient(pStr, 0x70, 0xFF, gMAX_LCD_CHARS_c);
#else
  ZTCQueue_QueueToTestClient(gpHostAppUart, pStr, 0x70, 0xFF, gMAX_LCD_CHARS_c);
#endif            
}
#endif

#if gASL_EnableEZCommissioning_d
/*****************************************************************************
* ASL_EZModeCommissioning_ConfigHandleKeys
*
* EZ mode Commissioning key handling. Handles config mode keys.
*****************************************************************************/
void ASL_EZModeCommissioning_ConfigHandleKeys
  (
  key_event_t events  /*IN: Events from keyboard modul */
  )
{
  switch ( events ) 
  {
    /*cfg.SW1 EZmode Commissioning Start */
	case gKBD_EventSW1_c:
	{
		EZComissioning_Start(gEzCommissioning_NetworkSteering_c | gEzCommissioning_FindingAndBinding_c);
		break;
	}
	 /*cfg.SW2 EZmode Commissioning Start - NetworkSteering */		
	case gKBD_EventSW2_c:
	{
		EZComissioning_Start(gEzCommissioning_NetworkSteering_c);
		break;
	}	
	/*cfg.SW3 EZmode Commissioning Start - FindingAndBinding */	
	case gKBD_EventSW3_c:
	{
		EZComissioning_Start(gEzCommissioning_FindingAndBinding_c);
		break;
	}	
			
        /* cfg.SW4 - Walk through all the channels (only when idle) */
        case gKBD_EventSW4_c:
        {
              /* only allow selecting if not on network */
              if(appState == mStateIdle_c) 
              {
                zbChannels_t aChannelMask = {0x00, 0x00, 0x00, 0x00};
                LED_SetLed(LED1,gLedOff_c);
                /* set the logical channel */
                if(!gmAslChannelNumber)
                  gmAslChannelNumber = NlmeGetRequest(gNwkLogicalChannel_c);
                  ++gmAslChannelNumber;
                if(gmAslChannelNumber < dLowestChannelVal || gmAslChannelNumber > dMaxChannelVal)
                  gmAslChannelNumber = dLowestChannelVal;
                /* turn it into a bit mask */
                BeeUtilSetIndexedBit(aChannelMask, gmAslChannelNumber);
                ApsmeSetRequest(gApsChannelMask_c, aChannelMask);
        
                NlmeSetRequest(gNwkLogicalChannel_c, &gmAslChannelNumber);   
                
              }
              /* display current channel on LEDs (0x0=channel 11, 0x1=channel 12, 0xf=channel 26) */
              ASL_UpdateDevice(DummyEndPoint,gScanningChannels_c);
              break;
        }
    
        /* cfg.LongSW1 - Change User Interface Mode: CfgMode -> App Mode*/
        case gKBD_EventLongSW1_c:
        {
            ASL_ChangeUserInterfaceModeTo(gApplicationMode_c);
            ASL_DisplayChangeToCurrentMode(gmUserInterfaceMode);
            break;    
        }
        /* cfg.LongSW2 - EZModeCommissioning - ResetToFactoryDefault*/
        case gKBD_EventLongSW2_c:
        /* cfg.LongSW3 - EZModeCommissioning - ResetToFactoryDefault*/
        case gKBD_EventLongSW3_c:    
        {
            EZComissioning_Start(gEzCommissioning_FactoryFresh_c);
            break;    	
        }
        /* cfg.LongSW4 */
        case gKBD_EventLongSW4_c:
        {
          /* change Current Endpoint */
          #if gInstantiableStackEnabled_d
            for(uint8_t i=0; i<EndPointConfigData(gNum_EndPoints); i++)
          #else
            for(uint8_t i=0; i<gNum_EndPoints_c; i++)
          #endif
          {
            if(EndPointConfigData(endPointList[i].pEndpointDesc->pSimpleDesc->endPoint) == BeeAppDataInit(appEndPoint))
            {
              #if gInstantiableStackEnabled_d
                if(i == EndPointConfigData(gNum_EndPoints)-1)
              #else
                if(i == gNum_EndPoints_c-1)
              #endif  
                BeeAppDataInit(appEndPoint) = EndPointConfigData(endPointList[0].pEndpointDesc->pSimpleDesc->endPoint);
              else
                BeeAppDataInit(appEndPoint) = EndPointConfigData(endPointList[++i].pEndpointDesc->pSimpleDesc->endPoint);
              
              break;
            }
          }
        }
  }	
}
#endif
/*****************************************************************************
* ZigBee Application.
*
* Copyright (c) 2007, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/
#include "EmbeddedTypes.h"
#include "ZigBee.h"
#include "BeeAppInit.h"
#include "AppAfInterface.h"
#include "EndPointConfig.h"
#include "AppZdoInterface.h"
#include "ZdpManager.h"
#include "ZDOStateMachineHandler.h"
#include "BeeStackInterface.h"
#include "ASL_ZdpInterface.h"
#include "ASL_ZCLInterface.h"
#include "ASL_UserInterface.h"
#include "TMR_Interface.h"
//#include "Timer.h"
#include "ZclGeneral.h"
#include "ZclClosures.h"
/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/
#define gLevelControlInitPayloadSize_c 0xFF
/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/
/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
void ASL_ZclStoreSceneTimerCallBack( uint8_t timerId );
void ASL_ZclAddGroupTimerCallBack( uint8_t timerId );
void ASL_ZclFillAddrInfo( afAddrInfo_t *addrInfo, zclClusterId_t clusterId,  zbEndPoint_t srcEndPoint );
zbStatus_t APSME_AddGroupRequest(zbApsmeAddGroupReq_t *pRequest);
/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
/* None */
/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/
uint8_t       gASL_ZclState;   /* state machine for scene binding */
uint8_t       gASL_ZclTimerID;
zbGroupId_t   gASL_ZclGroupId;
zclSceneId_t  gASL_ZclSceneId;
/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/
//extern zbEndPoint_t appEndPoint;
/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
void ASL_ZclInit(void){  
   /* allocate a timer for the application */
  gASL_ZclTimerID = ZbTMR_AllocateTimer(); 
  ZCL_Init();
}

#if gASL_ZclIdentifyReq_d
/*-------------------- ASL_ZclIdentifyReq --------------------
	3.5.2.3.1 Identify Command (ClusterID=0x0003, CmmId=0x00).
	The identify command starts or stops the receiving device identifying itself.
*/
zclStatus_t ASL_ZclIdentifyReq
(
	zclIdentifyReq_t *pReq
)
{
	/* send the request over the air */
  return ZCL_GenericReq(gZclCmdIdentify_c, sizeof(zclCmdIdentify_t), (zclGenericReq_t *)pReq);  
}
#endif

#if gASL_ZclIdentifyQueryReq_d
/*-------------------- ASL_ZclIdentifyQueryReq --------------------
	3.5.2.3.2 Identify Query Command (ClusterID=0x0003, CmmId=0x01).
	The identify query command allows the sending device to
	request the target or targets to respond if they are
	currently identifying themselves. 
*/

zclStatus_t ASL_ZclIdentifyQueryReq
(
	zclIdentifyQueryReq_t *pReq
)
{
  return ZCL_GenericReq(gZclCmdIdentifyQuery_c, 0, (zclGenericReq_t *)pReq);  
}
zclStatus_t ASL_ZclIdentifyQueryReqUI
(
	zbEndPoint_t srcEndPoint
)
{
  zclIdentifyQueryReq_t req;
	/* set up address info for a broadcast */
	req.addrInfo.dstAddrMode = gZbAddrMode16Bit_c;
	Copy2Bytes(req.addrInfo.dstAddr.aNwkAddr, gaBroadcastAddress);
	ASL_ZclFillAddrInfo(&req.addrInfo, gZclClusterIdentify_c, srcEndPoint);
  return ASL_ZclIdentifyQueryReq(&req);
}
#endif
#if gASL_ZclCmdEzModeInvokeReq_d
zclStatus_t ASL_ZclEzModeInvokeReq
(
    zclEzModeInvokeReq_t *pReq
)
{
  return ZCL_GenericReq(gZclCmdEzModeInvoke_c, sizeof(zclCmdEzModeInvokeReq_t), (zclGenericReq_t *)pReq);  
}
#endif
#if gASL_ZclCmdUpdateCommissioningStateReq_d
zclStatus_t ASL_ZclUpdateCommissioningStateReq
(
    zclUpdateCommissioningStateReq_t *pReq
)
{
  return ZCL_GenericReq(gZclCmdUpdateCommissioningState_c, sizeof(zclCmdUpdateCommissioningStateReq_t), (zclGenericReq_t *)pReq);  
}
#endif
/*****************************************************************************
* ASL_GenericGroupReq
*
* Helper function to create and send some group commands frames over the air.
*****************************************************************************/
#if gASL_ZclGroupViewGroupReq_d ||  gASL_ZclRemoveGroupReq_d || gASL_ZclRemoveAllGroupsReq_d
zclStatus_t ASL_GenericGroupReq 
(
  zclCmd_t command, uint8_t iPayloadLen, void* pCmdFrame, void* pAddrInfo
) 
{
	afToApsdeMessage_t *pMsg;
	pMsg = ZCL_CreateFrame(pAddrInfo, command, gZclFrameControl_FrameTypeSpecific, NULL, &iPayloadLen, pCmdFrame);
	if(!pMsg)
		return gZclNoMem_c;
	return ZCL_DataRequestNoCopy(pAddrInfo, iPayloadLen, pMsg);  
}
#endif

#if gASL_ZclGroupAddGroupReq_d 
/*-------------------- ASL_ZclGroupAddGroupReq -------------------
	3.6.2.2.3  Add Group Command (ClusterId=0x0004, CmmdId=0x00).
	The add group command allows the sending device to add
	membership in a particular group for one or more endpoints
	on the receiving device. 
*/
zclStatus_t ASL_ZclGroupAddGroupReq
(
	zclGroupAddGroupReq_t *pReq
)
{
#if gASL_EnableZLLClustersData_d  && gASL_EnableZllTouchlinkCommissioning_d
  zclStatus_t status = ZllTouchlinkComissioning_AddGroup(pReq->cmdFrame.aGroupId);
  if (status != gZclSuccess_c)
    return status;
#endif  
  return ZCL_GenericReq(gZclCmdGroup_AddGroup_c, sizeof(zclCmdGroup_AddGroup_t)+(pReq->cmdFrame.szGroupName[0]), (zclGenericReq_t *)pReq);
}
#endif
  
#if gASL_ZclGroupViewGroupReq_d
/*-------------------- ASL_ZclGroupViewGroupReq --------------------
	3.6.2.2.4 View Group Command (ClusterID=0x0004, CommandID=0x01).
	The view group command allows the sending device to request that the
	receiving entity or entities respond with a view group response
	command containing the application name string for a particular
	Client for View Group, sends the packet OTA.
*/
zclStatus_t ASL_ZclGroupViewGroupReq
(
	zclGroupViewGroupReq_t *pReq
)
{
  return ASL_GenericGroupReq(gZclCmdGroup_ViewGroup_c, sizeof(zclCmdGroup_ViewGroup_t), pReq->cmdFrame.aGroupId, &(pReq->addrInfo));
}
#endif
#if gASL_ZclGetGroupMembershipReq_d
/*-------------------- ASL_ZclGetGroupMembershipReq -------------------
	3.6.2.2.5 Get group membership command (CLusterID=0x0004 CommandID=0x02).
	The get group membership command allows the sending device to
	inquire about the group membership of the receiving device and
	endpoint in a number of ways.
	Client for Get Group Membership, sends the package OTA.
*/
zclStatus_t ASL_ZclGetGroupMembershipReq
(
	zclGroupGetGroupMembershipReq_t*pReq
)
{
	zbSize_t iPayloadLen = MbrOfs(zclCmdGroup_GetGroupMembership_t, aGroupId[0]) + 
								         (pReq->cmdFrame.count * MbrSizeof(zclCmdGroup_GetGroupMembership_t, aGroupId));
  return ZCL_GenericReq(gZclCmdGroup_GetGroupMembership_c, iPayloadLen, (zclGenericReq_t *)pReq);  
}
#endif
#if gASL_ZclRemoveGroupReq_d
/*-------------------- ASL_ZclRemoveGroupReq --------------------
	3.6.2.2.6 Remove Group command (ClusterID=0x0004 CommandID=0x03).
	The remove group command allows the sender to request that the
	receiving entity or entities remove their membership, if any, in
	a particular group.
	Note that if a group is removed the scenes associated with that
	group should be removed.
*/
zclStatus_t ASL_ZclRemoveGroupReq
(
	zclGroupRemoveGroupReq_t *pReq
)
{
#if gASL_EnableZLLClustersData_d && gASL_EnableZllTouchlinkCommissioning_d 
  (void)ZllTouchlinkComissioning_RemoveGroup(pReq->cmdFrame.aGroupId);
#endif  
  
  return ASL_GenericGroupReq(gZclCmdGroup_RemoveGroup_c, sizeof(zclCmdGroup_RemoveGroup_t), (pReq->cmdFrame.aGroupId), &(pReq->addrInfo));
}
#endif
#if gASL_ZclRemoveAllGroupsReq_d
/*-------------------- ASL_ZclRemoveAllGroupsReq --------------------
	3.6.2.2.7 Remove All Groups command (ClusterID=0x0004 Command=0x04)
	The remove all groups command allows the sending device to direct
	the receiving entity or entities to remove all group associations.
	Note that removing all groups necessitates the removal of all
	associated scenes as well. (Note: scenes not associated with a
	group need not be removed).
*/
zclStatus_t ASL_ZclRemoveAllGroupsReq
(
	zclGroupRemoveAllGroupsReq_t *pReq
)
{
#if gASL_EnableZLLClustersData_d  && gASL_EnableZllTouchlinkCommissioning_d
  zbGroupId_t aGroupId = {0xFF, 0xFF};
  (void)ZllTouchlinkComissioning_RemoveGroup(aGroupId);
#endif    
  return ASL_GenericGroupReq(gZclCmdGroup_RemoveAllGroups_c, 0, NULL, &(pReq->addrInfo));
}
#endif
#if gASL_ZclGroupAddGroupReq_d
/*-------------------- ASL_ZclGroupAddGroupReq -------------------
		3.6.2.2.8  Add Group If Identifying Command (ClusterId=0x0004, CmmdId=0x05).
	The add group commands allows the sending device to add
	membership in a particular group for one or more endpoints
	on the receiving device. The add group if identifying adds 
	the group membership only if destination device is identifying
	when it receives the request.
*/
zclStatus_t ASL_ZclGroupAddGroupIfIdentifyReq
(
	zclGroupAddGroupIfIdentifyingReq_t*pReq
)
{
	zbSize_t iPayloadLen = sizeof(zclCmdGroup_AddGroupIfIdentifying_t)+(pReq->cmdFrame.szGroupName[0]);
  return ZCL_GenericReq(gZclCmdGroup_AddGroupIfIdentifying_c, iPayloadLen, (zclGenericReq_t *)pReq);    
}
#endif
/* Scenes cluster commands */
#if gASL_ZclSceneAddSceneReq_d
/*-------------------- ASL_ZclSceneAddSceneReq --------------------
	3.7.2.4.1 Add Scene command (ClusterID=0x0005 CommandID=0x00).
	The add scene command allows the sender to request that the
	receiving entity to add a specified scene to their scene table.
  Returns gZbSuccess_t if worked.
*/
zbStatus_t ASL_ZclSceneAddSceneReq
  (
	zclSceneAddSceneReq_t *pReq,   /* IN */
	uint8_t len                    /* IN: length of command frame (already subtracted off addr info) */
  )
{
  /* generate the request (variable length) */
	return ZCL_GenericReq(gZclCmdScene_AddScene_c,
		len,(zclGenericReq_t *)pReq);
}
#if gASL_EnableZLLClustersData_d 
/*-------------------- ASL_ZclSceneEnhancedAddSceneReq --------------------
         ZLL Specifications: 6.5.1.3.2 Enhanced add scene command
        (ClusterID=0x0005 CommandID=0x40).
	The enhanced add scene command allows the sender to request that the
	receiving entity to add a specified scene to their scene table.
  Returns gZbSuccess_t if worked.
*/
zbStatus_t ASL_ZclSceneEnhancedAddSceneReq
  (
	zclSceneAddSceneReq_t *pReq,   /* IN */
	uint8_t len                    /* IN: length of command frame (already subtracted off addr info) */
  )
{
  /* generate the request (variable length) */
	return ZCL_GenericReq(gZclCmdScene_EnhancedAddScene_c,
		len,(zclGenericReq_t *)pReq);
}
#endif
#endif
#if gASL_ZclViewSceneReq_d
/*-------------------- ASL_ZclViewSceneReq --------------------
	3.7.2.4.2 View Scene command (ClusterID=0x0005 CommandID=0x01).
	The view scene command allows the sender to request that the
	receiving entity to generate a View Scene response for a specified scene.
	
  Returns gZbSuccess_t if worked.
 */
zbStatus_t ASL_ZclViewSceneReq
(
	zclSceneViewSceneReq_t *pReq
)
{
	return ZCL_GenericReq(gZclCmdScene_ViewScene_c,
		sizeof(zclCmdScene_ViewScene_t),
	(zclGenericReq_t *)pReq);
}
#if gASL_EnableZLLClustersData_d        
/*-------------------- ASL_ZclEnhancedViewSceneReq --------------------
        ZLL Specifications: 6.5.1.3.3 Enhanced view scene command
        (ClusterID=0x0005 CommandID=0x41).
	The enhanced view scene command allows the sender to request that the
	receiving entity to generate a View Scene response for a specified scene.
	
  Returns gZbSuccess_t if worked.
 */
zbStatus_t ASL_ZclEnhancedViewSceneReq
(
	zclSceneViewSceneReq_t *pReq
)
{
	return ZCL_GenericReq(gZclCmdScene_EnhancedViewScene_c,
		sizeof(zclCmdScene_ViewScene_t),
	(zclGenericReq_t *)pReq);
}
#endif
#endif

#if gASL_ZclRemoveSceneReq_d
/*-------------------- ASL_ZclRemoveSceneReq --------------------
	3.7.2.4.3 Remove Scene command (ClusterID=0x0005 CommandID=0x02).
	The remove scene command allows the sender to request that the
	receiving entity to remove a specified scene from the scene table.
 
  Returns gZbSuccess_t if worked.
*/
zbStatus_t ASL_ZclRemoveSceneReq
  (
  zclSceneRemoveSceneReq_t *pReq
  )
{
  return ZCL_GenericReq(gZclCmdScene_RemoveScene_c,
    sizeof(zclCmdScene_RemoveScene_t),
    (zclGenericReq_t *)pReq);
}
#endif
#if gASL_ZclRemoveAllScenesReq_d
/*-------------------- ASL_ZclRemoveAllScenesReq --------------------
	3.7.2.4.4 Remove All Scenes command (ClusterID=0x0005 CommandID=0x03).
	The remove all scenes command allows the sender to request that the
	receiving entity or entities to remove all scenes from the scene table.
 Returns gZbSuccess_t if worked.
*/
zbStatus_t ASL_ZclRemoveAllScenesReq
  (
  zclSceneRemoveAllScenesReq_t *pReq
  )
{
  return ZCL_GenericReq(gZclCmdScene_RemoveAllScenes_c,
    sizeof(zclCmdScene_RemoveAllScenes_t),
    (zclGenericReq_t *)pReq);
}
#endif
#if gASL_ZclStoreSceneReq_d
/*-------------------- ASL_ZclStoreSceneReq --------------------
	3.7.2.4.5 Store Scene command (ClusterID=0x0005 CommandID=0x04).
	On receipt of this command, the device shall (if possible) add
	an entry to the Scene Table with the Scene ID and Group ID given
	in the command, and all extension fields corresponding to the current
	state of other clusters on the device. If an entry already exists
	with the same Scene ID and Group ID it will be replaced.
	If the command was addressed to a single device (not to a group) then
	it shall generate an appropriate Store Scene Response command indicating
	success or failure. See 3.7.2.5.5.
*/
zbStatus_t ASL_ZclStoreSceneReq
(
	zclSceneStoreSceneReq_t *pReq
)
{
	return ZCL_GenericReq(gZclCmdScene_StoreScene_c,
						sizeof(zclCmdScene_StoreScene_t),
						(zclGenericReq_t *)pReq);
}
#endif
#if gASL_ZclRecallSceneReq_d
/*-------------------- ASL_ZclRecallSceneReq --------------------
	3.7.2.4.6 Recall Scene command (ClusterID=0x0005 Command=0x05)
	On receipt of this command, the device shall (if possible) locate
	the entry in its Scene Table with the Group ID and Scene ID given
	in the command. For each other cluster on the device, it shall then
	retrieve any corresponding extension fields from the Scene Table
	and set the attributes and corresponding state of the cluster accordingly.
*/
zbStatus_t ASL_ZclRecallSceneReq
(
	zclSceneRecallSceneReq_t *pReq
)
{
	return ZCL_GenericReq(gZclCmdScene_RecallScene_c,
												sizeof(zclCmdScene_StoreScene_t),
												(zclGenericReq_t *)pReq);
}
#endif
#if gASL_ZclGetSceneMembershipReq_d
/*-------------------- ASL_ZclGetSceneMembershipReq --------------------
	3.7.2.4.7 Get Scene Membership command (ClusterID=0x0005 CommandID=0x06).
	The get scene membership command allows the sender to request that the
	receiving entity to answer with the scene IDs associated to a specified 
	group ID in its scene table.
  Returns gZbSuccess_t if worked.
*/
zbStatus_t ASL_ZclGetSceneMembershipReq
  (
  zclSceneGetSceneMembershipReq_t *pReq
  )
{
  return ZCL_GenericReq(gZclCmdScene_GetSceneMembership_c,
    sizeof(zclCmdScene_GetSceneMembership_t),
    (zclGenericReq_t *)pReq);
}
#endif
 #if gASL_EnableZLLClustersData_d  &&  gASL_ZclCopySceneReq_d        
/*-------------------- ASL_ZclEnhancedViewSceneReq --------------------
        ZLL Specifications: 6.5.1.3.4 Copy Scene scene command
        (ClusterID=0x0005 CommandID=0x42).
	The copy scene command allows the sender to request that the
	receiving entity to generate a Copy Scene response for a specified scene.
	
  Returns gZbSuccess_t if worked.
 */
zbStatus_t ASL_ZclSceneCopySceneReq
(
	zclSceneCopyScene_t *pReq
)
{
	return ZCL_GenericReq(gZclCmdScene_CopyScene_c,
		sizeof(zclCmdScene_CopyScene_t),
	(zclGenericReq_t *)pReq);
}
#endif                       
    
  
#if gASL_ZclAddGroupHandler_d && gASL_ZclGroupAddGroupReq_d
/*****************************************************************************
* ASL_ZclAddGroupHandler
*
* Adds a group.
*****************************************************************************/
void ASL_ZclAddGroupHandler(void)
{
  /* done with state machine? */
  if(gASL_ZclState >= 1)
  {
    ZbTMR_StopTimer(gASL_ZclTimerID);
    ASL_SetLed(LED3, gLedOn_c);
  }
  /* on to the next state */
  ++gASL_ZclState;
  if(ASL_ZclAddGroupIfIdentifyUI(gASL_ZclState, gASL_ZclGroupId) != gZbSuccess_c) {
    ASL_SetLed(LED3, gLedOff_c);
  }
}
#endif
#if gASL_ZclStoreSceneHandler_d && gASL_ZclStoreSceneReq_d
/*****************************************************************************
* ASL_ZclStoreSceneHandler
*
* Store a scene.
*****************************************************************************/
void ASL_ZclStoreSceneHandler(void)
{
  /* done with state machine? */
  if(gASL_ZclState >= 2)
  {
    ZbTMR_StopTimer(gASL_ZclTimerID);
    ASL_SetLed(LED4, gLedOn_c);
  }
  /* on to the next state */
  ++gASL_ZclState;
  if(ASL_ZclStoreSceneUI(gASL_ZclState, gASL_ZclGroupId, gASL_ZclSceneId) != gZbSuccess_c) {
    ASL_SetLed(LED4, gLedOff_c);
  }
}
#endif

/*****************************************************************************
* ASL_ZclStoreSceneTimerCallBack
*
* Store a scene.
*****************************************************************************/
void ASL_ZclStoreSceneTimerCallBack( uint8_t timerId )
{
  /* avoid compiler warning */
  (void)timerId;
  TS_SendEvent(gAppTaskID, gAppEvtStoreScene_c);
}
/*****************************************************************************
* ASL_ZclAddGroupCallBack
*
* Store a scene.
*****************************************************************************/
void ASL_ZclAddGroupTimerCallBack( uint8_t timerId )
{
  /* avoid compiler warning */
  (void)timerId;
  TS_SendEvent(gAppTaskID, gAppEvtAddGroup_c);
}
void ASL_ZclInitStateUI
(
  zbGroupId_t aGroupId,
  zclSceneId_t sceneId,
  void (*pfTimerCallBack)(zbTmrTimerID_t)
)
{
	gASL_ZclState = gASL_Zcl_InitState_c;   /* timer will update the state */
	Copy2Bytes(gASL_ZclGroupId, aGroupId);	
	gASL_ZclSceneId = sceneId;
	ZbTMR_StartIntervalTimer(gASL_ZclTimerID, 100, pfTimerCallBack);
}
/*****************************************************************************
* ASL_ZclAddGroupIfIdentifyUI
*
* Adds remote nodes to the group if they are in identify mode.
* Function is used to add group adding functionality to board UI (button press).
*****************************************************************************/
#if gASL_ZclGroupAddGroupReq_d
zbStatus_t ASL_ZclAddGroupIfIdentifyUI
(
  uint8_t mode,
  zbGroupId_t aGroupId   /* IN: */
  
)
{
	zbStatus_t status = gZbSuccess_c;
	zclGroupAddGroupIfIdentifyingReq_t groupReq;
	/* start up the state machine */
	if(mode == gASL_Zcl_InitState_c) {
    ASL_ZclInitStateUI(aGroupId, 0, ASL_ZclAddGroupTimerCallBack); 
		return gZbSuccess_c;
	}
	/* add the group for those in identify mode, in this case this will be done if the, identify query returns success*/
	if(mode == gASL_Zcl_IdentifyState_c) {
		/* set up address info for a broadcast */
		groupReq.addrInfo.dstAddrMode = gZbAddrMode16Bit_c;
		Copy2Bytes(groupReq.addrInfo.dstAddr.aNwkAddr, gaBroadcastAddress);
		ASL_ZclFillAddrInfo(&groupReq.addrInfo, gZclClusterGroups_c, BeeAppDataInit(appEndPoint));
		/* set up add group information, group number and name to be added to the group table*/
		Copy2Bytes(groupReq.cmdFrame.aGroupId, gASL_ZclGroupId);
		groupReq.cmdFrame.szGroupName[0] = 0x00; /* no OTA name */
		/* Send the Add group req */
		status = ASL_ZclGroupAddGroupIfIdentifyReq(&groupReq);
	}
	
  return status;
}
#endif

/*****************************************************************************
* ASL_ZclStoreSceneUI
*
* Stores a scene on remote nodes that are in identify mode. Also adds the
* remote nodes to the group.
* Function is used to add store scene functionality to board UI (button press).
*****************************************************************************/
#if gASL_ZclStoreSceneReq_d
zbStatus_t ASL_ZclStoreSceneUI
  (
  uint8_t mode,
  zbGroupId_t aGroupId,   /* IN: */
  zclSceneId_t sceneId    /* IN: */
  )
{
  zbStatus_t status = gZbSuccess_c;
  zclSceneStoreSceneReq_t sceneReq;
  zclGroupAddGroupReq_t   groupReq;
  /* start up the state machine */
  if(mode == gASL_Zcl_InitState_c) {
    ASL_ZclInitStateUI(aGroupId, sceneId, ASL_ZclStoreSceneTimerCallBack); 
    return gZbSuccess_c;
  }
  /* set up address info for a broadcast */
  groupReq.addrInfo.dstAddrMode = gZbAddrModeGroup_c;
  Copy2Bytes(groupReq.addrInfo.dstAddr.aGroupId, aGroupId);
  ASL_ZclFillAddrInfo(&groupReq.addrInfo, gZclClusterGroups_c, BeeAppDataInit(appEndPoint));  
  /* call query identify (results come back to the app endpoint) */
  if(mode == gASL_Zcl_IdentifyState_c)
      return gZbSuccess_c;
  /* set up request */
  FLib_MemCpy(&sceneReq.addrInfo, &groupReq.addrInfo, sizeof(groupReq.addrInfo));  
  Set2Bytes(sceneReq.addrInfo.aClusterId, gZclClusterScenes_c);
  Copy2Bytes(sceneReq.cmdFrame.aGroupId, aGroupId);
  sceneReq.cmdFrame.sceneId = sceneId;
  /* call query identify (results come back to the app endpoint) */
  if(mode == gASL_Zcl_RequestState_c) {
    status = ASL_ZclStoreSceneReq(&sceneReq);
  }
  return status;
}
#endif

#if gASL_ZclRecallSceneReq_d
/*****************************************************************************
* ASL_ZclRecallSceneUI
*
* Recalls a previously stored scene on remote nodes.
* Function is used to add recall scene functionality to board UI (button press).
*****************************************************************************/
void ASL_ZclRecallSceneUI
  (
  zbGroupId_t aGroupId,   /* IN: */
  zclSceneId_t sceneId    /* IN: */
  )
{
  zclSceneRecallSceneReq_t req;
  /* set up address info for a broadcast */
  
  req.addrInfo.dstAddrMode = gZbAddrModeGroup_c;
  Copy2Bytes(req.addrInfo.dstAddr.aGroupId, aGroupId);
  ASL_ZclFillAddrInfo(&req.addrInfo, gZclClusterScenes_c, BeeAppDataInit(appEndPoint));
  Copy2Bytes(req.cmdFrame.aGroupId, aGroupId);
  req.cmdFrame.sceneId = sceneId;
  (void)ASL_ZclRecallSceneReq(&req);
}
#endif
#if gASL_ZclOnOffReq_d
/*-------------------- ASL_ZclOnOffReq --------------------
	3.8.2.3.1 Off Command (ClusterID=0x0006 CommandID=0x00).
	3.8.2.3.2 On Command (ClusterID=0x0006 CommandID=0x01).
	3.8.2.3.3 Toggle Command (ClusterID=0x0006 CommandID=0x02).
*/
zbStatus_t ASL_ZclOnOffReq
(
	zclOnOffReq_t *pReq
)
{
	Set2Bytes(pReq->addrInfo.aClusterId, gZclClusterOnOff_c);
	return ZCL_GenericReqNoData(&(pReq->addrInfo), pReq->command);
}
#endif
#if gASL_ZclDoorLockReq_d
/*-------------------- ASL_ZclDoorLockReq --------------------
	10.1.2.15.1 LockDoor, 10.1.2.15.2 UnlockDoor, 10.1.2.15.3 Toggle, 10.1.2.15.4  UnlockWithTimeout command,
    10.1.2.15.7  Set pin code command, 10.1.2.15.7 Set rfid code command 
*/
zbStatus_t ASL_ZclDoorLockReq
(
    zclDoorLockCmdReq_t *pReq
)
{
	uint8_t status = gZclUnsupportedClusterCommand_c;
	switch (pReq->cmdId) 
	{
		case gZclCmdDoorLock_Lock_c:
		case gZclCmdDoorLock_Unlock_c:
		case gZclCmdDoorLock_Toggle_c:	
			status = zclDoorLock_LockUnlockToogleReq(&pReq->zclZtcDoorLockReq.doorLockReq, pReq->cmdId);
			break;
		case gZclCmdDoorLock_UnlockWithTimeout_c:
			status = zclDoorLock_UnlockWithTimeoutReq(&pReq->zclZtcDoorLockReq.doorLockUnlockWithTimeout);
			break;
		case gZclCmdDoorLock_SetPinCode_c:
		case gZclCmdDoorLock_SetRFIDCode_c:
			status = zclDoorLock_SetPinRFIDCode(&pReq->zclZtcDoorLockReq.doorLockSetPinReq, pReq->cmdId);
			break;
		default:
			status = gZclUnsupportedClusterCommand_c;	
			break;
	}
	return status;
}
/*-------------------- ASL_ZclDoorLock_ClearCmdsReq --------------------
*/
zbStatus_t ASL_ZclDoorLock_ClearCmdsReq
(
	zclDoorLockClearCmdReq_t *pReq
)
{
	uint8_t status = gZclUnsupportedClusterCommand_c;
	switch (pReq->cmdId) 
	{
		case gZclCmdDoorLock_ClearPinCode_c:
		case gZclCmdDoorLock_ClearRFIDCode_c:
			status = zclDoorLock_ClearPinRFIDCode(&pReq->zclZtcDoorLockClearReq.doorLockClearPinRfidCode, pReq->cmdId);
			break;
		case gZclCmdDoorLock_ClearAllPinCodes_c:		
		case gZclCmdDoorLock_ClearAllRFIDCodes_c:
			status = zclDoorLock_ClearAllPinRFIDCodes(&pReq->zclZtcDoorLockClearReq.doorLockClearAllPinRfidCodes, pReq->cmdId);
			break;
		case gZclCmdDoorLock_ClearWeekdaySchedule_c:
			status = zclDoorLock_ClearWeekDaySchedule(&pReq->zclZtcDoorLockClearReq.doorLockClearWeekDaySchedule);
			break;			
		case gZclCmdDoorLock_ClearYeardaySchedule_c:
			status = zclDoorLock_ClearYearDaySchedule(&pReq->zclZtcDoorLockClearReq.doorLockClearYearDaySchedule);
			break;
		case gZclCmdDoorLock_ClearHolidaySchedule_c:
			status = zclDoorLock_ClearHolidaySchedule(&pReq->zclZtcDoorLockClearReq.doorLockClearHolidaySchedule);
			break;			
		default:
			status = gZclUnsupportedClusterCommand_c;	
			break;
	}
	return status;
}
#endif

/*-------------------- ASL_ZclLevelControlReq --------------------
  Level control requests:
  3.10.2.3.1 Move To Level (ClusterID=0x0008 CommandID=0x00).
  3.10.2.3.2 Move (ClusterID=0x0008 CommandID=0x01).
  3.10.2.3.3 Step (ClusterID=0x0008 CommandID=0x02).
  3.10.2.3.4 Stop (ClusterID=0x0008 CommandID=0x03).  
  3.10.2.3.5 Move To Level with On/Off (ClusterID=0x0008 CommandID=0x04).
  3.10.2.3.6 Move with On/Off (ClusterID=0x0008 CommandID=0x05).
  3.10.2.3.7 Step with On/Off (ClusterID=0x0008 CommandID=0x06).
  3.10.2.3.8 Stop with On/Off (ClusterID=0x0008 CommandID=0x07).   
*/
#if gASL_ZclLevelControlReq_d
zbStatus_t ASL_ZclLevelControlReq
(
	zclLevelControlReq_t * pReq,
	uint8_t command
)
{
  zbSize_t iPayloadLen = gLevelControlInitPayloadSize_c;
  afToApsdeMessage_t *pMsg=NULL;
  switch (command) {
     case gZclCmdLevelControl_MoveToLevel_c:
     case gZclCmdLevelControl_MoveToLevelOnOff_c:
         iPayloadLen = sizeof(zclCmdLevelControl_MoveToLevel_t);
         break;
     case gZclCmdLevelControl_Move_c:
     case gZclCmdLevelControl_MoveOnOff_c:
         iPayloadLen = sizeof(zclCmdLevelControl_Move_t);
         break;
     case gZclCmdLevelControl_Step_c:
     case gZclCmdLevelControl_StepOnOff_c:     
         iPayloadLen = sizeof(zclCmdLevelControl_Step_t);
         break;
     case gZclCmdLevelControl_Stop_c:
     case gZclCmdLevelControl_StopOnOff_c:
         iPayloadLen = gZero_c;
         break;
     default:
         break;     
  }                   
  
  if (iPayloadLen != gLevelControlInitPayloadSize_c) {
    pMsg = ZCL_CreateFrame(&(pReq->stopReq.addrInfo), command,gZclFrameControl_FrameTypeSpecific | gZclFrameControl_DisableDefaultRsp,
            NULL, &iPayloadLen,&(pReq->stopReq.cmdFrame));
    if (!pMsg)
      return gZbNoMem_c;
                     
    return ZCL_DataRequestNoCopy(&(pReq->stopReq.addrInfo), iPayloadLen, pMsg);
  }
  return gZclNoMem_c;
}
#endif
/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/
void ASL_ZclFillAddrInfo
(
  afAddrInfo_t *addrInfo, zclClusterId_t clusterId,  zbEndPoint_t srcEndPoint
) 
{
  addrInfo->dstEndPoint = gZbBroadcastEndPoint_c;
  Set2Bytes(addrInfo->aClusterId, clusterId);
  addrInfo->srcEndPoint = srcEndPoint;
  addrInfo->txOptions = gZclTxOptions;
  addrInfo->radiusCounter = afDefaultRadius_c;
}

/******************************************************************************
*******************************************************************************
* Private Debug stuff
*******************************************************************************
******************************************************************************/
/*****************************************************************************
* ZigBee Application.
*
* (c) Copyright 2005, Freescale, Inc. All rights reserved.
*
*
* No part of this document must be reproduced in any form - including copied,
* transcribed, printed or by any electronic means - without specific written
* permission from Freescale Semiconductor.
*
*****************************************************************************/
#include "EmbeddedTypes.h"
#include "ZigBee.h"
#include "Beecommon.h"
#include "BeeStack_Globals.h"
#include "BeeStackConfiguration.h"
#include "AppZdoInterface.h"
#include "ZtcInterface.h"
#if gBeeStackIncluded_d
#include "ZdoApsInterface.h"
#include "ZdoMain.h"
#include "ZdoVariables.h"
#endif
#include "ZDOStateMachineHandler.h"
#include "ZdpManager.h"
#include "EndPointConfig.h"
#include "ZclOptions.h"
#include "BeeAppInit.h"
#include "ASL_ZdpInterface.h"
#include "ASL_UserInterface.h"
#ifdef gHostApp_d
#include "ZtcHandler.h"
#endif
#if (gZclEnableOTAClient_d)
#include "ZclOta.h"
#endif
#if gMaxNoOfESISupported_c > 1
#include "ZclSe.h"
#endif
#include "EzCommissioning.h"
#include "ZdpFrequencyAgility.h"
#include "ZigbeeTask.h"
#if gInterPanCommunicationEnabled_c && gASL_EnableZllTouchlinkCommissioning_d
#include "ZllCommissioning.h"
#endif
/******************************************************************************
*******************************************************************************
* Private macros
*******************************************************************************
******************************************************************************/
/******************************************************************************
*******************************************************************************
* Public prototypes
*******************************************************************************
******************************************************************************/
/******************************************************************************
*******************************************************************************
* Private prototypes
*******************************************************************************
******************************************************************************/
/******************************************************************************
*******************************************************************************
* Private type definitions
*******************************************************************************
******************************************************************************/
/* None */
/******************************************************************************
*******************************************************************************
* Private memory declarations
*******************************************************************************
******************************************************************************/
/******************************************************************************
*******************************************************************************
* Public memory declarations
*******************************************************************************
******************************************************************************/
#if( gEndDevCapability_d || gComboDeviceCapability_d)
//zbCounter_t gFailureCounter = 0;
#endif
//extern ZDPCallBack_t gpZdpAppCallBackPtr;
//extern zbCounter_t gZdpSequence;
extern zbCounter_t gZdpResponseCounter;
/******************************************************************************
*******************************************************************************
* Public functions
*******************************************************************************
******************************************************************************/
#if gBeeStackIncluded_d
/*------------------- ZDP_APP_SapHandler --------------------
	Handles messages coming from ZDP to the Application
	IN: pMsg Message from ZDP to app
*/
uint8_t ZDP_APP_SapHandler(zdpToAppMessage_t *pMsg) {
#ifndef gHostApp_d
  /* For Host application , all received SAP messages are forward 
     to Host(if Host uart communication is enabled) by ZTC */
  ZTC_TaskEventMonitor( gZDPAppSAPHandlerId_c, ( uint8_t* ) pMsg,gZbSuccess_c );
#endif
  /* discover confirm complete, free the descriptor */
  if (pMsg->msgType == gNlmeNetworkDiscoveryConfirm_c)
  {
    if (pMsg->msgData.networkDiscoveryConf.pNetworkDescriptor != NULL)
    {
      MSG_Free( pMsg->msgData.networkDiscoveryConf.pNetworkDescriptor );
    }
  }
#ifndef gHostApp_d
  /* ZEDs must handle the poll error confirm, if reaches  */
#if gEndDevCapability_d || gComboDeviceCapability_d
#if gComboDeviceCapability_d
  if(NlmeGetRequest(gDevType_c) == gEndDevice_c)
#endif
  {
    if (pMsg->msgType == gSyncConf_c)
    {
      /*
        Due to ZEDs only talks or communicates through the parent, we concider each failing
        Tx as a failure to communicate with the parent, so we reuse the counter for FA as
        a code saving, because ZED do not participate in FA procedures directly. After an
        specific limit, the parent is considered as lost, and the zed must do a Nwk Rejoin.
      */
      if(pMsg->msgData.syncConf.status != gZbSuccess_c)
      {
        IncrementNwkTxTotalFailures();
        if (NlmeGetRequest(gNwkTxTotalFailures_c) >= NlmeGetRequest(gNwkLinkRetryThreshold_c))
        {
          if (ZDO_IsRunningState())
          {
            /* For Tx failures Rejoin must be done with the RAM info.  */
            ZDO_ProcessDeviceNwkRejoin();
            /* Save on NVM that we are orphans */
          }
          /* Reset the counter. */
          ResetTxCounters();
        }
      } 
      else
      {
        ResetTxCounters();
      }
    }
  }/* Combo device */
#endif /*( gEndDevCapability_d  || gComboDeviceCapability_d )*/
#endif /* gHostApp_d */
  /* Send the message up to the application */
  if( ZbZdoPublicData(gpZdpAppCallBackPtr) )
  {
#ifdef gHostApp_d
/* This allowed to send messages over the Uart */
  extern bool_t mMsgOriginIsZtc;
  mMsgOriginIsZtc = FALSE;
#endif  
    ZbZdoPublicData(gpZdpAppCallBackPtr( pMsg, ZbZdoPrivateData(gZdpResponseCounter) )); 
  }
  /* application will not see this message */
  else
  {
    /*
      If the call back is not registed and the message is a Energy scan confirm we
      should free the internal list.
    */
    if (pMsg->msgType == gNlmeEnergyScanConfirm_c)
      MSG_Free(pMsg->msgData.energyScanConf.resList.pEnergyDetectList);
    /*
      WARNING: This line must not be removed.
      Free the message received form the lower layers.
    */
    MSG_Free( pMsg );
  }
  return gZbSuccess_c;
}
#endif

/*==============================================================================================
	======================== Device and Service Discovery Client Services ========================
	==============================================================================================*/
/*-------------------- ASL_NWK_addr_req --------------------
	ClusterID=0x0000.
	This functions genertes a ZDP NWK_addr_req, and pass it to the
	ZDO layer thru the APP_ZDP_SapHandler function.
	Is generated from a Local Device wishing to inquire as to the
	16 bit address of the Remote Device based on its known IEEE address. The
	destination addressing on this command shall be broadcast to all RxOnWhenIdle
	devices.
	NOTE: The memory allocated for the message will be freed byt he lower layers.
*/
void ASL_NWK_addr_req
(
	zbCounter_t  *pSequenceNumber,  /* IN: The sequence number used to send the request. */
	zbNwkAddr_t  aDestAddress,      /* IN: The destination address where to send the request. */
	zbIeeeAddr_t  aIeeeAddr,        /* IN: The IEEE address to be matched by the Remote Device */
	uint8_t  requestType,           /* IN: Request type for this command: 0x00 ?Single device response
																																				0x01 ?Extended response
																																				0x02-0xFF ?reserved */
	index_t  startIndex             /* IN: If the Request type for this command is Extended response,
																				the StartIndex provides the starting index for the requested
																				elements of the associated devices list */
)
{
#if gNWK_addr_req_d
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbNwkAddrRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gNWK_addr_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy8Bytes(pAppZdoMsg->msgData.nwkAddressReq.aIeeeAddr, aIeeeAddr);
	pAppZdoMsg->msgData.nwkAddressReq.requestType = requestType;
	pAppZdoMsg->msgData.nwkAddressReq.startIndex = startIndex;
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
#else
(void)pSequenceNumber;
(void)aDestAddress;
(void)aIeeeAddr;
(void)requestType;
(void)startIndex;
#endif
}

#if gIEEE_addr_req_d
void ASL_IEEE_addr_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbNwkAddr_t aNwkAddrOfInterest,
	uint8_t  requestType,
	index_t  startIndex
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbIeeeAddrRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gIEEE_addr_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.extAddressReq.aNwkAddrOfInterest, aNwkAddrOfInterest);
	pAppZdoMsg->msgData.extAddressReq.requestType = requestType;
	pAppZdoMsg->msgData.extAddressReq.startIndex = startIndex;
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gNode_Desc_req_d
void ASL_Node_Desc_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbNodeDescriptorRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gNode_Desc_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.nodeDescriptorReq.aNwkAddrOfInterest, aDestAddress);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gPower_Desc_req_d
void ASL_Power_Desc_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbPowerDescriptorRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gPower_Desc_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.powDescriptorReq.aNwkAddrOfInterest, aDestAddress);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gExtended_Simple_Desc_req_d
void ASL_Extended_Simple_Desc_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbEndPoint_t  endPoint,
	index_t startIndex
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbExtSimpleDescriptorRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gExtended_Simple_Desc_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.extendedSimpleDescriptorReq.aNwkAddrOfInterest, aDestAddress);
	pAppZdoMsg->msgData.extendedSimpleDescriptorReq.endPoint = endPoint;
	pAppZdoMsg->msgData.extendedSimpleDescriptorReq.startIndex = startIndex;
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gSimple_Desc_req_d
void ASL_Simple_Desc_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbEndPoint_t  endPoint
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbSimpleDescriptorRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gSimple_Desc_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.simpleDescriptorReq.aNwkAddrOfInterest, aDestAddress);
	pAppZdoMsg->msgData.simpleDescriptorReq.endPoint = endPoint;
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gActive_EP_req_d
void ASL_Active_EP_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbActiveEpRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gActive_EP_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.activeEpReq.aNwkAddrOfInterest, aDestAddress);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gExtended_Active_EP_req_d
void ASL_Extended_Active_EP_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	index_t startIndex
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbExtActiveEpRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gExtended_Active_EP_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.extendedActiveEpReq.aNwkAddrOfInterest, aDestAddress);
  pAppZdoMsg->msgData.extendedActiveEpReq.startIndex = startIndex;
  
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gMatch_Desc_req_d
void ASL_MatchDescriptor_req
(
  zbCounter_t  *pSequenceNumber,
  zbNwkAddr_t  aDestAddress,
  zbSimpleDescriptor_t  *pSimpleDescriptor
)
{
  appToZdpMessage_t  *pAppZdoMsg;
  zbSize_t iSizeOfSimpleDescriptor;
  /* allocate a message for building the request */
  pAppZdoMsg = AF_MsgAlloc();
  if (!pAppZdoMsg)
    return;
  if (pSequenceNumber)
    pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
  pAppZdoMsg->msgType = gMatch_Desc_req_c;
  Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
  Copy2Bytes(pAppZdoMsg->msgData.matchDescriptorRequest.aNwkAddrOfInterest,aDestAddress);
  Copy2Bytes(pAppZdoMsg->msgData.matchDescriptorRequest.aProfileId, pSimpleDescriptor->aAppProfId);
  /* The size of both counters ... */
  iSizeOfSimpleDescriptor = sizeof(pSimpleDescriptor->appNumInClusters) * 2;
  iSizeOfSimpleDescriptor = (pSimpleDescriptor->appNumInClusters * sizeof(zbClusterId_t)) +  /* The amount of clusters in the input list. */
                            (pSimpleDescriptor->appNumOutClusters * sizeof(zbClusterId_t)) + /* The amnount of clusters in the output list. */
                            (sizeof(pSimpleDescriptor->pAppInClusterList) * 2);               /* The sise of both pionters, input list pointer and out put list poiunter. */
  /* Get all the info into the packet.! */
  FLib_MemCpy(&pAppZdoMsg->msgData.matchDescriptorRequest.cNumClusters, &pSimpleDescriptor->appNumInClusters, iSizeOfSimpleDescriptor);
  if(APP_ZDP_SapHandler(pAppZdoMsg))
  {
    /* insert code here to handle error  */
  }
}
#endif
#if gComplex_Desc_req_d
void ASL_Complex_Desc_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbComplexDescriptorRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gComplex_Desc_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.complexDescReq.aNwkAddrOfInterest, aDestAddress);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gUser_Desc_req_d
void ASL_User_Desc_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbUserDescriptorRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gUser_Desc_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.complexDescReq.aNwkAddrOfInterest, aDestAddress);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gDiscovery_Cache_req_d
void ASL_Discovery_Cache_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbNwkAddr_t  aNwkAddrOfInterest,
	zbIeeeAddr_t  aIEEEAddrOfInterest
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbDiscoveryCacheRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gDiscovery_Cache_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.discoveryCacheReq.aNwkAddress, aNwkAddrOfInterest);
	Copy8Bytes(pAppZdoMsg->msgData.discoveryCacheReq.aIeeeAddress, aIEEEAddrOfInterest);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if  gDevice_annce_d
void ASL_Device_annce
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbNwkAddr_t   aNwkAddress,
	zbIeeeAddr_t  aIeeeAddress,
	macCapabilityInfo_t  capability
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbEndDeviceAnnounce_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gDevice_annce_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.endDeviceAnnce.aNwkAddress, aNwkAddress);
	Copy8Bytes(pAppZdoMsg->msgData.endDeviceAnnce.aIeeeAddress,aIeeeAddress);
	pAppZdoMsg->msgData.endDeviceAnnce.capability = capability;
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gUser_Desc_set_d
void ASL_User_Desc_set
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbSize_t  length,
	zbUserDescriptor_t  aUserDescription
)
{
		appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbUserDescriptorSet_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gUser_Desc_set_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.userDescSet.aNwkAddrOfInterest, aDestAddress);
	pAppZdoMsg->msgData.userDescSet.descriptor.length = length;
	FLib_MemCpy(pAppZdoMsg->msgData.userDescSet.descriptor.aUserDescription,
							aUserDescription,
							sizeof(zbUserDescriptor_t));
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gSystem_Server_Discovery_req_d
void ASL_System_Server_Discovery_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbServerMask_t  aServerMask
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbSystemServerDiscoveryRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gSystem_Server_Discovery_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.systemServerDiscReq.aServerMask, aServerMask);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gDiscovery_store_req_d
void ASL_Discovery_store_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbDiscoveryStoreRequest_t  *pDiscoveryStore
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) +
													MbrOfs(zbDiscoveryStoreRequest_t,simpleDescriptorList[0]) +
													pDiscoveryStore->simpleDescriptorCount);
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gDiscovery_store_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	FLib_MemCpy(&pAppZdoMsg->msgData.discoveryStoreReq,
							pDiscoveryStore,
							MbrOfs(zbDiscoveryStoreRequest_t,simpleDescriptorList[0]));
	FLib_MemCpy(pAppZdoMsg->msgData.discoveryStoreReq.simpleDescriptorList,
							pDiscoveryStore->simpleDescriptorList,
							pDiscoveryStore->simpleDescriptorCount);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gNode_Desc_store_req_d
void ASL_Node_Desc_store_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbNodeDescriptor_t  *pNodeDescriptor
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbNodeDescriptorStoreRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gNode_Desc_store_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	FLib_MemCpy(&pAppZdoMsg->msgData.nodeDescriptorStoreReq,
							pNodeDescriptor,
							sizeof(zbNodeDescriptorStoreRequest_t));
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gPower_Desc_store_req_d
void ASL_Power_Desc_store_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbPowerDescriptor_t  *pPowerDescriptor
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbPowerDescriptorStoreRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gPower_Desc_store_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	FLib_MemCpy(&pAppZdoMsg->msgData.powerDescriptorStoreReq,
							pPowerDescriptor,
							sizeof(zbPowerDescriptorStoreRequest_t));
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gActive_EP_store_req_d
void ASL_Active_EP_store_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbCounter_t  activeEPcount,
	zbEndPoint_t  *pActiveEPList
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) +
													MbrOfs(zbActiveEPStoreRequest_t, activeEPList[0]) + activeEPcount);
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gActive_EP_store_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	pAppZdoMsg->msgData.activeEPStoreReq.activeEPCount = activeEPcount;
	FLib_MemCpy(pAppZdoMsg->msgData.activeEPStoreReq.activeEPList,pActiveEPList,activeEPcount);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gSimple_Desc_store_req_d
void ASL_Simple_Desc_store_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbSimpleDescriptorStoreRequest_t  *pSimpleDescStore
)
{
	appToZdpMessage_t *pAppZdoMsg;
	uint8_t  *pPtr;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) +
													MbrOfs(zbSimpleDescriptorStoreRequest_t, simpleDescriptor.inputClusters) +
													pSimpleDescStore->simpleDescriptor.inputClusters.cNumClusters * sizeof(zbClusterId_t) +
													pSimpleDescStore->simpleDescriptor.outputClusters.cNumClusters * sizeof(zbClusterId_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gSimple_Desc_store_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	FLib_MemCpy(&pAppZdoMsg->msgData.simpleDescriptorStoreReq,
							pSimpleDescStore,
							MbrOfs(zbSimpleDescriptorStoreRequest_t, simpleDescriptor.inputClusters));
	pAppZdoMsg->msgData.simpleDescriptorStoreReq.simpleDescriptor.inputClusters.cNumClusters = pSimpleDescStore->simpleDescriptor.inputClusters.cNumClusters;
	pAppZdoMsg->msgData.simpleDescriptorStoreReq.simpleDescriptor.outputClusters.cNumClusters = pSimpleDescStore->simpleDescriptor.outputClusters.cNumClusters;
	pPtr = (uint8_t *)(((uint8_t *)&pAppZdoMsg->msgData.simpleDescriptorStoreReq) + sizeof(zbSimpleDescriptorStoreRequest_t));
	FLib_MemCpy(pPtr, pSimpleDescStore->simpleDescriptor.inputClusters.pClusterList,
							pSimpleDescStore->simpleDescriptor.inputClusters.cNumClusters * sizeof(zbClusterId_t));
	pAppZdoMsg->msgData.simpleDescriptorStoreReq.simpleDescriptor.inputClusters.pClusterList = (zbClusterId_t *)pPtr;
	pPtr += (pSimpleDescStore->simpleDescriptor.inputClusters.cNumClusters * sizeof(zbClusterId_t));
	FLib_MemCpy(pPtr, pSimpleDescStore->simpleDescriptor.outputClusters.pClusterList,
							pSimpleDescStore->simpleDescriptor.outputClusters.cNumClusters * sizeof(zbClusterId_t));
	pAppZdoMsg->msgData.simpleDescriptorStoreReq.simpleDescriptor.outputClusters.pClusterList = (zbClusterId_t *)pPtr;
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gRemove_node_cache_req_d
void ASL_Remove_node_cache_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbNwkAddr_t  aNwkAddress,
	zbIeeeAddr_t  aIeeeAddress
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbRemoveNodeCacheRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gRemove_node_cache_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.removeNodeCacheReq.aNwkAddress, aNwkAddress);
	Copy8Bytes(pAppZdoMsg->msgData.removeNodeCacheReq.aIeeeAddress, aIeeeAddress);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gFind_node_cache_req_d
void ASL_Find_node_cache_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbNwkAddr_t  aNwkAddress,
	zbIeeeAddr_t  aIeeeAddress
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbFindNodeCacheRequest_t));
	if (pAppZdoMsg == NULL)
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gFind_node_cache_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.findNodeCacheReq.aNwkAddrOfInterest, aNwkAddress);
	Copy8Bytes(pAppZdoMsg->msgData.findNodeCacheReq.aIeeeAddrOfInterest, aIeeeAddress);
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
/*==============================================================================================
	======== End Device Bind, Bind, Unbind and Bind Management Client Services Primitives ========
	==============================================================================================*/
#if gEnd_Device_Bind_req_d
void ASL_EndDeviceBindRequest
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbSimpleDescriptor_t *pSimpleDescriptor
)
{
	appToZdpMessage_t *pAppZdoMsg;
	uint8_t * tmp_ptr;
	/* assign memory to the EndDeviceBindRequest	messenge (special case)*/
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) +
													sizeof(zbEndDeviceBindRequest_t)+
													((pSimpleDescriptor->appNumInClusters*sizeof(zbClusterId_t))+
													(pSimpleDescriptor->appNumOutClusters*sizeof(zbClusterId_t))));
	if (pAppZdoMsg == NULL)
		return;
	if( pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gEnd_Device_Bind_req_c; /* EndDeviceBindRequest type message */
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.endDeviceBindReq.aBindingTarget,NlmeGetRequest(gNwkShortAddress_c));
	Copy2Bytes(pAppZdoMsg->msgData.endDeviceBindReq.aProfileId, pSimpleDescriptor->aAppProfId);
	Copy8Bytes(pAppZdoMsg->msgData.endDeviceBindReq.aSrcIeeeAddress,NlmeGetRequest(gNwkIeeeAddress_c));
	pAppZdoMsg->msgData.endDeviceBindReq.srcEndPoint = pSimpleDescriptor->endPoint;
	pAppZdoMsg->msgData.endDeviceBindReq.inputClusterList.cNumClusters = pSimpleDescriptor->appNumInClusters;
	pAppZdoMsg->msgData.endDeviceBindReq.outputClusterList.cNumClusters = pSimpleDescriptor->appNumOutClusters;
	/* Copying the InClusterList to tmp_ptr, just the list not the counters */
	tmp_ptr = (uint8_t *)pAppZdoMsg + sizeof(zbMsgId_t)+sizeof(zbNwkAddr_t)+sizeof(zbEndDeviceBindRequest_t);
	FLib_MemCpy( tmp_ptr, pSimpleDescriptor->pAppInClusterList,(pSimpleDescriptor->appNumInClusters * sizeof(zbClusterId_t)));
	pAppZdoMsg->msgData.endDeviceBindReq.inputClusterList.pClusterList = (zbClusterId_t *) tmp_ptr;
	/* Copying OutClusterList to tmp_ptr, just the list not the counters */
	tmp_ptr= (uint8_t *) pAppZdoMsg->msgData.endDeviceBindReq.inputClusterList.pClusterList + (pSimpleDescriptor->appNumInClusters * sizeof(zbClusterId_t));
	FLib_MemCpy( tmp_ptr, pSimpleDescriptor->pAppOutClusterList, (pSimpleDescriptor->appNumOutClusters*sizeof(zbClusterId_t)));
	pAppZdoMsg->msgData.endDeviceBindReq.outputClusterList.pClusterList = (zbClusterId_t*)tmp_ptr;
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#endif
#if gBind_req_d || gUnbind_req_d
void APP_ZDP_BindUnbindRequest
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbMsgId_t  BindUnbind,
	zbBindUnbindRequest_t  *pBindUnBindRequest
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbBindUnbindRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = BindUnbind;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	/* Filling up self IEEEAddress */
	Copy8Bytes(pAppZdoMsg->msgData.bindReq.aSrcAddress,pBindUnBindRequest->aSrcAddress);
	pAppZdoMsg->msgData.bindReq.srcEndPoint = pBindUnBindRequest->srcEndPoint;
	/* Filling up self ClusterID */
	Copy2Bytes(pAppZdoMsg->msgData.bindReq.aClusterId,pBindUnBindRequest->aClusterId);
	/* Filling up self use indirect address mode to read binding table */
	pAppZdoMsg->msgData.bindReq.addressMode = pBindUnBindRequest->addressMode;
	if (pAppZdoMsg->msgData.bindReq.addressMode == zbGroupMode)
		Copy2Bytes(pAppZdoMsg->msgData.bindReq.destData.groupMode.aDstaddress,pBindUnBindRequest->destData.groupMode.aDstaddress);
	else
	{
		Copy8Bytes(pAppZdoMsg->msgData.bindReq.destData.extendedMode.aDstAddress, pBindUnBindRequest->destData.extendedMode.aDstAddress);
		pAppZdoMsg->msgData.bindReq.destData.extendedMode.dstEndPoint = pBindUnBindRequest->destData.extendedMode.dstEndPoint;
	}
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
void APP_ZDP_RequestKeyRequest
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbApsmeRequestKeyReq_t  *pApsmeRequestKey
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbApsmeRequestKeyReq_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gZdoApsmeRequestKeyReq_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr, aDestAddress);
        
	/* Filling up self and partner IEEEAddress */
	Copy8Bytes(pAppZdoMsg->msgData.requestKeyReq.aDestLongAddr, pApsmeRequestKey->aDestAddress);
	Copy8Bytes(pAppZdoMsg->msgData.requestKeyReq.aPartnerLongAddr, pApsmeRequestKey->aPartnerAddress);
        
        pAppZdoMsg->msgData.requestKeyReq.keyType = pApsmeRequestKey->keyType;
        
        if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}

#if gBind_Register_req_d
void ASL_Bind_Register_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbIeeeAddr_t  aNodeAddress
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbBindRegisterRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gBind_Register_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.bindRegisterReq.aIeeeAddress, aNodeAddress);
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gReplace_Device_req_d
void ASL_Replace_Device_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbNwkAddr_t  aOldAddress,
	zbEndPoint_t  oldEndPoint,
	zbNwkAddr_t  aNewAddress,
	zbEndPoint_t  newEndPoint
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbReplaceDeviceRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gReplace_Device_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	Copy2Bytes(pAppZdoMsg->msgData.replaceDeviceReq.aOldAddress, aOldAddress);
	Copy2Bytes(pAppZdoMsg->msgData.replaceDeviceReq.aNewAddress, aNewAddress);
	pAppZdoMsg->msgData.replaceDeviceReq.oldEndPoint = oldEndPoint;
	pAppZdoMsg->msgData.replaceDeviceReq.newEndPoint = newEndPoint;
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gStore_Bkup_Bind_Entry_req_d
void ASL_Store_Bkup_Bind_Entry_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbStoreBkupBindEntryRequest_t  *pStoreBkupEntry
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbStoreBkupBindEntryRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gStore_Bkup_Bind_Entry_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	FLib_MemCpy(&pAppZdoMsg->msgData.storeBackupBindingEntryReq, pStoreBkupEntry, sizeof(zbStoreBkupBindEntryRequest_t));
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gRemove_Bkup_Bind_Entry_req_d
void ASL_Remove_Bkup_Bind_Entry_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbRemoveBackupBindEntryRequest_t  *pRemoveBkupEntry
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbRemoveBackupBindEntryRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gRemove_Bkup_Bind_Entry_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	FLib_MemCpy(&pAppZdoMsg->msgData.removeBackupBindingEntryReq, pRemoveBkupEntry, sizeof(zbRemoveBackupBindEntryRequest_t));
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gBackup_Bind_Table_req_d
void ASL_Backup_Bind_Table_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbBackupBindTableRequest_t  *pBackupBindTable
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) +
													MbrOfs(zbBackupBindTableRequest_t, BindingTableList[0]) +
													(OTA2Native16(TwoBytesToUint16(pBackupBindTable->BindingTableListCount)) *
													MbrSizeof(zbBackupBindTableRequest_t, BindingTableList[0])));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gBackup_Bind_Table_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	FLib_MemCpy(&pAppZdoMsg->msgData.backupBindTableReq,
							pBackupBindTable,
							(MbrOfs(zbBackupBindTableRequest_t, BindingTableList[0]) +
							(OTA2Native16(TwoBytesToUint16(pBackupBindTable->BindingTableListCount)) *
							MbrSizeof(zbBackupBindTableRequest_t, BindingTableList[0]))));
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gRecover_Bind_Table_req_d
void ASL_Recover_Bind_Table_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	index_t  index
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbRecoverBindTableRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gRecover_Bind_Table_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	Set2Bytes(pAppZdoMsg->msgData.recoverBindTableReq.startIndex, index);
#if (gBigEndian_c)       
	Swap2BytesArray(pAppZdoMsg->msgData.recoverBindTableReq.startIndex);
#endif       
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gBackup_Source_Bind_req_d
void ASL_Backup_Source_Bind_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbBackupSourceBindRequest_t  *pBkupSourceBindTable
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) +
													MbrOfs(zbBackupSourceBindRequest_t, SourceTableList[0]) +
													(TwoBytesToUint16(pBkupSourceBindTable->SourceTableListCount) *
													MbrSizeof(zbBackupSourceBindRequest_t, SourceTableList[0])));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gBackup_Source_Bind_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	FLib_MemCpy(&pAppZdoMsg->msgData.backupSourceBindReq,
							pBkupSourceBindTable,
							(MbrOfs(zbBackupSourceBindRequest_t, SourceTableList[0]) +
							(TwoBytesToUint16(pBkupSourceBindTable->SourceTableListCount) *
							MbrSizeof(zbBackupSourceBindRequest_t, SourceTableList[0]))));
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gRecover_Source_Bind_req_d
void ASL_Recover_Source_Bind_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	index_t  index
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbRecoverSourceBindRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gRecover_Source_Bind_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	Set2Bytes(pAppZdoMsg->msgData.recoverSourceBindReq.StartIndex, index);
#if (gBigEndian_c)        
	Swap2BytesArray(pAppZdoMsg->msgData.recoverSourceBindReq.StartIndex);
#endif
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
/*==============================================================================================
	============================= Network Management Client Services =============================
	==============================================================================================*/
#if gMgmt_NWK_Disc_req_d
void ASL_Mgmt_NWK_Disc_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbChannels_t  aScanChannel,
	zbCounter_t  scanDuration,
	index_t  startIndex
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtNwkDiscoveryRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gMgmt_NWK_Disc_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	FLib_MemCpy(pAppZdoMsg->msgData.mgmtNwkDiscReq.aScanChannel, aScanChannel, sizeof(zbChannels_t));
	pAppZdoMsg->msgData.mgmtNwkDiscReq.scanDuration = scanDuration;
	pAppZdoMsg->msgData.mgmtNwkDiscReq.startIndex = startIndex;
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gMgmt_Lqi_req_d
void ASL_Mgmt_Lqi_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	index_t  index
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtLqiRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gMgmt_Lqi_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	pAppZdoMsg->msgData.mgmtLqiReq.startIndex = index;
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gMgmt_Rtg_req_d
void ASL_Mgmt_Rtg_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	index_t  index
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtRtgRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gMgmt_Lqi_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	pAppZdoMsg->msgData.mgmtRtgReq.startIndex = index;
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gMgmt_Bind_req_d
void ASL_Mgmt_Bind_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	index_t  index
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtBindRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gMgmt_Bind_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	pAppZdoMsg->msgData.mgmtBindReq.startIndex = index;
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gMgmt_Leave_req_d
void ASL_Mgmt_Leave_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbIeeeAddr_t aDeviceAddres,
	zbMgmtOptions_t  mgmtOptions
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtLeaveRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gMgmt_Leave_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	pAppZdoMsg->msgData.mgmtLeaveReq.mgmtOptions = mgmtOptions;
	Copy8Bytes(pAppZdoMsg->msgData.mgmtLeaveReq.aDeviceAddress, aDeviceAddres);
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif

#if gMgmt_Permit_Joining_req_d
void ASL_Mgmt_Permit_Joining_req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	zbCounter_t  permitDuration,
	uint8_t  TC_Significance
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtPermitJoiningRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gMgmt_Permit_Joining_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	pAppZdoMsg->msgData.mgmtPermitJoiningReq.PermitDuration = permitDuration;
	pAppZdoMsg->msgData.mgmtPermitJoiningReq.TC_Significance = TC_Significance;
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gMgmt_Cache_req_d
void ASL_Mgmt_Cache_Req
(
	zbCounter_t  *pSequenceNumber,
	zbNwkAddr_t  aDestAddress,
	index_t  index
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtCacheRequest_t));
	if( pAppZdoMsg == NULL )
		return;
	if (pSequenceNumber != NULL)
		pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
	pAppZdoMsg->msgType = gMgmt_Cache_req_c;
	Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
	pAppZdoMsg->msgData.mgmtCacheReq.startIndex = index;
	if(APP_ZDP_SapHandler( pAppZdoMsg ))
	{
		/* Insert code here to handle error */
	}
}
#endif
#if gMgmt_Direct_Join_req_d && (gCoordinatorCapability_d || gRouterCapability_d || gComboDeviceCapability_d)
void ASL_Mgmt_Direct_Join_req
(
  zbCounter_t  *pSequenceNumber,
  zbNwkAddr_t  aDestinationAddress,
  zbIeeeAddr_t  aDeviceAddres,
  uint8_t       capabilityInfo
)
{
  /* The message to pass down to the sap handler. */
  appToZdpMessage_t *pAppZdoMsg;
  /* The pointer to be use to fill up the message. */
  zbMgmtDirectJoinRequest_t  *pDirectJoin;
#if gComboDeviceCapability_d
  if (NlmeGetRequest(gDevType_c) == gEndDevice_c)
  {
    return;
  }
#endif
  /* Allocate the message to be sended. */
  pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtDirectJoinRequest_t));
  if (pAppZdoMsg == NULL)
    return;
  if (pSequenceNumber != NULL)
    pSequenceNumber[0] = ZbZdoPrivateData(gZdpSequence);
  /* Set the address where the message is going to. */
  Copy2Bytes(pAppZdoMsg->aDestAddr, aDestinationAddress);
  /* Point to the plce in memory wher to fill the message. */
  pDirectJoin = (zbMgmtDirectJoinRequest_t *)&pAppZdoMsg->msgData.mgmtDirectJoinReq;
  /* The IEEE Address of the receiving device. */
  Copy8Bytes(pDirectJoin->aDeviceAddress, aDeviceAddres);
  /* The capability information to be used by the receiving device. */
  pDirectJoin->capabilityInformation = capabilityInfo;
  if(APP_ZDP_SapHandler( pAppZdoMsg ))
  {
    /* Insert code here to handle error */
  }
}
#endif
void APP_ZDP_PermitJoinRequest
(
	uint8_t  permit /* IN: Duration */
)
{
	appToZdpMessage_t *pAppZdoMsg;
	pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtDirectJoinRequest_t));
	if(pAppZdoMsg == NULL)
		return;
	pAppZdoMsg->msgType = gPermitJoinReq_c;
	pAppZdoMsg->msgData.permitJoinReq.permitDuration = permit;
	if(APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
void ASL_Nlme_Sync_req
(
	bool_t track
)
{
	appToZdpMessage_t  *pAppZdoMsg;
	uint8_t *pPtr;
	pAppZdoMsg = MSG_Alloc(MbrOfs(appToNwkMessage_t,msgData) + sizeof(zdoNwkSyncReq_t));
	if (pAppZdoMsg == NULL)
		return;
	pAppZdoMsg->msgType = gZdoNwkSyncReq_c;
//	Set2Bytes(pAppZdoMsg->aDestAddr, 0x0000); /* address shouldn't be needed for local calls */
	pPtr = (uint8_t *)&pAppZdoMsg->msgData;
	*pPtr = track;
	if (APP_ZDP_SapHandler ( pAppZdoMsg ))
	{
		/* Insert code here to handle error  */
	}
}
#if (gMgmt_NWK_Update_req_d && ( gCoordinatorCapability_d || gRouterCapability_d || gComboDeviceCapability_d))&& (gNetworkManagerCapability_d || gComboDeviceCapability_d)
void ASL_Mgmt_NWK_Update_req
(
  zbNwkAddr_t   aDestAddress,
  zbChannels_t  aChannelList,
  uint8_t       iScanDuration,
  uintn8_t      iScanCount
)
{
  appToZdpMessage_t *pAppZdoMsg;
  zbMgmtNwkUpdateRequest_t* pmsgData;
  if (!ZdoGetNwkManagerBitMask(ZbZdoPublicData(gaServerMask)))
  {
    return;
  }

  pAppZdoMsg = MSG_Alloc( MbrOfs(appToZdpMessage_t, msgData) + sizeof(zbMgmtNwkUpdateRequest_t));
  if( pAppZdoMsg == NULL )
    return;
  pAppZdoMsg->msgType = gMgmt_NWK_Update_req_c;
  Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
  pmsgData =(zbMgmtNwkUpdateRequest_t*) &pAppZdoMsg->msgData;
  FLib_MemCpy(pmsgData->aScanChannels,
              aChannelList, sizeof (zbChannels_t));
  pmsgData->ScanDuration = iScanDuration;
  if (iScanDuration <=0x05)
    pmsgData->ExtraData.ScanCount= iScanCount;
  if (iScanDuration == 0xfe ||iScanDuration ==0xff)
    pmsgData->ExtraData.NwkManagerData.nwkUpdateId = iScanCount;
  if (iScanDuration ==0xff)
    Copy2Bytes(pmsgData->ExtraData.NwkManagerData.aNwkManagerAddr,NlmeGetRequest(gNwkShortAddress_c));
  if(APP_ZDP_SapHandler( pAppZdoMsg ))
  {
    /* Insert code here to handle error */
  }
}
#endif
/*gMgmt_NWK_Update_req_d*/
#if (gMgmt_NWK_Update_notify_d && ( gCoordinatorCapability_d || gRouterCapability_d || gComboDeviceCapability_d))&&(!gNetworkManagerCapability_d || gComboDeviceCapability_d)
void ASL_Mgmt_NWK_Update_Notify
(
  zbNwkAddr_t  aDestAddress,
  zbChannels_t  aScannedChannels,
  uint16_t      iTotalTransmissions,
  uint16_t      iTransmissionFailures,
  uint8_t       iScannedChannelListCount,
  zbEnergyValue_t  *paEnergyVslues,
  zbStatus_t    status
)
{
  appToZdpMessage_t *pAppZdoMsg;
  zbMgmtNwkUpdateNotify_t *pMsgData;
  if (ZdoGetNwkManagerBitMask(ZbZdoPublicData(gaServerMask)))
  {
    return;
  }
  pAppZdoMsg = MSG_Alloc(MbrOfs(appToZdpMessage_t, msgData) +
                         sizeof(zbMgmtNwkUpdateNotify_t) +
                         iScannedChannelListCount);
  if( pAppZdoMsg == NULL )
    return;
  pAppZdoMsg->msgType = gMgmt_NWK_Update_notify_c;
  Copy2Bytes(pAppZdoMsg->aDestAddr,aDestAddress);
  pMsgData = (zbMgmtNwkUpdateNotify_t*)&pAppZdoMsg->msgData;
  pMsgData->Status = status;
  
  FLib_MemCpy(pMsgData->ScannedChannels,aScannedChannels, sizeof(zbChannels_t));
  pMsgData->TotalTransmissions = Native2OTA16(iTotalTransmissions);
  pMsgData->TransmissionFailures = Native2OTA16(iTransmissionFailures);
  pMsgData->ScannedChannelsListCount = iScannedChannelListCount;
  if (paEnergyVslues)
  {
    FLib_MemCpy(pMsgData->aEnergyValues,paEnergyVslues,
               iScannedChannelListCount /* * MbrSizeof(zbMgmtNwkUpdateNotify_t, aEnergyValues)*/);
  }
  if(APP_ZDP_SapHandler( pAppZdoMsg ))
  {
    /* Insert code here to handle error */
  }
}
#endif
/*gMgmt_NWK_Update_notify_d*/
/*! @attention - Start ShowLink Specific Code
*   We need just this function, not the rest of freq agility...
*/
//#if gFrequencyAgilityCapability_d 
void ASL_ChangeChannel
(
  uint8_t  channelNumber
)
{
  zdoNlmeMessage_t  zdoMsg;
  zdoMsg.msgType = gNlmeSetChannelRequest_c;
  zdoMsg.msgData.SetChannelReq.Channel = channelNumber;
  if (ZDO_NLME_SapHandler(&zdoMsg))
  {
    /*
      Handle error here.
    */
  }
}
//#endif
/*! End ShowLink Specific Code */
/*gFrequencyAgilityCapability_d*/
/************************************************************************************
* Send a Many-To-One Nlme-Route-Discovery.req to all routers 0xFFFC, pass the confirm
* to the application using the ZDP_APP Sap handler. This function only require the 
* nwkConcentratorRadius and the NoRouteCache value.
*
* Interface assumptions:
*   The parameter iRadius, is the nwkConcentratorRadius.
*   
*
* Return value:
*   NONE.
*
************************************************************************************/
#if gConcentratorFlag_d
void ASL_SendRouteDiscoveryManyToOne
(
  uint8_t iRadius,
  bool_t  noRouteCacheFlag
)
{
  zdoNlmeMessage_t  *pZdoMsg;
  pZdoMsg = MSG_Alloc( sizeof( nlmeRouteDiscoveryReq_t )  + sizeof(zbMsgId_t));
  if (!pZdoMsg) 
    return;
  NlmeSetRequest(gNwkConcentratorRadius_c, iRadius);
  pZdoMsg->msgType = gNlmeRouteDiscReq_c;
  /* Many-to-one destination address mode is 0x00 */
	pZdoMsg->msgData.routeDiscoveryReq.dstAddrMode =	gZbAddrModeIndirect_c;
	Copy2Bytes(pZdoMsg->msgData.routeDiscoveryReq.aDstAddr,	gaBroadcastZCnZR);
  pZdoMsg->msgData.routeDiscoveryReq.noRouteCache = noRouteCacheFlag; 
    if (ZDO_NLME_SapHandler(pZdoMsg))
  {
    /*
      Handle error here.
    */
  }
}

      
#endif
/************************************************************************************
* Using the input values, generates an Nlme command to discover the ZigBee networks 
* that may exist on the current channel list (issues beacon requests and listens to 
* beacon responses).
*
* Interface assumptions:
*     aScanChannelMask 
*     scanDuration 
*
* Return value:
*   NONE.
*
************************************************************************************/
void ASL_GenerateNwkDiscoveryReq
(
  uint32_t     aScanChannelMask,
  uint8_t      scanDuration
)
{
  zdoNlmeMessage_t *pNwkMsg;
  zbChannels_t aChannelMask = {0x00, 0x00, 0x00, 0x00};
  /*
    We need to generate a message to Nwk layer, a buffer needs to be allocated.
  */
  pNwkMsg = MSG_AllocType( zdoNlmeMessage_t );
  /*
    For some reason we fail to allocate the memory, no further
    processing can be done.
  */
  if (!pNwkMsg)
    return;
  
  aChannelMask[0] = (( aScanChannelMask ) & 0xFF );
  aChannelMask[1] = (( aScanChannelMask >>  8 ) & 0xFF );
  aChannelMask[2] = (( aScanChannelMask >> 16 ) & 0xFF );
  aChannelMask[3] = (( aScanChannelMask >> 24 ) & 0xFF );
  
  /*
    Create the network descovery request and sent to the network Layer.
  */
  pNwkMsg->msgType = gNlmeNetworkDiscoveryRequest_c;
  pNwkMsg->msgData.networkDiscoveryReq.scanDuration = scanDuration;
  FLib_MemCpy(&pNwkMsg->msgData.networkDiscoveryReq.aScanChannels, aChannelMask, sizeof(zbChannels_t));
  /*
    Pass the command down to the Nwk layer.
  */
  if (ZDO_NLME_SapHandler( pNwkMsg ))
  {
    /*
      Catch the error if needed.
    */
  }
}
/************************************************************************************
* Using the input values, generates local an NLME-DIRECT-JOIN.Req
*
* Interface assumptions:
*     aDeviceAddress  - ieee address of the device to be directly joined          
*     capabilityInformation - operating capabilities
*
* Return value:
*   NONE.
*
************************************************************************************/
void ASL_GenerateDirectJoinReq
(
  zbIeeeAddr_t aDeviceAddress, 
  uint8_t      capabilityInformation
)
{
  zdoNlmeMessage_t *pNwkMsg;
  
#if gComboDeviceCapability_d
  if (NlmeGetRequest(gDevType_c) == gEndDevice_c)
  {
    return;
  }
#endif  
    
  /*
    We need to generate a message to Nwk layer, a buffer needs to be allocated.
  */
  pNwkMsg = MSG_AllocType( zdoNlmeMessage_t );
  /*
    For some reason we fail to allocate the memory, no further
    processing can be done.
  */
  if (!pNwkMsg)
    return;
  
  /*
    Create the NLME direct join request and sent to the network Layer.
  */
  pNwkMsg->msgType = gNlmeDirectJoinRequest_c;
  pNwkMsg->msgData.directJoinReq.capabilityInformation = capabilityInformation;
  Copy8Bytes(pNwkMsg->msgData.directJoinReq.aDeviceAddress, aDeviceAddress);
  /*
    Pass the command down to the Nwk layer.
  */
  if (ZDO_NLME_SapHandler( pNwkMsg ))
  {
    /*
      Catch the error if needed.
    */
  }
}
/*****************************************************************************
* ASL_ZdoCallBack
*
* Default key handling. Handles all config mode keys.
*****************************************************************************/
void ASL_ZdoCallBack
  (
  zdpToAppMessage_t *MsgFromZDP,
  zbCounter_t MsgCounter
  )
{
  uint8_t event;
  uint8_t appEvent;
  bool_t  sendEventToApp = FALSE;
  (void)MsgCounter; /* eliminate compiler warnings */
  /* return the event */
  event = MsgFromZDP->msgType;
  /*********************************************************/
  /* if event indication, message type is the zdo2AppEvent */
  /*********************************************************/
  if(event == gzdo2AppEventInd_c) 
  {
      /* get system event */
      appEvent = MsgFromZDP->msgData.zdo2AppEvent;
#ifdef gHostApp_d
      ZDO_SetState(appEvent);
#endif 
    
      /* process system events */
      if(appEvent == gZDOToAppMgmtZCRunning_c || appEvent == gZDOToAppMgmtZRRunning_c ||
        appEvent == gZDOToAppMgmtZEDRunning_c)   
      {
#ifdef gHcGenericApp_d    
#if gHcGenericApp_d 
        {
            zbClusterId_t  ClusterId={gaZclClusterGeneralTunnel_c};
            uint8_t  ProtocolAddress[sizeof(uint8_t) + sizeof(zbIeeeAddr_t)];
          
            ProtocolAddress[0] = sizeof(zbIeeeAddr_t);
            FLib_MemCpy(&ProtocolAddress[1], aExtendedAddress, sizeof(zbIeeeAddr_t));
            Swap8Bytes(&ProtocolAddress[1]);
            (void)ZCL_SetAttribute(EndPointConfigData(endPointList[0].pEndpointDesc->pSimpleDesc->endPoint),
                ClusterId, gZclAttrGTProtoAddr_c, gZclServerAttr_c, ProtocolAddress);    
         }    
#endif
#endif   
#if gASL_EnableEZCommissioning_d    	
         TS_SendEvent(gEzCmsTaskId, gDeviceInNetwork_c);  
#endif    
        
#ifdef gHostApp_d        
        ZdoSaveAllDataSetsLocal();
#endif        
#if gInterPanCommunicationEnabled_c && gASL_EnableZllTouchlinkCommissioning_d
      ZllTouchlink_ProcessEvent(appEvent, NULL);
#endif         
#if gZclEnableReporting_c
        ZCL_CheckAndStartAttrReporting(); 
#endif          
      }
    
      if(appEvent == gZDOToAppMgmtJoinNwkFailed_c)
      {
#if gInterPanCommunicationEnabled_c && gASL_EnableZllTouchlinkCommissioning_d
#if gComboDeviceCapability_d || gEndDevCapability_d
        if(ZllTouchlink_ProcessEvent(appEvent, NULL) != TRUE)
        {
#endif      
#endif        
#if gASL_EnableEZCommissioning_d && gComboDeviceCapability_d
          TS_SendEvent(gEzCmsTaskId, gJoiningfailed_c);
#endif
       
#if gInterPanCommunicationEnabled_c && gASL_EnableZllTouchlinkCommissioning_d
#if gComboDeviceCapability_d || gEndDevCapability_d
        }
#endif      
#endif         
             
      }    
    
      if(appEvent == gZDOToAppMgmtStopped_c)
    {
#if gASL_EnableEZCommissioning_d
          EZCommissioning_Reset();
#endif
#if gInterPanCommunicationEnabled_c && gASL_EnableZllTouchlinkCommissioning_d
          ZllTouchlink_ProcessEvent(appEvent, NULL);
#endif            
      }    
      
      
      MSG_Free(MsgFromZDP);
      /* display the results of the event */
      BeeAppUpdateDevice(BeeAppDataInit(appEndPoint), appEvent, 0, NULL, NULL); 
      return;
  }
  
  /*********************************************************/
  /*                    ZDO message types                  */
  /*********************************************************/  
  if (event == gZdoApsmeTranspKeyInd_c)
  {
    ApsmeSetRequest(gApsTrustCenterAddress_c, MsgFromZDP->msgData.transportKeyInd.aSrcLongAddr);
  }
  if(event == gEnd_Device_Bind_rsp_c) 
  {
        sendEventToApp = TRUE;
        appEvent = gBindingSuccess_c;
        gSendingNwkData.endPoint = MsgFromZDP->msgData.matchDescriptorResponse.matchList[0];
        if(MsgFromZDP->msgData.endDeviceBindResp.status != gZbSuccess_c )
            appEvent = gBindingFailure_c;
#ifdef gHaThermostat_d    
#if gHaThermostat_d    
        else 
            CreateTempMeasurementBinding();
#endif     
#endif
  }
  if(event == gBind_rsp_c) 
  {
        sendEventToApp = TRUE;
        appEvent = gBindingSuccess_c;
        if(MsgFromZDP->msgData.bindResp.status != gZbSuccess_c )
          appEvent = gBindingFailure_c;
  }
#if gMatch_Desc_req_d
  if (event == gMatch_Desc_rsp_c ) 
  {
        sendEventToApp = TRUE;
        if (MsgFromZDP->msgData.matchDescriptorResponse.status == gZbSuccess_c)
        {
            appEvent = gMatchDescriptorSuccess_c;
            gSendingNwkData.NwkAddrOfIntrest[0] = MsgFromZDP->msgData.matchDescriptorResponse.aNwkAddrOfInterest[0];
            gSendingNwkData.NwkAddrOfIntrest[1] = MsgFromZDP->msgData.matchDescriptorResponse.aNwkAddrOfInterest[1];
            gSendingNwkData.endPoint = MsgFromZDP->msgData.matchDescriptorResponse.matchList[0];
        }
        else if (MsgFromZDP->msgData.matchDescriptorResponse.status == gZdoDeviceNotFound_c)
            appEvent = gMatchNotFound_c;
        else
            appEvent = gMatchFailure_c;
  }
#endif
#if gIEEE_addr_req_d
  if (event == gIEEE_addr_rsp_c ) 
  {
        if (MsgFromZDP->msgData.extAddressResp.status == gZbSuccess_c)
        {
          
#if (gZclEnableOTAClient_d)
              OtaClient_ServerDiscoveryProcessIeeeAddrRsp(MsgFromZDP->msgData.extAddressResp.devResponse.singleDevResp.aIeeeAddrRemoteDev);	
#endif    	
              
#if gASL_EnableEZCommissioning_d && gASL_EnableEZCommissioning_Initiator_d 
              EZCommissioning_VerifyIEEEaddrRsp(MsgFromZDP->msgData.extAddressResp.devResponse.singleDevResp.aIeeeAddrRemoteDev);     
#endif    
              
              BeeAppUpdateDevice(BeeAppDataInit(appEndPoint), gIeeeAddrSuccess_c, 0, NULL, &MsgFromZDP->msgData.extAddressResp.devResponse.extendedDevResp);
              MSG_Free(MsgFromZDP);
              return;
        }
        else
        {
              sendEventToApp = TRUE;
              appEvent = gIeeeAddrFailed_c;
        }
  }
#endif  
  if (event == gNWK_addr_rsp_c ) 
  {  
        if (MsgFromZDP->msgData.nwkAddressResp.status == gZbSuccess_c)
        {
              BeeAppUpdateDevice(BeeAppDataInit(appEndPoint), gNwkAddrSuccess_c, 0, NULL, &MsgFromZDP->msgData.nwkAddressResp.devResponse.singleDevResp);
        }
        else
        {
              sendEventToApp = TRUE;
              appEvent = gNwkAddrFailed_c;          
        }
  }
  
  if (event == gNode_Desc_rsp_c ) 
  {  
        if (MsgFromZDP->msgData.nodeDescriptorResp.status == gZbSuccess_c)
        {
              BeeAppUpdateDevice(BeeAppDataInit(appEndPoint), gNodeDescSuccess_c, 0, NULL, &MsgFromZDP->msgData.nodeDescriptorResp);
        }
        else
        {
              sendEventToApp = TRUE;
              appEvent = gNodeDescFailed_c;                   
        }
  }
  
  if (event == gMgmt_NWK_Update_notify_c)
  {
        /* Already handle in ZDP. */
  }
  
  if (event == gSimple_Desc_rsp_c)
  {
#if gASL_EnableEZCommissioning_d && gASL_EnableEZCommissioning_Initiator_d
    if(EZCommissioningConfigData(gEZCommissioning_LastEvent) == gSendSimpleDescReq_c)
    {
      uint8_t length;
      zbSimpleDescriptorResponse_t* pSimpleDescRsp = &(MsgFromZDP->msgData.simpleDescriptorResp);
      length = sizeof(zbSimpleDescriptorResponse_t) - sizeof(zbSimpleDescriptorPackageResponse_t) + pSimpleDescRsp->length;
      if(EZCommissioningConfigData(gpEZCommissioningTempData))
      {
        FLib_MemCpy(EZCommissioningConfigData(gpEZCommissioningTempData), &MsgFromZDP->msgData.simpleDescriptorResp, length);
        TS_SendEvent(gEzCmsTaskId, gReceivedSimpleDescRsp_c);
      }
    }
#endif
  } 
  
  if(event == gDevice_annce_c)
  {
       BeeAppUpdateDevice(BeeAppDataInit(appEndPoint), gDeviceAnnceEvent_c, 0, NULL, &MsgFromZDP->msgData.endDeviceAnnce); 
       MSG_Free(MsgFromZDP);
       return;
  }
  
  
  /*********************************************************/
  /*                   Nwk  message types                  */
  /*********************************************************/  
  if (event == gNlmeTxReport_c)
  {
        FA_ProcessNlmeTxReport(&MsgFromZDP->msgData.nlmeNwkTxReport);
  }
  
  if (event == gNlmeEnergyScanConfirm_c)
  {
        FA_ProcessEnergyScanCnf(&MsgFromZDP->msgData.energyScanConf);
  }
  
  if (event == gChannelMasterReport_c)
  {
        FA_SelectChannelAndChange();
  }
  
  if (event == gNlmeNetworkDiscoveryConfirm_c)
  {    
        sendEventToApp = TRUE;
        appEvent = gDiscoveryConfirm_c;      
#if gInterPanCommunicationEnabled_c && gASL_EnableZllTouchlinkCommissioning_d
        if(ZllTouchlink_ProcessEvent(event, (uint8_t *)&MsgFromZDP->msgData.networkDiscoveryConf))
          sendEventToApp = FALSE;
#endif       
  }  
  
  if (event == gNlmeEnergyScanConfirm_c)
  {
        MSG_Free(MsgFromZDP->msgData.energyScanConf.resList.pEnergyDetectList);
  }
  
  if (event == gNlmeJoinConfirm_c)
  {
        if (MsgFromZDP->msgData.joinCnf.status != gZbSuccess_c)
        {
            if (ZDO_GetJoinMode() == gNwkRejoin_c)
              BeeAppUpdateDevice(BeeAppDataInit(appEndPoint), gRejoinFailed_c, 0, NULL, &MsgFromZDP->msgData.joinCnf);
        }
        else
        {
            sendEventToApp = TRUE;
            appEvent = gRejoinSucces_c;
        }
  }
  
  
  /* free the message, we're done */
  MSG_Free(MsgFromZDP);
  
  /* check send Event flag */
  if(sendEventToApp)
  {
      /* display the results of the event */
      BeeAppUpdateDevice(BeeAppDataInit(appEndPoint), appEvent, 0, NULL, NULL); 
  }
}
/******************************************************************************
*******************************************************************************
* Private functions
*******************************************************************************
******************************************************************************/

/******************************************************************************
*******************************************************************************
* Private Debug stuff
*******************************************************************************
******************************************************************************/
/***************************************************************************//**
 * @file
 *
 * Implements API to communicate with ZigBee stack
 *
 * @note
 *
 * @warning
 *******************************************************************************/
#include "ZigBeeRadio.hpp"
#include <string.h>
#include "BspSubsys.hpp"
#include "DataSubsys.hpp"
#include "DmpZigbeeAddrDefinitions.h"
#include "EventServerSubsystem.h"
#include "IEventPublisher.h"
#include "IModelController.hpp"
#include "NvApp.hpp"
#include "NvMfr.hpp"
#include "assert.hpp"
#include "ApsdeClstrMsgDefinitions.h"
#include "PhyInterface.h"
#include "MacInterface.h"
#include "RNG_Interface.h"
#include "MemManager.h"
#include "TimersManager.h"
#include "ZigbeeTask.h"
#include "ZdpManager.h"
#include "Phy.h"
#include "MCR20Drv.h"
#include "MCR20Reg.h" // for radio overrides
#include "NwkMacInterface.h"
#include "ZdoApsInterface.h"
#include "AspInterface.h"
#include "ApsVariables.h" // for APS counter

cZigBeeRadio * cZigBeeRadio::m_spThisPtr = NULL;
#define RADIO_PERIODIC_POLL     ( 300 )   /* NOTE, current base resolution on this is shared in the BSP */
#define KEEP_ALIVE_TIMEOUT_MS   ( 1500 )
#define LQI_PUBLISH_TIMEOUT_MS  ( 10000 ) /* Every 10 seconds publish LQI and LQI bars events */
#define TX_FAIL_CALC_TIMEOUT_MS ( 1500 )
#define TEMP_COMP_TIMEOUT_MS    ( 300000 ) /* Every 5 minutes look for temp compensation adjustments */
#define TX_FAILURE_LIMIT  	 40 /* in percentage 40 = 40% */
#define BAR_1_LQI_LIMIT    45
#define BAR_2_LQI_LIMIT    63
#define BAR_3_LQI_LIMIT    81
#define BAR_4_LQI_LIMIT    96
#define ZIGBEE_BASE_CHANNEL  11 // used to turn a Channel into an index
/******************************************************************************/
/* Default power calibration whenever nothing is in EEPROM */
struct DefaultCoarseCal_t
{
   BSP::TxType_t txType;
   uint8_t       cal[IZigBeeRadioController::ZIGBEE_PWR_MAX_ENUM_VAL][NUM_ZIGBEE_CHANNELS];
};
/* Default calibration if the unit is uncalibrated */
static const DefaultCoarseCal_t DEFAULT_ZIGBEE_COARSE_CAL[] =
{
   {
      BSP::TX_TYPE_ENHANCED_BP,
      { { 17, 17, 17, 17, 17, 17, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },  /* ZIGBEE_PWR_MODE_LOW */
        { 19, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 },  /* ZIGBEE_PWR_MODE_MID */
        { 19, 20, 20, 20, 20, 20, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 } } /* ZIGBEE_PWR_MODE_HIGH */
   },
   {
      BSP::TX_TYPE_ENHANCED_HH,
      { { 17, 16, 16, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },  /* ZIGBEE_PWR_MODE_LOW */
        { 19, 19, 19, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 },  /* ZIGBEE_PWR_MODE_MID */
        { 19, 19, 19, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 } } /* ZIGBEE_PWR_MODE_HIGH */
   },
   {
      BSP::TX_TYPE_FD_HH,
      { { 17, 16, 16, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16 },  /* ZIGBEE_PWR_MODE_LOW */
        { 19, 19, 19, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 },  /* ZIGBEE_PWR_MODE_MID */
        { 19, 19, 19, 18, 18, 18, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19 } } /* ZIGBEE_PWR_MODE_HIGH */
   },
   {
      BSP::TX_TYPE_MICRO_BP,
      { { 27, 26, 25, 24, 23, 23, 24, 25, 25, 26, 26, 26, 25, 24, 24, 26 },  /* ZIGBEE_PWR_MODE_LOW */
        { 30, 30, 26, 29, 28, 28, 28, 29, 29, 30, 30, 30, 30, 30, 29, 29 },  /* ZIGBEE_PWR_MODE_MID */
        /* Not uBP doesn't technically support high power mode - so this is a copy of mid */
        { 30, 30, 26, 29, 28, 28, 28, 29, 29, 30, 30, 30, 30, 30, 29, 29 } } /* ZIGBEE_PWR_MODE_HIGH */
   },
};
static const uint16_t DEF_COARSE_CAL_TABLE_ENTRIES = sizeof( DEFAULT_ZIGBEE_COARSE_CAL ) / sizeof( DefaultCoarseCal_t );
static const uint8_t DEF_FINE_CAL = 40; // 40 is the midpoint for fine calibration and ensures no change from coarse

typedef struct
{
   float m;
   float b;
} SlpIntrcpt_t;
static const SlpIntrcpt_t  m_TempCompVals[NUM_ZIGBEE_CHANNELS] =
{ { -0.040809973, -0.323222222 }, // 11
  { -0.039480442, -0.194222222 }, // 12
  { -0.038473132,  0.014777778 }, // 13
  { -0.045669855,  0.294444444 }, // 14
  { -0.045341715,  0.069444444 }, // 15
  { -0.04415521,   0.433111111 }, // 16
  { -0.039781159,  0.248777778 }, // 17
  { -0.036220349, -0.001222222 }, // 18
  { -0.033854006, -0.111555556 }, // 19
  { -0.038932174, -0.279888889 }, // 20
  { -0.038000255, -0.433222222 }, // 21
  { -0.038038973, -0.067888889 }, // 22
  { -0.039172759,  0.121444444 }, // 23
  { -0.043964721,  0.076444444 }, // 24
  { -0.043148381,  0.152777778 }, // 25
  { -0.045737298,  0           }  // 26
};
/******************************************************************************/

/***************************************************************************//**
 * Callback for whenever PHY radio changes the channel
 *
 * @param[in]    channel  the new channel (11-26)
 * @param[in]    pan      PAN (0 or 1)
 *
 * @note          This is a callback from the BeeStack - don't call directly.
 *
 * @warning       none
 *******************************************************************************/
extern "C" void OnPhyChannel
(
  uint8_t channel,
  uint8_t pan
)
{
   cZigBeeRadio::PhyChanCallBack(channel, pan);
}

/***************************************************************************//**
 * Endpoint 8 config callback for data indication.
 *
 * @param[in]    pMsg  pointer to AF message.
 *
 * @note          This is a callback from the BeeStack - don't call directly.
 *
 * @warning       none
 *******************************************************************************/
extern "C" void ShureAppMsgCallBack(apsdeToAfMessage_t *pMsg)
{
   cZigBeeRadio::AppMsgCallBack(pMsg);
}

/***************************************************************************//**
 * Endpoint 8 config callback for data confirmation.
 *
 * @param[in]    pMsg  pointer to AF message.
 *
 * @note          This is a callback from the BeeStack - don't call directly.
 *
 * @warning       none
 *******************************************************************************/
extern "C" void ShureAppCnfCallBack(apsdeToAfMessage_t *pMsg)
{
   cZigBeeRadio::AppCnfCallBack(pMsg);
}

/***************************************************************************//**
 * The only constructor!!!!
 *
 * @param[in]     nExtAddr           ZigBee extended 64bit address
 * @param[in]	  pCallbackHandler   Callback interface that is used by cZigBeeRadio
 *                                   to communicate asynchronous events in the radio. If
 *                                   NULL, radio would function normally, but the application
 *                                   would not know about changes in its state
 * @returns       none
 *
 * @note          none
 *
 * @warning       none
 *******************************************************************************/
cZigBeeRadio::cZigBeeRadio(const uint64_t& nExtAddr, IZigBeeCallback* pCallbackHandler) :
   m_pCallbackHandler(pCallbackHandler),
   m_nExtAddr(nExtAddr),
   m_bRadioInitialized(false),
   m_nChannel(0),
   m_nPanId(0),
   m_ConfirmSem("cs", false),
   m_RadioLock("rl"),
   m_bApsConfirm(false),
   TxAttemptCounter(0),
   TxFailureCounter(0),
   PeriodicFunction( RADIO_PERIODIC_POLL, "ZigbeeRadio" ),
   m_StartStopSem("ss", false),
   m_CurrLqi(0),
   m_ZigbeePowerMode( ZIGBEE_PWR_MODE_NORMAL ),
   m_bPaLnaEnable( true ), // default PA/LNA is enabled
   m_TempCompTimeout( 0 ),
   m_TempC ( 0 ),
   m_DiversityMode( DIVERSITY_DISABLE ),
   m_bFineCalValid( false )
{
   m_spThisPtr = this;
   memset(m_nVersion, 0, sizeof(m_nVersion));
   memset(ATxAttemptCounter, 0, sizeof(ATxAttemptCounter));
   memset(ATxFailureCounter, 0, sizeof(ATxFailureCounter));
   
   for( uint8_t index = 0; index < NUM_ZIGBEE_CHANNELS; index++ )
   {
     m_ZigbeePowerCal[index].coarseCal = DEF_COARSE_CAL;
     m_ZigbeePowerCal[index].fineCal = DEF_FINE_CAL;
   }
}

/***************************************************************************//**
 * Class destructor
 *
 * @returns       none
 *
 * @note          none
 *
 * @warning       none
 *******************************************************************************/
cZigBeeRadio::~cZigBeeRadio()
{
   m_spThisPtr = NULL;
}

/***************************************************************************//**
 * BeeStack ZDP callback.
 *
 * @param[in]    pMsg  pointer to ZDP message.
 *
 * @note          This is a callback from the BeeStack - don't call directly.
 *
 * @warning       none
 *******************************************************************************/
void cZigBeeRadio::BeeAppZdpCallBack ( zdpToAppMessage_t *pMsg, zbCounter_t counter )
{
  uint8_t event;
  (void)counter;
  /* get the event from ZDP */
  event = pMsg->msgType;
  if(event == gzdo2AppEventInd_c) /* many possible application events */
    event = pMsg->msgData.zdo2AppEvent;
  /* got a response to match descriptor */
  switch(event) {
     /* network start event processed */
  case gZDOToAppMgmtInitDevice_c:
     //ASSERT_INFO("gZDOToAppMgmtInitDevice_c");
     if ( NULL != m_spThisPtr )
     {
        m_spThisPtr->m_StartStopSem.Set();
     }
     break;
    /* network has been started */
  case gZDOToAppMgmtZEDRunning_c:
     if ( NULL != m_spThisPtr )
     {
         uint32_t timeNow = Time::MillisecondsElapsed();
         m_spThisPtr->TxAttemptCounter = 0;
         m_spThisPtr->TxFailureCounter = 0;
         memset( m_spThisPtr->ATxAttemptCounter, 0, sizeof( m_spThisPtr->ATxAttemptCounter));
         memset( m_spThisPtr->ATxFailureCounter, 0, sizeof( m_spThisPtr->ATxFailureCounter));
         m_spThisPtr->m_nChannel = NlmeGetRequest(gNwkLogicalChannel_c);
         memcpy(&m_spThisPtr->m_nPanId, NlmeGetRequest(gNwkPanId_c), sizeof( NlmeGetRequest(gNwkPanId_c) ));
         memcpy(&m_spThisPtr->m_nShortAddr, NlmeGetRequest(gNwkShortAddress_c), sizeof ( NlmeGetRequest(gNwkShortAddress_c) ));
         m_spThisPtr->m_CurrLqi = BAR_1_LQI_LIMIT; // set to a low, non-zero value starting out...@todo - JP
         /* Initialize keep alive payload to the short address of this device */
         m_spThisPtr->m_KeepAlivePayload = m_spThisPtr->m_nShortAddr; // ok to truncate
         m_spThisPtr->m_KeepAliveTimeout = timeNow + KEEP_ALIVE_TIMEOUT_MS;
         m_spThisPtr->m_LqiTimeout = timeNow + 1000; // set to expire soon after joining network...
         m_spThisPtr->m_FailureCalcTimeout = timeNow + TX_FAIL_CALC_TIMEOUT_MS;
         if ( NULL != m_spThisPtr->m_pCallbackHandler )
         {
            m_spThisPtr->m_pCallbackHandler->OnJoinedNetwork( m_spThisPtr->m_nShortAddr, m_spThisPtr->m_nPanId, m_spThisPtr->m_nChannel );
         }
     }
     break;
  case gZDOToAppMgmtStopped_c:
     //ASSERT_INFO("gZDOToAppMgmtStopped_c");
     if ( NULL != m_spThisPtr )
     {
        m_spThisPtr->m_StartStopSem.Set();
     }
     // let fall through to next case
  case gZDOToAppMgmtOffTheNetwork_c:
     if ( NULL != m_spThisPtr && m_spThisPtr->m_nChannel != 0 && m_spThisPtr->m_nPanId != 0 )
     {
        m_spThisPtr->m_nChannel = 0;
        m_spThisPtr->m_nPanId = 0;
        m_spThisPtr->m_nShortAddr = 0xFFFF;
        m_spThisPtr->m_CurrLqi = 0;
        if ( NULL != m_spThisPtr->m_pCallbackHandler )
        {
           m_spThisPtr->m_pCallbackHandler->OnLeftNetwork( );
        }
     }
     break;
  default:
     break;
  }
  if (pMsg->msgType == gNlmeEnergyScanConfirm_c)
    MSG_Free(pMsg->msgData.energyScanConf.resList.pEnergyDetectList);
   /* free the message from ZDP */
   MSG_Free(pMsg);
}

/***************************************************************************//**
 * BeeStack data indication callback.
 *
 * @param[in]    pMsg  pointer to ZDP message.
 *
 * @note          This is a callback from the BeeStack - don't call directly.
 *
 * @warning       none
 *******************************************************************************/
void cZigBeeRadio::AppMsgCallBack(apsdeToAfMessage_t *pMsg)
{
   if ( NULL != pMsg )
   {
      if ( gApsdeDataIndMsgType_d == pMsg->msgType )
      {
         if ( NULL != m_spThisPtr )
         {
            if ( NULL != m_spThisPtr->m_pCallbackHandler )
            {
               zbApsdeDataIndication_t *pIndication;
               pIndication = &(pMsg->msgData.dataIndication);
               uint16_t clusterId;
               memcpy(&clusterId, pIndication->aClusterId, sizeof(clusterId));
               m_spThisPtr->m_pCallbackHandler->OnApsdeDataIndicationReceived( pIndication->pAsdu, pIndication->asduLength, clusterId, pIndication->linkQuality, 0 ); //!@todo - APS seq number
               m_spThisPtr->m_CurrLqi = (uint8_t)( ( (float)( m_spThisPtr->m_CurrLqi + m_spThisPtr->m_CurrLqi + pIndication->linkQuality ) ) / 3.0f );
               if ( 0 == m_spThisPtr->m_CurrLqi )
               {
                  m_spThisPtr->m_CurrLqi = 1;
               }
            }
         }
      }
      /* Free memory allocated by data indication */
      AF_FreeDataIndicationMsg(pMsg);
   }
}
/***************************************************************************//**
 * BeeStack data confirm callback.
 *
 * @param[in]    pMsg  pointer to ZDP message.
 *
 * @note          This is a callback from the BeeStack - don't call directly.
 *
 * @warning       none
 *******************************************************************************/
void cZigBeeRadio::AppCnfCallBack(apsdeToAfMessage_t *pMsg)
{
   bool msgSuccess = false;
   if ( NULL != pMsg )
   {
      if ( gApsdeDataCnfMsgType_d == pMsg->msgType )
      {
         zbApsdeDataConfirm_t *pConfirm;
         pConfirm = &(pMsg->msgData.dataConfirm);
         if( pConfirm->status == gSuccess_c )
         {
            /* successful confirm */
            msgSuccess =  true;
         }
         if ( NULL != m_spThisPtr )
         {
            m_spThisPtr->TxAttemptCounter++;
            if ( !msgSuccess )
            {
               m_spThisPtr->TxFailureCounter++;
            }
            if ( pConfirm->confirmId == m_spThisPtr->m_nConfirmId )
            {
               m_spThisPtr->m_bApsConfirm = msgSuccess;
               /* Signal semaphore */
               m_spThisPtr->m_ConfirmSem.Set();
            }
         }
      }
      /* Free memory allocated in Call Back function */
      MSG_Free(pMsg);
   }
}

/***************************************************************************//**
 * Callback for whenever PHY radio changes the channel
 *
 * @param[in]    channel  the new channel (11-26)
 * @param[in]    pan      PAN (0 or 1)
 *
 * @note          This is a callback from the BeeStack - don't call directly.
 *
 * @warning       none
 *******************************************************************************/
void cZigBeeRadio::PhyChanCallBack( uint8_t channel, uint8_t pan)
{
   if ( NULL != m_spThisPtr )
   {
      // only pan 0 supported now
      if ( 0 == pan )
      {
         m_spThisPtr->m_nChannel = channel;
         
         m_spThisPtr->SetRadioPowerLevel( m_spThisPtr->GetChannelPowerCalibration( m_spThisPtr->m_nChannel ) );
         m_spThisPtr->SetRadioFinePowerLevel( m_spThisPtr->GetChannelFinePowerCalibration( m_spThisPtr->m_nChannel ) );
      }
   }
}

/***************************************************************************//**
* ZigBee radio periodic task. Does periodic items.
*
* @note
*     None.
*
* @warning
*     None.
*******************************************************************************/
void cZigBeeRadio::OnTaskRun( )
{
   uint32_t timeNow = Time::MillisecondsElapsed();
   /* If on the network - do Keep alive processing */
   if(0 != m_nChannel && 0 != m_nPanId)
   {
      if ( timeNow > m_KeepAliveTimeout )
      {
         //! @note m_KeepAliveTimeout is updated in SendApsdeDataRequest()
         m_KeepAlivePayload++; // its ok to wrap around
         SendApsdeDataRequest(&m_KeepAlivePayload, sizeof(m_KeepAlivePayload), KEEP_ALIVE_CLUSTER_ID, 0); // no timeout/retry on KeepAlives
      }
      if ( timeNow > m_LqiTimeout )
      {
         m_LqiTimeout = timeNow + LQI_PUBLISH_TIMEOUT_MS;
         uint8_t lqi = GetLqi();
         uint8_t lqiBars = GetLqiBars();
         ESS::GetEventPublisherInterface()->PublishEvent( DMP_ZIGBEE_RECEIVED_LINK_QUALITY,
                                                         (const void*)&lqi, sizeof( lqi ) );
         ESS::GetEventPublisherInterface()->PublishEvent( DMP_ZIGBEE_RECEIVED_LINK_QUALITY_BARS,
                                                         (const void*)&lqiBars, sizeof( lqiBars ) );
      }
      if ( timeNow > m_FailureCalcTimeout )
      {
         m_FailureCalcTimeout = timeNow + TX_FAIL_CALC_TIMEOUT_MS;
         TxFailureCalc();
      }
   }
   if ( timeNow > m_TempCompTimeout )
   {
      m_TempCompTimeout = timeNow + TEMP_COMP_TIMEOUT_MS;
      int32_t newTempC = BSP::GetMcuTemperatureC();
      if ( m_TempC != newTempC )
      {
         m_TempC = newTempC;
         // If we are on the network - adjust fine power per new temperature.
         if ( 0 != m_nChannel )
         {
            SetRadioFinePowerLevel(GetChannelFinePowerCalibration(m_nChannel));
         }
      }
   }
}
/***************************************************************************//**
 * Returns the number of Zigbee interfaces.
 *
 * @returns       1 - There is only 1 Zigbee interface.
 *
 * @note          This is included for completeness only.
 *
 * @warning       none
 *******************************************************************************/
uint8_t cZigBeeRadio::GetNumIfaces ( void ) const
{
   return 1; // This object supports only 1 Zigbee interface
}

/***************************************************************************//**
 * Initializes the BeeStack and its associated tasks.
 *
 * @note          This only needs to be called once at startup
 *
 * @note          This follows the AppInit procedure with the relevant items
 *
 * @warning       none
 *******************************************************************************/
void cZigBeeRadio::InitStack()
{
   /* Init memory blocks manager */
   MEM_Init();
   /* Init  timers module */
   TMR_Init();
   /* Init phy module */
   Phy_Init();
   /* RNG must be initialized after the PHY is Initialized */
   RNG_Init();
   /* Init mac module */
   MAC_Init();
   /* Init zigbeepro module */
   ZbPro_Init();
   /* Call after Nvm module init - to be able to restore NVM data */
   ZbApplicationInit();
}
/***************************************************************************//**
* Executes network initialization sequence of commands and in case
* of all of them succeeding, starts active object that reads data from UART
*
* @returns		true if network started successfully (radio and receiving task);
*             otherwise- false
*
* @note       Channel and pan id are set automatically by the zigbee radio if
*             not previously set.
*
* @warning
*******************************************************************************/
bool  cZigBeeRadio::StartNetwork()
{
   //Initialize Radio
   if(!IsInitialized())
   {
/*-------------------------------------*/
/*! @todo - a few radio overrides - do they go here or in stack for maintenance purposes?
 *     Output low modem GPIO for lowest power.
 *     Enable single mode for PA/LNA logic
 *     Offset compensation for LNA
 *     FAD
 */
      /* output low modem GPIO, no pulls */
      /** @note JP - MUST have GPIO 2 low for PA/LNA engaged */
      MCR20Drv_IndirectAccessSPIWrite(GPIO_PUL_EN, 0x00 );
      MCR20Drv_IndirectAccessSPIWrite(GPIO_DATA, 0x00 );
      MCR20Drv_IndirectAccessSPIWrite(GPIO_DIR, 0xFF );
      /*! ANT PAD CTL for PA/LNA:
      *    ANTX_CTRLMODE: Single mode: ANT_A=ANTX TX_SWITCH=TXON and RX_SWITCH=(RXON OR TXON).
      *    ANTX_EN: All enabled.
      */
      MCR20Drv_IndirectAccessSPIWrite(ANT_PAD_CTRL, cANT_PAD_CTRL_ANTX_EN);

      BSP::TxType_t boardType = DATA::GetModelController()->GetTxType();
      /* JP  temporarily - swap antennas for HandHelds */
      // This includes default values for XTAL_TRIM. These values should now
      // not change, as ATE has begun calibrating board by board. Keeping
      // these static values here accomplishes two goals:
      // 1) Keep operating values within a reasonable tolerance in the case of
      //    corrupted EEPROM
      // 2) Help ATE so they don't have to traverse as many values
      uint8_t defaultTrim = 0;
      switch (boardType)
      {
      case BSP::TX_TYPE_ENHANCED_HH:
         /* swap antennas for HandHelds */
         Asp_SetANTXState(true);
         // Enable FAD
         SetDiversityMode(DIVERSITY_ENABLE);
         defaultTrim = 86;
         break;
      case BSP::TX_TYPE_FD_HH:
         /* swap antennas for HandHelds */
         Asp_SetANTXState(true);
         // Enable FAD
         SetDiversityMode(DIVERSITY_ENABLE);
         defaultTrim = 91;
         break;
      case BSP::TX_TYPE_ENHANCED_BP:
         // Enable FAD
         SetDiversityMode(DIVERSITY_ENABLE);
         defaultTrim = 148;
         break;
      case BSP::TX_TYPE_MICRO_BP:
         // Enable FAD
         SetDiversityMode(DIVERSITY_ENABLE);
         defaultTrim = 77;
         break;
      default:
         break;
      }
      uint8_t trim = 0;
      if ( !DATA::Mfr().GetXtalTrim(trim) )
      {
         trim = defaultTrim;
      }
      MCR20Drv_IndirectAccessSPIWrite(XTAL_TRIM, trim);
      /* Offset for LNA */
      MCR20Drv_IndirectAccessSPIWrite(CCA1_ED_OFFSET_COMP, 0x74); // default is 0x6d
      //! @note UPDATE 9/29/2016 update per latest data collection - still not final
      MCR20Drv_IndirectAccessSPIWrite(LQI_OFFSET_COMP, 0x22);     // default is 0x24
      /* Setup CCA3 for CCA1 OR CCA2 (defaults to CCA1 AND CCA2 ) */
      uint8_t ccaCtlReg = MCR20Drv_IndirectAccessSPIRead( CCA_CTRL );
      ccaCtlReg &= ~(cCCA_CTRL_CCA3_AND_NOT_OR);
      MCR20Drv_IndirectAccessSPIWrite( CCA_CTRL, ccaCtlReg);
/*! End Radio overrides */
/*-------------------------------------*/

      /* Setup IEEE EUI 64 address */
      MacPhyInit_WriteExtAddress( reinterpret_cast<uint8_t *>(&m_nExtAddr) );
      NlmeSetRequest(gNwkIeeeAddress_c, reinterpret_cast<uint8_t *>(&m_nExtAddr) );
      Zdp_AppRegisterCallBack(cZigBeeRadio::BeeAppZdpCallBack);
      // Initialize temperature
      m_TempC = BSP::GetMcuTemperatureC();
      // Read power mode
      if ( !DATA::App().GetZigbeePowerMode( m_ZigbeePowerMode ) )
      {
         m_ZigbeePowerMode = ZIGBEE_PWR_MODE_NORMAL;
      }
      /* Load zigbee calibration. If uncalibrated, a default config
       * will be loaded. */
      LoadZigbeeCal( m_ZigbeePowerMode );
      /* Start the network */
      m_StartStopSem.Wait(0); // clear out any previous event
      ZDO_Start(gZdoStartMode_Associate_c);
      ZbTriggerZigbeeRTOSTask(0);
      m_StartStopSem.Wait(); // wait for startup event
      SetInitialized(true);
      ASSERT_ARGS1("Network Started, Radio level %d", GetRadioPowerLevel());
   }
   else
   {
      ASSERT_SH (false, "Attempt to start already started radio");
   }
   return IsInitialized();
}

/***************************************************************************//**
 * Stops ZigBee Network. No devices can join the network or send
 * data to this device after network is stopped
 *
 * @returns		true if network was stopped (only radio, not including
 *				   receiving task); otherwise false
 *
 * @note
 *
 * @warning
 *******************************************************************************/
bool  cZigBeeRadio::StopNetwork()
{
   ASSERT_INFO( "Stopping Network" );
   if ( IsInitialized() )
   {
      // equivalent to ZDO leave plus...
      //   gZdoStopMode_ShureStop_c will stop network for good...
      m_StartStopSem.Wait(0); // clear out any previous event
      ZDO_StopEx(gZdoStopMode_ShureStop_c | gZdoStopMode_Announce_c | gZdoStopMode_ResetTables_c | gZdoStopMode_ResetNvm_c );
      if ( RESULT_SUCCESS != m_StartStopSem.Wait(3000) ) // wait for stopping for 3 second
      {
         ASSERT_INFO("Trying to stop again");
         ZDO_StopEx(gZdoStopMode_ShureStop_c | gZdoStopMode_ResetTables_c | gZdoStopMode_ResetNvm_c );
         m_StartStopSem.Wait(3000);
      }
      /* Set RX on when IDLE to OFF after stopping network. This leaves
       * the modem sequencer in IDLE mode instead of RX mode. The stack
       * may or may not leave the modem in IDLE and we want to make sure it is.
       * The stack restores this setting on startup of the network - so no
       * need to restore it in StartNetwork (in fact that could cause other
       * issues because it starts the receiver mode prematurely.
       */
      CTask::Sleep(20);
      PhyPlmeSetRxOnWhenIdle(FALSE, 0);
      PhyPlmeForceTrxOffRequest();
      SetInitialized(false);
   }
   ASSERT_INFO("Network stopped");
   return (!IsInitialized());
}

/***************************************************************************//**
 * Shutdown ZigBee Network for power off.
 *
 * @returns		true if network was stopped (only radio, not including
 *				   receiving task); otherwise false
 *
 * @note
 *
 * @warning
 *******************************************************************************/
bool  cZigBeeRadio::ShutdownNetwork()
{
   ASSERT_INFO( "Shutdown Network" );
   if ( IsInitialized() )
   {
      // equivalent to ZDO leave plus...
      //   gZdoStopMode_ShureStop_c will stop network for good...
      ZDO_StopEx(gZdoStopMode_ShureStop_c | gZdoStopMode_Announce_c | gZdoStopMode_ResetTables_c | gZdoStopMode_ResetNvm_c );
   }
   return (!IsInitialized());
}
/***************************************************************************//**
 * Stops than Starts ZigBee Network. No devices can join the network or send
 * data to this device after network is stopped
 *
 * @returns    true if network was restarted otherwise false
 *
 * @note      This method is an agreggation of Start and Stop network
 *            calls. Each of those calls can have their own reasons for
 *            failures.
 *
 * @warning
 *******************************************************************************/
bool cZigBeeRadio::RestartNetwork()
{
   bool bResult = false;
   if (IsInitialized())
   {
      // ZDO_Stop will issue a gZdoStopMode_Stop_c which will just restart the network
      ZDO_Stop();
   }
   else
   {
      //network is not running, just start it
      if(0 != m_nChannel && 0 != m_nPanId)
      {
         bResult = StartNetwork();
      }
   }
   return bResult;
}

/***************************************************************************//**
 * Checks if ZigBee network is started
 *
 *
 * @returns		true if network was started successfully
 *				   (radio and receiving task), otherwise- false
 *
 * @note
 *
 * @warning This function does not return a definitive result about the state of
 *				ZigBee network. It only returns whether this class started the network
 *				successfully or not.
 *******************************************************************************/
bool cZigBeeRadio::IsNetworkStarted() const
{
   return IsInitialized();
}

/***************************************************************************//**
 * Sets the ZigBee radio extended address
 *
 * @param[out]	sInfo structure containing ZigBee radio information
 *
 * @returns
 *
 * @note    This function should only be exposed by the subsystem.
 *
 * @note    If this function is called after startup, then a
 *          restart of radio is necessary for this to take effect
 *
 * @warning
 *******************************************************************************/
void cZigBeeRadio::SetExtendedAddress( const uint64_t& nExtAddr )
{
   m_nExtAddr = nExtAddr;
}

/***************************************************************************//**
 * Retrieves basic info about ZigBee radio
 *
 * @param[out]	sInfo structure containing ZigBee radio information
 *
 * @returns
 *
 * @note    The capabilities of this function can be extended by adding more members
 *			    to ZigBeeRadioInfo structure
 *
 * @warning
 *******************************************************************************/
uint64_t cZigBeeRadio::GetExtendedAddress() const
{
   return m_nExtAddr;
}

/***************************************************************************//**
 * Sets handler that would be called when certain event(s) occurr in the radio.
 * Events are defined by IZigBeeCallback interface
 *
 * @param[in]	pCallbackHandler pointer to the object implementing IZigBeeCallback
 *
 * @returns		the previous callback interface pointer or NULL if no interface
 *				   was set before
 *
 * @note
 *
 * @warning
 *******************************************************************************/
IZigBeeCallback* cZigBeeRadio::SetCallbackHandler(IZigBeeCallback* pCallbackHandler)
{
   IZigBeeCallback* pExisting = m_pCallbackHandler;
   m_pCallbackHandler = pCallbackHandler;
   return pExisting;
}

/***************************************************************************//**
* Enables user's of this class to send APSDE data to ZigBee coordinator
*
* @param[in]  pData   pointer to data to be sent
* @param[in]  nDataLength -length of the data to be set in octets
* @param[in]  nClusterId   cluster id for the request
* @param[in]  nTimeoutMs   request's timeout (currently unused)
*
* @returns    true if request is successfully sent to ZigBee radio
*
* @note       none
*
* @warning    none
*******************************************************************************/
bool cZigBeeRadio::SendApsdeDataRequest( const void* pData,
                                         uint8_t nDataLength,
                                         uint16_t nClusterId,
                                         uint32_t nTimeoutMs )
{
   bool retVal = false;
   uint8_t lastApsdeCounter = 0;
   afAddrInfo_t addrInfo;
   static const zbNwkAddr_t   msgDstAddr = {0x00, 0x00};   /* Default Short Addr of Coordinator */
   static const zbEndPoint_t  dstEndPoint = 8;             /* Destination End Point */
   static const zbEndPoint_t  endPoint = 8;                /* Source End Point */
   /* If on the network - send the message */
   if(0 != m_nChannel && 0 != m_nPanId)
   {
      addrInfo.dstAddrMode = gZbAddrMode16Bit_c;
      Copy2Bytes(addrInfo.dstAddr.aNwkAddr, msgDstAddr);
      addrInfo.dstEndPoint = dstEndPoint;
      addrInfo.srcEndPoint = endPoint;
      addrInfo.txOptions = gApsTxOptionNone_c;
      addrInfo.radiusCounter = 1;
      /* set up cluster ID */
      Copy2Bytes(addrInfo.aClusterId, &nClusterId);
      m_RadioLock.Lock();
      /* Reset keep alive counter anytime any message is sent */
      m_KeepAliveTimeout = Time::MillisecondsElapsed() + KEEP_ALIVE_TIMEOUT_MS;
      if ( nTimeoutMs > 0 )
      {
         m_bApsConfirm = false;
         m_ConfirmSem.Wait(OS_NO_WAIT); // clear any semaphore
         /* Grab the APSDE counter for potential resend */
         lastApsdeCounter = ApsmeGetCounter();
         AF_DataRequest(&addrInfo, nDataLength, (uint8_t *)pData, &m_nConfirmId);
         m_ConfirmSem.Wait(nTimeoutMs);
       //  ASSERT_INFO("Got conf!");
         retVal = m_bApsConfirm;
         //! @note - use ApsmeGetCounter() to get the APS counter on the initial send
         //          and giApsCounter to set it on the resend (see the old BeeStack TxMsg)
         if ( false == retVal )
         {
            // wait before trying again...this should be a wait until
            // the very next tick... which makes it random from 0 - 5 msec.
            CTask::Sleep(1);
            /* Reset the APSDE counter for the retry */
            if ( 1 == ( ApsmeGetCounter() - lastApsdeCounter) ||
                ( 0 == ApsmeGetCounter() && 0xFF == lastApsdeCounter ) )
            {
               giApsCounter = lastApsdeCounter;
            }
             AF_DataRequest(&addrInfo, nDataLength, (uint8_t *)pData, &m_nConfirmId);
             m_ConfirmSem.Wait(nTimeoutMs);
             //  ASSERT_INFO("Got conf!");
             retVal = m_bApsConfirm;
         }
      }
      else
      {
         zbApsConfirmId_t confirmId;
         AF_DataRequest(&addrInfo, nDataLength, (uint8_t *)pData, &confirmId);
         retVal = true; // sent the request - didn't ask for response
      }
      m_RadioLock.Unlock();
   }
   return retVal;
}

/***************************************************************************//**
 * Retrieves radio channel currently used by the Radio
 *
 * @returns	current radio channel; 0 if radio was not started
 *
 *
 * @note		 In this implementation the channel # returned is not the one provided
 *			    by ZigBee stack but the one remembered from the time radio was successfully
 *           started or when radio changed channel last time.
 *
 * @warning
 *******************************************************************************/
uint8_t cZigBeeRadio::GetRadioChannel() const
{
   return m_nChannel;
}

/***************************************************************************//**
 * Retrieves network PAN ID from the Radio
 *
 * @returns	current PAN ID, 0 if radio was not started
 *
 * @note		 In this implementation the Pan Id returned is not the one provided
 *			    by ZigBee stack but the one remembered from the time radio was successfully
 *           started.
 *
 * @warning
 *******************************************************************************/
uint16_t cZigBeeRadio::GetPanId() const
{
   return m_nPanId;
}

/***************************************************************************//**
 * Forceably requests the device to leave the network.
 *
 * @returns      True if the operation was sucessful, false otherwise.
 *
 * @note         This function simply requests the device to leave this network,
 *               it does not prevent the device from actually attempting to
 *               rejoin this network.
 *
 * @warning
 *******************************************************************************/
bool cZigBeeRadio::ForceDeviceToLeaveNetwork(void)
{
   ZDO_Leave();
   return true;
}

/***************************************************************************//**
 * Returns a copy of current statistics  values
 *
 * @param[out]	rsStatsCounters   structure into which stats counters get copied
 *
 * @returns
 *
 * @note
 *
 * @warning
 *******************************************************************************/
void cZigBeeRadio::GetCurrStatsCounters(SStatsCounters& rsStatsCounters) const
{
 //  rsStatsCounters = m_sStatsCounters; //Shallow copy is intended
}

/***************************************************************************//**
 * Resets counters (various statistical info)
 *
 * @returns
 *
 * @note
 *
 * @warning
 *******************************************************************************/
void cZigBeeRadio::ResetStatsCounter()
{
 //  memset(&m_sStatsCounters, 0, sizeof(m_sStatsCounters));
}


/***************************************************************************//**
* Sets the Radio Power Level for test purposes.
*
* @param[in]     level Radio Power Level
*
* @note          This function is intended for test purposes only
*
* @warning       none
*******************************************************************************/
void cZigBeeRadio::SetRadioPowerLevel(uint8_t level)
{
   uint8_t retVal = Asp_SetPowerLevel(level);
   if ( gSuccess_c != retVal )
   {
      ASSERT_ARGS1("Failed to set power level %d!", level);
   }
}

/***************************************************************************//**
* Gets the Radio Power Level
*
* @return        current Radio Power Level
*
* @note          Radio Power Level is a test only parameter
*
* @warning       none
*******************************************************************************/
uint8_t cZigBeeRadio::GetRadioPowerLevel() const
{
   return Asp_GetPowerLevel();
}

/***************************************************************************//**
* Sets the Radio Fine Power Level for test purposes.
*
* @param[in]     level Radio Fine Power Level
*
* @note          This function is intended for test purposes only
*
* @warning       none
*******************************************************************************/
void cZigBeeRadio::SetRadioFinePowerLevel(uint8_t level)
{
   if ( level >= MIN_FINE_POWER && level <= MAX_FINE_POWER )
   {
      level |= 0x20; // always ensure PA Cal is disabled (NOTE the values of MIN_FINE_POWER, and MAX_FINE_POWER ensure this, but this is for completeness
      MCR20Drv_IndirectAccessSPIWrite( PA_CAL, level );
   }
   else
   {
      ASSERT_ARGS1("Invalid fine power level %d!", level);
   }
}

/***************************************************************************//**
* Gets the Radio Fine Power Level
*
* @return        current Radio Fine Power Level
*
* @note          Radio Fine Power Level is a test only parameter
*
* @warning       none
*******************************************************************************/
uint8_t cZigBeeRadio::GetRadioFinePowerLevel() const
{
   uint8_t level = MCR20Drv_IndirectAccessSPIRead( PA_CAL );
   return level;
}
/***************************************************************************//**
* Set radio power mode
*
* @return        True = success
*
* @note          none
*
* @warning       none
*******************************************************************************/
bool cZigBeeRadio::SetRadioPowerMode( ZigbeePowerMode_t mode )
{
   bool ret = false;
   if ( mode < ZIGBEE_PWR_MAX_ENUM_VAL )
   {
      //**************************************************
      /** @note Limit all TXs to mid power */
      if ( mode > ZIGBEE_PWR_MODE_MID )
      {
         mode = ZIGBEE_PWR_MODE_MID;
      }
      //**************************************************
      if ( m_ZigbeePowerMode != mode )
      {
         // Load cal for this power mode and program power for current
         // channel
         LoadZigbeeCal( mode );
         SetRadioPowerLevel( GetChannelPowerCalibration( PhyPlmeGetCurrentChannelRequest(0) ) ); // only PAN 0
         SetRadioFinePowerLevel( GetChannelFinePowerCalibration( PhyPlmeGetCurrentChannelRequest(0) ) ); // only PAN 0
         // Save to NV memory
         DATA::App().SetZigbeePowerMode( mode );
         // Publish to Event Server
         uint8_t u8Mode = mode;
         ESS::GetEventPublisherInterface()->PublishEvent( DMP_ZIGBEE_TRANSMIT_POWER_MODE, &u8Mode, sizeof(u8Mode) );
         m_ZigbeePowerMode = mode;
         ret = true;
      }
      else
      {
         ret = true;
      }
   }
   return ret;
}
/***************************************************************************//**
* Return current radio power mode
*
* @note          Radio Power Level is a test only parameter
*
* @warning       none
*******************************************************************************/
IZigBeeRadioController::ZigbeePowerMode_t cZigBeeRadio::GetRadioPowerMode( ) const
{
   return m_ZigbeePowerMode;
}
/***************************************************************************//**
* Gets the Radio Link Quality Indicator (LQI)
*
* @return        current LQI
*
* @note          none
*
* @warning       none
*******************************************************************************/
uint8_t cZigBeeRadio::GetLqi(void) const
{
   return m_CurrLqi;
}

/***************************************************************************//**
* Gets the Radio Link Quality Indicator in bars (LQI)
*
* @return        current LQI bars
*
* @note          none
*
* @warning       none
*******************************************************************************/
uint8_t cZigBeeRadio::GetLqiBars(void) const
{
   uint8_t lqi = m_CurrLqi;
   uint8_t lqiBars;
   // Current LQI bars based upon LQI
   if (lqi > BAR_4_LQI_LIMIT )
   {
      lqiBars = 5;
   }
   else if ( lqi > BAR_3_LQI_LIMIT )
   {
      lqiBars = 4;
   }
   else if ( lqi > BAR_2_LQI_LIMIT )
   {
      lqiBars = 3;
   }
   else if ( lqi > BAR_1_LQI_LIMIT )
   {
      lqiBars = 2;
   }
   else if ( lqi > 0 )
   {
      lqiBars = 1;
   }
   else
   {
      lqiBars = 0;
   }
   return lqiBars;
}

/***************************************************************************//**
* Set PA/LNA mode to On or Bypass.
*
* @param[in]     enable   True = PA/LNA on; False = PA/LNA in bypass
*
* @note          RF6555 based designs can only do an RX bypass.
*                RFFM6204 based designs can do a true RX and TX bypass.
*
* @warning       none
*******************************************************************************/
void cZigBeeRadio::SetPaLnaEnable (bool enable)
{
   uint8_t regVal;
   // RFFM6204 based products...
   if ( BSP::TX_TYPE_MICRO_BP == DATA::GetModelController()->GetTxType() )
   {
      // All we need to do is set GPIO2.
      /* Switch Control Logic Table
            Mode        C_EN    C_TX    C_RX    C_BYP
            TX Mode     High    High    Low     Low
            RX Mode     High    Low     High    Low
            Bypass Mode High    Low     Low     High
            Power Down  Low     Low     Low     Low
      */
      regVal = MCR20Drv_IndirectAccessSPIRead(GPIO_DATA);
      if (enable)
      {
         regVal &= ~(0x2); // set GPIO2 low
      }
      else
      {
         regVal |= (0x2);  // set GPIO2 high
      }
      MCR20Drv_IndirectAccessSPIWrite(GPIO_DATA, regVal );
   }
   else // RF6555 based products...
   {
      /* Control Logic
            Mode        CE      C_RX_TX C_LNA   ANTSEL
            TX-ANT1     High    High    Low     Low
            TX-ANT2     High    High    Low     High
            RX-ANT1 LNA High    Low     Low     Low
            RX-ANT1 BYP High    Low     High    Low
            RX-ANT2 LNA High    Low     Low     High
            RX-ANT2 BYP High    Low     High    High
            Power Down  Low     Low     Low     Low
      */
      // This is a hack for the RFMD6555. It doesn't technically support a
      // TX bypass mode - only the RX bypass. So we just power down the chip.
      // The only way to do that is to disable RXSWITCH and TXSWITCH
      // Its a bit of a hack, but we want the chip to be disabled
      // when either TX or RX is on.

      regVal = MCR20Drv_IndirectAccessSPIRead(ANT_PAD_CTRL);
      if (enable)
      {
         regVal |= 0x3;  //  All enabled (RX SWITCH and TX SWITCH)
      }
      else
      {
         regVal &= ~(0x1); // Disable RX SWITCH and TX SWITCH (only ANT A/ANT B enabled)
      }
      MCR20Drv_IndirectAccessSPIWrite(ANT_PAD_CTRL, regVal );
   }
   m_bPaLnaEnable = enable;
}

/***************************************************************************//**
* Set Diversity Mode
*
* @param[in]     mode   FAD mode
*
* @note          none
*
* @warning       none
*******************************************************************************/
bool cZigBeeRadio::SetDiversityMode (AntDiversityMode_t mode)
{
   bool retVal = false;
   uint8_t AgcCtlReg = 0;
   //! @note - JP this touches some deep registers in the modem with some
   //          very specific values. Care must be taken.
   if ( gAspSuccess_c == Asp_XcvrReadReq(true, ANT_AGC_CTRL, sizeof(AgcCtlReg), &AgcCtlReg) )
   {
      if ( DIVERSITY_ENABLE == mode )  //! @todo LPPS
      {
         AgcCtlReg |= (cANT_AGC_CTRL_FAD_EN_Mask_c);
         // Adjust FAD threshold
         uint8_t fadThr = 140; //! @note - JP 8/2/2017 FAD threshold default is 130. Set to 140
         Asp_XcvrWriteReq(true, FAD_THR, sizeof(fadThr), &fadThr);
         // Adjust CORR NVAL down by 1
         uint8_t corrNval = 0x12;
         Asp_XcvrWriteReq(true, CORR_NVAL, sizeof(corrNval), &corrNval);
      }
      else
      {
         AgcCtlReg &= (~cANT_AGC_CTRL_FAD_EN_Mask_c);
         // Disabling diversity - force an antenna
         BSP::TxType_t boardType = DATA::GetModelController()->GetTxType();
         switch (boardType)
         {
         case BSP::TX_TYPE_ENHANCED_HH:
         case BSP::TX_TYPE_FD_HH:
            /* Diversity disabled, force swap antennas for HandHelds */
            AgcCtlReg |= (cANT_AGC_CTRL_ANTX_Mask_c);
            break;
         default:
            //! @todo - do we want to force one - or leave at last one  ???
            AgcCtlReg &= (~cANT_AGC_CTRL_ANTX_Mask_c);
            break;
         }
         // Adjust CORR NVAL back to default
         //! @note - see MCR20Overwrites.h
         uint8_t corrNval = 0x13;
         Asp_XcvrWriteReq(true, CORR_NVAL, sizeof(corrNval), &corrNval);
         // Adjust FAD threshold back to default
         uint8_t fadThr = 130;
         Asp_XcvrWriteReq(true, FAD_THR, sizeof(fadThr), &fadThr);
      }
      if ( gAspSuccess_c == Asp_XcvrWriteReq(true, ANT_AGC_CTRL, sizeof(AgcCtlReg), &AgcCtlReg ) )
      {
         m_DiversityMode = mode;
         retVal = true;
      }
   }
   return retVal;
}

/***************************************************************************//**
*
* @returns       none
*
* @note          The ZigBee stack will self leave whenever gMaxNwkLinkRetryThreshold_c
*                (defined in BeeStackConfiguration.h) number of attempts in a row
*                have failed. This is currently set to 10 failures in a row.
*                This calculation is to help the node also leave on a lossy link
*                based upon a running average.
*
* @note          See this chart for the formula on percentageOfFailure
*                percentageOfFailure is 'desensitized' by 1 attempt.
*
                        attempts    failures    "percent fail"    above 40%?
                        4           0           0                 NO
                        4           1           20                NO
                        4           2           40                YES
                        4           3           60                YES
                        4           4           80                YES
                        5           0           0                 NO
                        5           1           16.6666           NO
                        5           2           33.3333           NO
                        5           3           50                YES
                        5           4           66.6666           YES
                        5           5           83.3333           YES
                        6           0           0                 NO
                        6           1           14.2857           NO
                        6           2           28.5714           NO
                        6           3           42.8571           YES
                        6           4           57.1428           YES
                        6           5           71.4285           YES
                        6           6           85.7142           YES
                        7           0           0                 NO
                        7           1           12.5              NO
                        7           2           25                NO
                        7           3           37.5              NO
                        7           4           50                YES
                        7           5           62.5              YES
                        7           6           75                YES
                        7           7           87.5              YES
                        8           0           0                 NO
                        8           1           11.1111           NO
                        8           2           22.2222           NO
                        8           3           33.3333           NO
                        8           4           44.4444           YES
                        8           5           55.5555           YES
                        8           6           66.6666           YES
                        8           7           77.7777           YES
                        8           8           88.8888           YES
                        9           0           0                 NO
                        9           1           10                NO
                        9           2           20                NO
                        9           3           30                NO
                        9           4           40                YES
                        9           5           50                YES
                        9           6           60                YES
                        9           7           70                YES
                        9           8           80                YES
                        9           9           90                YES
*
*
*
* @warning       none
*******************************************************************************/
void cZigBeeRadio::TxFailureCalc()
{
  uint32_t percentageOfFailure;
  int32_t  attemptDelta;
  int32_t  failureDelta;
   /* This runs in timer callback so loop unrolling for speed */
  /* Moving history of attempts and failures */
  ATxAttemptCounter[6] = ATxAttemptCounter[5];
  ATxAttemptCounter[5] = ATxAttemptCounter[4];
  ATxAttemptCounter[4] = ATxAttemptCounter[3];
  ATxAttemptCounter[3] = ATxAttemptCounter[2];
  ATxAttemptCounter[2] = ATxAttemptCounter[1];
  ATxAttemptCounter[1] = ATxAttemptCounter[0];
  ATxAttemptCounter[0] = TxAttemptCounter;
  attemptDelta = ATxAttemptCounter[0] - ATxAttemptCounter[6];
  if (attemptDelta < 0)
     attemptDelta = 0;
  ATxFailureCounter[6] = ATxFailureCounter[5];
  ATxFailureCounter[5] = ATxFailureCounter[4];
  ATxFailureCounter[4] = ATxFailureCounter[3];
  ATxFailureCounter[3] = ATxFailureCounter[2];
  ATxFailureCounter[2] = ATxFailureCounter[1];
  ATxFailureCounter[1] = ATxFailureCounter[0];
  ATxFailureCounter[0] = TxFailureCounter;
  failureDelta = ATxFailureCounter[0] - ATxFailureCounter[6];
  if (failureDelta < 0)
     failureDelta = 0;
  /* Ensure there are enough attempts for a significant calculation */
  if ( attemptDelta > 4 )
  {
     /* The current percentage of failures, of the total of transmitions attempts. */
     percentageOfFailure = ( failureDelta * 100 ) / ( attemptDelta + 1 );
     if ( percentageOfFailure >= TX_FAILURE_LIMIT )
     {
        ZeDTransmitFailureCounterCheck(0xffff); // a very large number
     }
  }
}
/***************************************************************************//**
* Return calibrated channel coarse power.
* Calibration validity determined during network initialization.
*
* @note          none
*
* @warning       none
*******************************************************************************/
uint8_t cZigBeeRadio::GetChannelPowerCalibration( uint8_t channel ) const
{
   uint8_t ret = DEF_COARSE_CAL;
   channel -= ZIGBEE_BASE_CHANNEL;
   
   if ( channel < NUM_ZIGBEE_CHANNELS )
   {
      ret = m_ZigbeePowerCal[ channel ].coarseCal;
   }
   return ret;
}

/***************************************************************************//**
* Return calibrated channel fine power.
* Calibration validity determined during network initialization.
*
* @note          none
*
* @warning       none
*******************************************************************************/
uint8_t cZigBeeRadio::GetChannelFinePowerCalibration( uint8_t channel ) const
{
   uint8_t fineCal = DEF_FINE_CAL;
   channel -= ZIGBEE_BASE_CHANNEL;
   if ( channel < NUM_ZIGBEE_CHANNELS )
   {
      fineCal = m_ZigbeePowerCal[ channel ].fineCal;
     /** @note, tempCompFineDelta is in 1dB steps. The fine calibration
       *          is also in approx 1dB steps.
       */
      int32_t tempCompFineDelta = CalculateTempCompFinePowerDelta(channel); // signed integer
      int32_t adjustedFineCal = static_cast<int32_t>(fineCal) + tempCompFineDelta;
      
      fineCal = static_cast<uint8_t>(adjustedFineCal);
   }
   return fineCal;
}
/***************************************************************************//**
* Load zigbee calibration for specified power mode into power cal member.
*
* @param [in]     mode     Zigbee power mode
*
* @note           Start network will validate the zigbee calibration and set
*                 the valid flag. If cal valid, we will load from EEPROM, otherwise
*                 we will use default values.
*
* @warning        none
*******************************************************************************/
void cZigBeeRadio::LoadZigbeeCal( ZigbeePowerMode_t mode)
{
   if ( mode < ZIGBEE_PWR_MAX_ENUM_VAL )
   {
      /* Load default calibration table */
      /* First, initialize to "safe" value */
      const DefaultCoarseCal_t* pDefaultTable = &DEFAULT_ZIGBEE_COARSE_CAL[0];
      /* Now see if we can grab something closer ...*/
      const BSP::TxType_t txType = DATA::GetModelController()->GetTxType();
      for ( int i = 0; i < DEF_COARSE_CAL_TABLE_ENTRIES; i++ )
      {
         if ( DEFAULT_ZIGBEE_COARSE_CAL[i].txType == txType )
         {
            pDefaultTable = &DEFAULT_ZIGBEE_COARSE_CAL[i];
            break;
         }
      }
      m_bFineCalValid = false;
      /* Check coarse calibration validity */
      if ( DATA::Mfr().IsZigbeeCalibrationValid() )
      {
         m_bFineCalValid = DATA::Mfr().IsZigbeeFineCalibrationValid();
         NvMfr::ZigbeePowerModeIdx_t idx;
         switch ( mode )
         {
            case ZIGBEE_PWR_MODE_MID:   idx = NvMfr::ZB_PWR_FCC_IDX;        break;
            case ZIGBEE_PWR_MODE_LOW:   idx = NvMfr::ZB_PWR_ETSI_LOW_IDX;   break;
            //*************************************************************************
            //! @note - always load MID table - even in high mode
            case ZIGBEE_PWR_MODE_HIGH:  idx = NvMfr::ZB_PWR_FCC_IDX;        break;
            //*************************************************************************
            default:                    idx = NvMfr::ZB_PWR_ETSI_LOW_IDX;   break;
         }
         for ( uint8_t chan = 0; chan < NUM_ZIGBEE_CHANNELS; ++chan )
         {
            if ( DATA::Mfr().GetZigbeePowerCalibration( idx, chan, m_ZigbeePowerCal[ chan ].coarseCal ) )
            {
               /* Got a valid coarse cal, now lets try for fine calibration */
               if (!( m_bFineCalValid && DATA::Mfr().GetZigbeeFinePowerCalibration( idx, chan, m_ZigbeePowerCal[ chan ].fineCal ) ) )
               {
                  /* NV RAM bad for fine, use default fine calibration */
                  m_ZigbeePowerCal[ chan ].fineCal   = DEF_FINE_CAL;
               }
            }
            else
            {
               /* NV RAM bad for coarse, use default table, and also default
                * fine calibration, as it makes no sense if coarse isn't valid
                */
               m_ZigbeePowerCal[ chan ].coarseCal = pDefaultTable->cal[mode] [ chan ];
               m_ZigbeePowerCal[ chan ].fineCal   = DEF_FINE_CAL;
            }
         }
      }
      else
      {
         /* NV RAM uncalibrated, use default table*/
         /** @note - if coarse isn't calibrated, then default fine as fine
                     makes no sense for uncalibrated coarse values
          */
         for ( uint8_t chan = 0; chan < NUM_ZIGBEE_CHANNELS; ++chan )
         {
            m_ZigbeePowerCal[ chan ].coarseCal = pDefaultTable->cal[mode] [ chan ];
            m_ZigbeePowerCal[ chan ].fineCal   = DEF_FINE_CAL;
         }
      }
   }
}

/***************************************************************************//**
* Calculates a new fine power delta based on current temperature delta from room temp
*
* @param[in]     chanIndex  channel index into array
*
* @note          Taken from the DWAP.
*
* @warning       None
*******************************************************************************/
int32_t cZigBeeRadio::CalculateTempCompFinePowerDelta(uint8_t chanIndex) const
{
   int32_t rounded_dBm = 0;
   /* Only apply temperature compensation in normal mode */
   if ( ZIGBEE_PWR_MODE_NORMAL == m_ZigbeePowerMode )
   {
      //! @note: JP this could use the temperature calibration from ATE stored in NVRAM.
      //          But on the WAP we found that room temperature is a better metric.
      //          AVERAGE of enhHH, enhBP, uBP at room temperature is 33 C.
      const int32_t roomTempMcu = 33;
      int32_t tempDeltaFromRoomTemp = m_TempC - roomTempMcu;
      //! @note: JP if the temperature delta is close to room temperature - don't
      //         bother at all.  Testing on the transmitter's slope/intercept
      //         shows we cross 1dB (non-rounded) at +15 degrees and -13 degrees.
      //         When we round, we cross 1dB at +2 and -2 degrees.  Therefore,
      //         we will limit it to more than 8 degrees from target room temperature.
      if ( ( tempDeltaFromRoomTemp > 8 ) || ( tempDeltaFromRoomTemp < -8 ) )
      {
         if ( chanIndex < NUM_ZIGBEE_CHANNELS )
         {
             /** @note - if we can't get fine calibration, then we
             *           DO NOT want to send to the KW2x, as we don't want
             *           any compensation to occur.  This way on new board
             *           they can calibrate without temperature compensation
             *           "chasing the tail".  Just clear the cal flags
             *           before calibration and it won't get sent.
             */
            if ( m_bFineCalValid )
            {
               // y = mx + b
               float m = m_TempCompVals[chanIndex].m;
               float b = m_TempCompVals[chanIndex].b;
               float delta_dBm = ( m * (float) tempDeltaFromRoomTemp ) + b;
               delta_dBm = -delta_dBm; // sign is reversed...
               if ( delta_dBm < 0 )
               {
                  // round away from zero
                  rounded_dBm = (int32_t)(delta_dBm - 0.5f);
               }
               else
               {
                  // round away from zero
                  rounded_dBm = (int32_t)(delta_dBm + 0.5f);
               }
               // just in case, lets limit to +-2dBm, there isn't much compensation.
               if (rounded_dBm > 2)
                  rounded_dBm = 2;
               if (rounded_dBm < -2)
                  rounded_dBm = -2;
            }
         }
      }
   }
   return rounded_dBm;
}
/***************************************************************************//**
* @file
*
*     This file provides CLI commands related to BSP subsystem.
*
* @note
*     None.
*
* @warning
*     None.
*******************************************************************************/
#include <stdio.h>
#include "ActiveMethod.hpp"
#include "assert.hpp"
#include "BspSubsys.hpp"
#include "CliCommand.hpp"
#include "CliPeekPokePuke.hpp"
#include "CliSubSys.hpp"
#include "DataSubsys.hpp"
#include "ICliCommand.hpp"
#include "ICliHandle.hpp"
#include "IZigBeeRadio.hpp"
#include "NvMfr.hpp"
#include "Task.hpp"
#include "ZigBeeRadioSubSys.hpp"
#define MEM_STATISTICS
#define MEM_TRACKING
#include "MemManager.h"
extern poolInfo_t poolInfo[ ];
extern pools_t memPools[ ];
#include "MCR20Reg.h" // only for register limits
#include "AspInterface.h"
#include "Phy.h"

extern phyCCAType_t mPhyCcaType;
extern bool         mOverrideCcaType;
namespace
{
   uint8_t m_TelecMode = gTestForceIdle_c;
   uint8_t m_TelecFreq = 15;
   /*******************************************************************************
   *  KW2x Direct Modem Peek/Poke/Puke Commands
   *******************************************************************************/
   class CDirectModemRegHelper : public CLI::IPeekPokePukeHelper
   {
   public:
      uint32_t       GetEndOffset() const       {  return IAR_INDEX;  }
      unsigned int   GetOffsetWidth() const     {  return 2;  }
      unsigned int   GetAlignment() const       {  return 1;  }
      Format_t       GetValueFormat() const     {  return FORMAT_HEX;  }
      unsigned int   GetValueWidth() const      {  return 2;  }
      unsigned int   GetColumns() const         {  return 16;  }
      bool Read ( uint32_t offset, uint32_t& value ) const
      {
         uint8_t value8;
         Asp_XcvrReadReq(false, offset, sizeof(value8), &value8);
         value = value8;
         return true;
      }
      bool Write ( uint32_t offset, uint32_t value ) const
      {
         uint8_t value8 = static_cast<uint8_t>(value);
         Asp_XcvrWriteReq(false, offset, 1, &value8 );
         return true;
      }
   } Kw2xDirectModemHelper;
   class CIndirectModemRegHelper : public CLI::IPeekPokePukeHelper
   {
   public:
      uint32_t       GetEndOffset() const       {  return SCAN_DTM_PROTECT_0;  }
      unsigned int   GetOffsetWidth() const     {  return 2;  }
      unsigned int   GetAlignment() const       {  return 1;  }
      Format_t       GetValueFormat() const     {  return FORMAT_HEX;  }
      unsigned int   GetValueWidth() const      {  return 2;  }
      unsigned int   GetColumns() const         {  return 16;  }
      bool Read ( uint32_t offset, uint32_t& value ) const
      {
         uint8_t value8;
         Asp_XcvrReadReq(true, offset, sizeof(value8), &value8);
         value = value8;
         return true;
      }
      bool Write ( uint32_t offset, uint32_t value ) const
      {
         uint8_t value8 = static_cast<uint8_t>(value);
         Asp_XcvrWriteReq(true, offset, 1, &value8 );
         return true;
      }
   } Kw2xIndirectModemHelper;
   bool zbdirpeek ( ICliHandle &handle, int argc, const std::string argv[] )   {  return CLI::PeekFunction ( handle, argc, argv, Kw2xDirectModemHelper );  }
   bool zbdirpoke ( ICliHandle &handle, int argc, const std::string argv[] )   {  return CLI::PokeFunction ( handle, argc, argv, Kw2xDirectModemHelper );  }
   bool zbindirpeek ( ICliHandle &handle, int argc, const std::string argv[] )   {  return CLI::PeekFunction ( handle, argc, argv, Kw2xIndirectModemHelper );  }
   bool zbindirpoke ( ICliHandle &handle, int argc, const std::string argv[] )   {  return CLI::PokeFunction ( handle, argc, argv, Kw2xIndirectModemHelper );  }

   /***************************************************************************//**
   * This routine actually handles the 100 hz test task in its own thread.
   *
   * @param[in]      dummy1       not used
   * @param[in]      dummy2       not used
   *
   * @note           none
   *
   * @warning        none
   *******************************************************************************/
   void HundredHzTestTask ( int32_t dummy1, int32_t dummy2 )
   {
      // lives on the stack...
      uint8_t testdata[] = "  100Hz test running         ";
      CTask::Sleep(100);
      while ( m_TelecMode == gTestContinuousTxExternalSrc_c )
      {
         testdata[0] = 38; // size
         ASP_TelecSendRawData( testdata );
         CTask::Sleep(4);
         ASP_TelecTest(gTestContinuousRx_c);
         CTask::Sleep(6);
      }
      // let task and object die...
   }
   /***************************************************************************//**
   * CLI command to set Telec test frequency.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  telecfreq( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      uint32_t channel;
      // Set telec channel
      if ( argc )
      {
         // Parse telec channel
         bool parse = false;
         if (  handle.ConvertInteger( argv[ 0 ], channel) && channel >= 11 && channel <= 26 )
            parse = true;
         else
            handle.printf( ICliHandle::InvalidSyntax );
         if ( parse )
         {
            m_TelecFreq = channel;
            if ( m_TelecMode != gTestForceIdle_c )
            {
               // only actually set the freq if really in telec mode
               ASP_TelecTest(gTestForceIdle_c);
               ASP_TelecSetFreq( m_TelecFreq );
               ASP_TelecTest(m_TelecMode);
            }
            ret = true;
         }
      }
      // Return status
      else
      {
         ret = true;
         handle.printf( "%d\n", m_TelecFreq );
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to set Telec test mode.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  telecmode( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Set mode
      if ( argc )
      {
         // Parse mode
         if       ( argv[ 0 ] == "off" )     m_TelecMode = gTestForceIdle_c;
         else if  ( argv[ 0 ] == "prbs9" )   m_TelecMode = gTestPulseTxPrbs9_c;
         else if  ( argv[ 0 ] == "txmod" )   m_TelecMode = gTestContinuousTxMod_c;
         else if  ( argv[ 0 ] == "txnomod" ) m_TelecMode = gTestContinuousTxNoMod_c;
         else if  ( argv[ 0 ] == "100hz" )   m_TelecMode = gTestContinuousTxExternalSrc_c;
         else if  ( argv[ 0 ] == "fsl" )     m_TelecMode = gTestContinuousTxModOne_c;
         else                               handle.printf( ICliHandle::InvalidSyntax );
         if ( gTestForceIdle_c == m_TelecMode )
         {
            PhyPpSetPromiscuous(false); // to clear out any test mode...
            ret = ( gAspSuccess_c == ASP_TelecTest(m_TelecMode) );
            handle.printf("Must start network using 'zigbee on' command\n");
         }
         else
         {
            // if starting up telec mode . . .
            if ( RADIO::GetRadioControlInterface( )->IsNetworkStarted() )
            {
               RADIO::GetRadioControlInterface( )->StopNetwork();
               CTask::Sleep(100);
            }
            ASP_TelecTest(gTestForceIdle_c);
            CTask::Sleep(10);
            ASP_TelecSetFreq( m_TelecFreq ); // restore frequency
            if ( gTestContinuousTxExternalSrc_c == m_TelecMode)
            {
               CStartAndForgetMethod testtask ( "100hztest", HundredHzTestTask, NULL );
               ret = true;
            }
            else if ( gTestContinuousTxModOne_c == m_TelecMode)
            {
               // FSL "special packet mode" TEST MODE ONLY
               PhyPpSetPromiscuous(true);       // receive all packets into PHY
               PhyPlmeSetRxOnWhenIdle(true, 0); // always on RX mode
               ret = true;
            }
            else
            {
               ret = ( gAspSuccess_c == ASP_TelecTest(m_TelecMode) );
            }
         }
      }
      // Return status
      else
      {
         ret = true;
         std::string modeStr;
         switch ( m_TelecMode )
         {
         case gTestForceIdle_c:               modeStr = "off";     break;
         case gTestPulseTxPrbs9_c:            modeStr = "prbs9";   break;
         case gTestContinuousTxMod_c:         modeStr = "txmod";   break;
         case gTestContinuousTxNoMod_c:       modeStr = "txnomod"; break;
         case gTestContinuousTxExternalSrc_c: modeStr = "100hz";   break;
         case gTestContinuousTxModOne_c:      modeStr = "fsl";     break;
         default:                             modeStr = "unknown"; break;
         }
         handle.printf( "%s\n", modeStr.c_str() );
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to get/set XTAL Trim.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  xtaltrim( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      uint8_t value8 = 0;
      if ( 0 == argc || "finish" == argv[0] )
      {
         Asp_XcvrReadReq( true, XTAL_TRIM, sizeof(value8), &value8 );
         // print current value
         if ( 0 == argc )
         {
            ret = true;
            handle.printf( "%d\n", value8 );
         }
         // set current value to EEPROM
         else
         {
            ret = DATA::Mfr().SetXtalTrim( value8 );
         }
      }
      // Set operating trim
      else
      {
         uint32_t trim = 0;
         if ( handle.ConvertInteger( argv[0], trim ) && trim < 256 )
         {
            value8 = trim;
            Asp_XcvrWriteReq( true, XTAL_TRIM, sizeof(value8), &value8 );
            ret = true;
         }
         else
         {
            handle.printf( ICliHandle::InvalidSyntax );
         }
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to turn on/off ZigBee radio.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zigbee( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Set enable
      if ( argc )
      {
         // Parse enable
         bool parse = true;
         bool enable = false;
         if       ( argv[ 0 ] == "on" )     enable = true;
         else if  ( argv[ 0 ] == "off" )    enable = false;
         else                               handle.printf( ICliHandle::InvalidSyntax );
         if ( parse )
         {
            if ( enable )
               ret = RADIO::GetRadioControlInterface( )->StartNetwork();
            else
               ret = RADIO::GetRadioControlInterface( )->StopNetwork();
         }
      }
      // Return status
      else
      {
         ret = true;
         if ( RADIO::GetRadioControlInterface( )->IsNetworkStarted() )
         {
            handle.printf( "on\n" );
         }
         else
         {
            handle.printf( "off\n" );
         }
      }
      return ret;
   }
  /***************************************************************************//**
   * CLI command to switch the ZigBee antenna.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbant( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Set antenna
      if ( argc )
      {
         // Parse antenna
         bool parse = true;
         int antenna = 0;
         if       ( argv[ 0 ] == "A" )     antenna = 0;
         else if  ( argv[ 0 ] == "a" )     antenna = 0;
         else if  ( argv[ 0 ] == "B" )     antenna = 1;
         else if  ( argv[ 0 ] == "b" )     antenna = 1;
         else                              handle.printf( ICliHandle::InvalidSyntax );
         if ( parse )
         {
            ret = ( gAspSuccess_c == Asp_SetANTXState(antenna) );
         }
      }
      // Return status
      else
      {
         ret = true;
         uint8_t antx = Asp_GetANTXState();
         if ( antx )
         {
            handle.printf( "B\n" );
         }
         else
         {
            handle.printf( "A\n" );
         }
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to override the ZigBee CCA mode.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbccamode( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Set mode
      if ( argc )
      {
         // Parse mode
         bool parse = false;
         uint32_t mode = gPhyCCAMode1_c;
         if (  handle.ConvertInteger( argv[ 0 ], mode) && mode > gPhyEnergyDetectMode_c && mode < gPhyNoCCABeforeTx_c )
            parse = true;
         else
            handle.printf( ICliHandle::InvalidSyntax );
         if ( parse )
         {
            mPhyCcaType = static_cast<phyCCAType_t>(mode);
            mOverrideCcaType = true;
            ret = true;
         }
      }
      // Return status
      else
      {
         ret = true;
         if ( mOverrideCcaType )
         {
            handle.printf( "Mode: %d\n", mPhyCcaType );
         }
         else
         {
            handle.printf( "Not overriden, stack Default\n" );
         }
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to override the ZigBee CCA mode.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbccathresh( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      int32_t threshold = 0;
      uint8_t value8 = 0;
      // Set threshold
      if ( argc )
      {
         // Parse threshold
         bool parse = false;
         if (  handle.ConvertInteger( argv[ 0 ], threshold) && threshold < -20 && threshold > -100 )
            parse = true;
         else
            handle.printf( ICliHandle::InvalidSyntax );
         if ( parse )
         {
            value8 = (threshold * -1);
            Asp_XcvrWriteReq(true, CCA1_THRESH, sizeof(value8), &value8);
            ret = true;
         }
      }
      // Return status
      else
      {
         ret = true;
         Asp_XcvrReadReq(true, CCA1_THRESH, sizeof(value8), &value8);
         threshold = value8 * -1; // make negative
         handle.printf( "%d\n", threshold );
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to get ZigBee Energy Detect Level on the current telec frequency.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbed( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      int32_t edLevel;
      if ( !RADIO::GetRadioControlInterface( )->IsNetworkStarted() )
      {
         // Get Level
         ret = true;
         ASP_TelecSetFreq( m_TelecFreq ); // Set the modem to the telec frequency
         edLevel = ShureEdRequestTest (  );
         edLevel *= -1; // make negative... (-X dBm)
         handle.printf( "%d\n", edLevel );
      }
      else
      {
         handle.printf("Must stop network using 'zigbee off' command\n");
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to set ZigBee PA/LNA on or bypass.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbpalna( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Set enable
      if ( argc )
      {
         // Parse enable
         bool parse = false;
         bool enable = false;
         if ( argv[ 0 ] == "on" )
         {
            enable = true;
            parse = true;
         }
         else if ( argv[ 0 ] == "bypass" )
         {
            enable = false;
            parse = true;
         }
         else
         {
            handle.printf( ICliHandle::InvalidSyntax );
         }
         if ( parse )
         {
            RADIO::GetRadioControlInterface()->SetPaLnaEnable(enable);
            ret = true;
         }
      }
      // Return status
      else
      {
         ret = true;
         if ( RADIO::GetRadioControlInterface()->GetPaLnaEnable() )
         {
            handle.printf( "on\n" );
         }
         else
         {
            handle.printf( "bypass\n" );
         }
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to set ZigBee radio power level.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbpwr( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      uint32_t powerLevel;
      // Set power
      if ( argc )
      {
         // Parse power level
         bool parse = false;
         if (  handle.ConvertInteger( argv[ 0 ], powerLevel) && powerLevel >= 3 && powerLevel <= 31 )
            parse = true;
         else
            handle.printf( ICliHandle::InvalidSyntax );
         if ( parse )
         {
               RADIO::GetRadioControlInterface( )->SetRadioPowerLevel( powerLevel );
               ret = true;
         }
      }
      // Return status
      else
      {
         ret = true;
         powerLevel = RADIO::GetRadioControlInterface( )->GetRadioPowerLevel(  );
         handle.printf( "%d\n", powerLevel );
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to set ZigBee radio fine power level.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbfinepwr( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      uint32_t finePowerLevel;
      // Set power
      if ( argc )
      {
         // Parse fine power level
         bool parse = false;
         if (  handle.ConvertInteger( argv[ 0 ], finePowerLevel) && finePowerLevel >= MIN_FINE_POWER && finePowerLevel <= MAX_FINE_POWER )
            parse = true;
         else
            handle.printf( ICliHandle::InvalidSyntax );
         if ( parse )
         {
               RADIO::GetRadioControlInterface( )->SetRadioFinePowerLevel( finePowerLevel );
               ret = true;
         }
      }
      // Return status
      else
      {
         ret = true;
         finePowerLevel = RADIO::GetRadioControlInterface( )->GetRadioFinePowerLevel(  );
         handle.printf( "%d\n", finePowerLevel );
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to print memory usage for zigbee stack.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         True = success; false = fail;
   *
   * @note           MEM_TRACKING and MEM_STATISTICS must be #define'd in
   *                 MemManager.c for this command to function.
   *******************************************************************************/
   bool  zbmem( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      handle.printf( "BlockSize  NumBlocks  Alloc  AllocPeak  AllocFail  FreeFail  FragWaste  FragWastePeak\n" );
      pools_t *pool = memPools;
      for ( poolInfo_t *info = poolInfo;
            info->blockSize; ++info, ++pool )
      {
         poolStat_t __packed *stat = &pool->poolStatistics;
         handle.printf( "%9d  %9d  %5d  %9d  %9d  %8d  %9d  %13d\n",
               info->blockSize, stat->numBlocks,
               stat->allocatedBlocks, stat->allocatedBlocksPeak,
               stat->allocationFailures, stat->freeFailures,
               stat->poolFragmentWaste, stat->poolFragmentWastePeak );
      }
      ret = true;
      return ret;
   }
   /***************************************************************************//**
   * CLI command to set ZigBee radio power mode.
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbpwrmode( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Set power
      if ( argc )
      {
         if ( argv[ 0 ] == "mid" )
         {
            ret = RADIO::GetRadioControlInterface()->SetRadioPowerMode( IZigBeeRadioController::ZIGBEE_PWR_MODE_MID );
         }
         else if ( argv[ 0 ] == "low" )
         {
            ret = RADIO::GetRadioControlInterface()->SetRadioPowerMode( IZigBeeRadioController::ZIGBEE_PWR_MODE_LOW );
         }
//! @note - JP 9/28/2017. Remove High power support from CLI. This will cause
//                        the command to fail and then alert users of CLI
//                        (e.g. ATE) of the error in their ways.  We do this
//                        for CLI command as other callers of radio power mode
//                        we simply want to limit it to mid power.
//         else if ( argv[ 0 ] == "high" )
//         {
//            ret = RADIO::GetRadioControlInterface()->SetRadioPowerMode( IZigBeeRadioController::ZIGBEE_PWR_MODE_HIGH );
//         }
         else
         {
            handle.printf( ICliHandle::InvalidSyntax );
         }
      }
      // Return status
      else
      {
         IZigBeeRadioController::ZigbeePowerMode_t mode = RADIO::GetRadioControlInterface()->GetRadioPowerMode();
         if ( IZigBeeRadioController::ZIGBEE_PWR_MODE_MID == mode )
         {
            handle.printf( "mid\n" );
            ret = true;
         }
         else if ( IZigBeeRadioController::ZIGBEE_PWR_MODE_LOW == mode )
         {
            handle.printf( "low\n" );
            ret = true;
         }
         else if ( IZigBeeRadioController::ZIGBEE_PWR_MODE_HIGH == mode )
         {
            handle.printf( "high\n" );
            ret = true;
         }
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command to set/get Zigbee diversity
   *
   * @param [in]     handle      CLI session handle
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool  zbdiv( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Set diversity
      if ( argc )
      {
         bool parse = false;
         IZigBeeRadioController::AntDiversityMode_t mode = IZigBeeRadioController::DIVERSITY_DISABLE;
         if ( argv[0] == "on" ) { mode = IZigBeeRadioController::DIVERSITY_ENABLE; parse = true; }
         else if ( argv[0] == "off" ) { mode = IZigBeeRadioController::DIVERSITY_DISABLE; parse = true; }
         if ( parse )
         {
            ret = RADIO::GetRadioControlInterface()->SetDiversityMode ( mode );
         }
         else
         {
            handle.printf( ICliHandle::InvalidSyntax );
         }
      }
      // Return diversity
      else
      {
         IZigBeeRadioController::AntDiversityMode_t mode =  RADIO::GetRadioControlInterface()->GetDiversityMode();
         {
            handle.printf( "%s\n",
                          ( IZigBeeRadioController::DIVERSITY_ENABLE == mode ) ? "on" : "off" );
            ret = true;
         }
      }
      return ret;
   }
   /***************************************************************************//**
   * CLI command for reading/writing zigbee power calibration.
   *
   * @param [in]     handle      CLI session handle.
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool zbchpwr( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Get idx for power cal
      NvMfr::ZigbeePowerModeIdx_t modeIdx;
      switch ( RADIO::GetRadioControlInterface()->GetRadioPowerMode() )
      {
         case IZigBeeRadioController::ZIGBEE_PWR_MODE_MID:   modeIdx = NvMfr::ZB_PWR_FCC_IDX;        break;
         case IZigBeeRadioController::ZIGBEE_PWR_MODE_LOW:   modeIdx = NvMfr::ZB_PWR_ETSI_LOW_IDX;   break;
         case IZigBeeRadioController::ZIGBEE_PWR_MODE_HIGH:  modeIdx = NvMfr::ZB_PWR_ETSI_HIGH_IDX;  break;
      }
      if ( argc )
      {
         if ( argv[ 0 ] == "crc" )
         {
            ret = DATA::Mfr().FinalizeZigbeeCalibration();
         }
         else
         {
            // Write power values
            ret = true;
            const char* p = &argv[0][0];
            for ( int chan = 0; chan < NvMap::ZigbeeCoarseCal::Data::NUM_CHANNELS; ++chan )
            {
               // strtol is defined as char** even though it doesn't modify input string.
               // http://stackoverflow.com/questions/3874196/why-is-the-endptr-parameter-to-strtof-and-strtod-a-pointer-to-a-non-const-char-p
               uint8_t pow = static_cast<uint8_t>( strtol( p, const_cast<char**>( &p ), 10 ) );
               // Check formatting of input. Break on error. For first 15 args, we should be
               // on a comma after call to strtol. On last call, we'll be in uninit memory,
               // as std::string doesn't store NULL
               if ( chan < NvMap::ZigbeeCoarseCal::Data::NUM_CHANNELS - 1 )
               {
                  if ( *p != ',' )
                  {
                     ret = false;
                     break;
                  }
                  else
                  {
                     p++;
                  }
               }
               // Program power if valid
               if ( pow < MAX_RADIO_POWER )
               {
                  ret &= DATA::Mfr().SetZigbeePowerCalibration( modeIdx, chan, pow );
               }
               else
               {
                  ret = false;
                  break;
               }
            }
         }
      }
      else
      {
         // Check calibration
         if ( !DATA::Mfr().IsZigbeeCalibrationValid() )
         {
            handle.printf( "Zigbee calibration not valid! Corrupt or uninitialized.\n" );
         }
         // Print calibration
         uint8_t power;
         for ( int chan = 0; chan < NvMap::ZigbeeCoarseCal::Data::NUM_CHANNELS - 1; ++chan )
         {
            DATA::Mfr().GetZigbeePowerCalibration( modeIdx, chan, power );
            handle.printf("%d,", power );
         }
         DATA::Mfr().GetZigbeePowerCalibration( modeIdx, NvMap::ZigbeeCoarseCal::Data::NUM_CHANNELS - 1, power );
         handle.printf("%d\n", power );
         ret = true;
      }
      return ret;
   }
   bool zbch( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = true;
      uint8_t chan = RADIO::GetRadioControlInterface()->GetRadioChannel();
      handle.printf("%d\n", chan );
      return ret;
   }
   /***************************************************************************//**
   * CLI command for reading/writing zigbee fine power calibration.
   *
   * @param [in]     handle      CLI session handle.
   * @param [in]     argc        Number of arguments to the command.
   * @param [in]     argv        Array of argument strings.
   *
   * @return         bool        True = success; false = fail;
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   bool zbchfinepwr( ICliHandle &handle, int argc, const std::string argv[ ] )
   {
      bool ret = false;
      // Get idx for power cal
      NvMfr::ZigbeePowerModeIdx_t modeIdx;
      switch ( RADIO::GetRadioControlInterface()->GetRadioPowerMode() )
      {
         case IZigBeeRadioController::ZIGBEE_PWR_MODE_MID:   modeIdx = NvMfr::ZB_PWR_FCC_IDX;        break;
         case IZigBeeRadioController::ZIGBEE_PWR_MODE_LOW:   modeIdx = NvMfr::ZB_PWR_ETSI_LOW_IDX;   break;
         case IZigBeeRadioController::ZIGBEE_PWR_MODE_HIGH:  modeIdx = NvMfr::ZB_PWR_ETSI_HIGH_IDX;  break;
      }
      if ( argc )
      {
         if ( argv[ 0 ] == "crc" )
         {
            if ( DATA::Mfr().SetZigbeeFinePowerCalibrationTemp( BSP::GetMcuTemperatureC() ) )
            {
               ret = DATA::Mfr().FinalizeZigbeeFineCalibration();
            }
         }
         else
         {
            // Write power values
            ret = true;
            const char* p = &argv[0][0];
            for ( int chan = 0; chan < NvMap::ZigbeeFineCal::Data::NUM_CHANNELS; ++chan )
            {
               // strtol is defined as char** even though it doesn't modify input string.
               // http://stackoverflow.com/questions/3874196/why-is-the-endptr-parameter-to-strtof-and-strtod-a-pointer-to-a-non-const-char-p
               uint8_t pow = static_cast<uint8_t>( strtol( p, const_cast<char**>( &p ), 10 ) );
               // Check formatting of input. Break on error. For first 15 args, we should be
               // on a comma after call to strtol. On last call, we'll be in uninit memory,
               // as std::string doesn't store NULL
               if ( chan < NvMap::ZigbeeFineCal::Data::NUM_CHANNELS - 1 )
               {
                  if ( *p != ',' )
                  {
                     ret = false;
                     break;
                  }
                  else
                  {
                     p++;
                  }
               }
               // Program power if valid
               if ( pow >= MIN_FINE_POWER && pow <= MAX_FINE_POWER )
               {
                  ret &= DATA::Mfr().SetZigbeeFinePowerCalibration( modeIdx, chan, pow );
               }
               else
               {
                  ret = false;
                  break;
               }
            }
         }
      }
      else
      {
         // Check calibration
         if ( !DATA::Mfr().IsZigbeeFineCalibrationValid() )
         {
            handle.printf( "Zigbee calibration not valid! Corrupt or uninitialized.\n" );
         }
         // Print temp
         int16_t tempC = 0;
         DATA::Mfr().GetZigbeeFinePowerCalibrationTemp( tempC );
         handle.printf( "Temp %dC\n", tempC );
         // Print calibration
         uint8_t power;
         for ( int chan = 0; chan < NvMap::ZigbeeFineCal::Data::NUM_CHANNELS - 1; ++chan )
         {
            DATA::Mfr().GetZigbeeFinePowerCalibration( modeIdx, chan, power );
            handle.printf("%d,", power );
         }
         DATA::Mfr().GetZigbeeFinePowerCalibration( modeIdx, NvMap::ZigbeeFineCal::Data::NUM_CHANNELS - 1, power );
         handle.printf("%d\n", power );
         ret = true;
      }
      return ret;
   }
   /*! Command syntax */
   const char telecfreqSyntax[ ]          = "[ { chan (11-26) } ]";
   const char telecmodeSyntax[ ]          = "[ { off | prbs9 | txmod | txnomod | 100hz | fsl } ]";
   const char xtaltrimSyntax[ ]           = "[ { trim (0-255) | finish } ]";
   const char zbantSyntax[ ]              = "[ { A | B } ]";
   const char zbccamodeSyntax[ ]          = "[ { 1 | 2 | 3 } ]";
   const char zbchpwrSyntax[ ]            = "[ crc | [<pow0>,<pow1>,...,<pow15>] ]";
   const char zbdivSyntax[ ]              = "[ { on | off } ]";
   const char zbpalnaSyntax[ ]            = "[ { on | bypass  } ]";
   const char zbpwrmodeSyntax[ ]          = "[ { mid | low | high } ]";
   const char zbpwrSyntax[ ]              = "[ { power (3-31) } ]";
   const char zbfinepwrSyntax[ ]          = "[ { fine power (32-47; 40 is midpoint) } ]";
   const char zigbeeSyntax[ ]             = "[ { on | off  } ]";
   /*! Command help */
   const char  telecfreqHelp[ ]           = "Set telec channel.\n"
                                            "Returns current channel if no args supplied.";
   const char  telecmodeHelp[ ]           = "Set telec mode.\n"
                                            "Returns current mode if no args supplied.";
   const char  xtaltrimHelp[ ]            = "Set crystal trim.\n"
                                            "'finish' will set the current value into EEPROM\n"
                                            "Returns current crystal trim if no args supplied.";
   const char  zbDirPeekHelp[ ]           = "Display a direct register value from the KW2x modem.";
   const char  zbDirPokeHelp[ ]           = "Set a KW2x direct modem register value.";
   const char  zbInDirPeekHelp[ ]         = "Display an indirect register value from the KW2x modem.";
   const char  zbInDirPokeHelp[ ]         = "Set a KW2x indirect modem register value.";
   const char  zbantHelp[ ]               = "Set ZigBee antenna.\n"
                                            "Returns ZigBee antenna if no args supplied.";
   const char  zbccamodeHelp[ ]           = "Override CCA Mode.\n"
                                            "Returns CCA mode if no args supplied.";
   const char  zbccathreshHelp[ ]         = "Set CCA threshold.\n"
                                            "Returns CCA threshold if no args supplied.";
   const char  zbchHelp[ ]                = "Get zigbee channel.\n";
   const char  zbchpwrHelp[ ]             = "Get/set zigbee power calibration.\n"
                                            "  To set, specify all powers in comma-delimited string";
   const char  zbdivHelp[ ]               = "Turn zigbee diversity on/off.\n"
                                            "Returns zigbee diversity status if no args.";
   const char  zbedHelp[ ]                = "Get ZigBee Energy Detect Level on the current telecfreq.\n";
   const char  zbmemHelp[ ]               = "Print memory usage for zigbee stack.\n"
                                            "Zigbee stack must be compiled with MEM_DEBUG options.";
   const char  zbpalnaHelp[ ]             = "Set ZigBee PA/LNA on or bypass.\n"
                                            "Returns ZigBee PA/LNA status if no args supplied.";
   const char  zbpwrHelp[ ]               = "Set ZigBee radio power.\n"
                                            "Returns ZigBee power level if no args supplied.";
   const char  zbfinepwrHelp[ ]           = "Set ZigBee Fine radio power\n"
                                            "Returns ZigBee Fine power level if no args supplied.";
   const char  zbpwrmodeHelp[ ]           = "Set ZigBee radio power cal mode.\n"
                                            "Returns ZigBee power cal mode if no args supplied.";
   const char  zigbeeHelp[ ]              = "Turn on/off ZigBee radio.\n"
                                            "Returns ZigBee radio status if no args supplied, otherwise turn on off.";
   /*! BSP command table. */
   const CliCmdEntry_t  g_RadioCmdTable[ ]  =
   {
      {  "telecfreq",    0,  1,  telecfreq,    telecfreqSyntax,  telecfreqHelp,    ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "telecmode",    0,  1,  telecmode,    telecmodeSyntax,  telecmodeHelp,    ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "xtaltrim",     0,  1,  xtaltrim,     xtaltrimSyntax,   xtaltrimHelp,     ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbant",        0,  1,  zbant,        zbantSyntax,      zbantHelp,        ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbccamode",    0,  1,  zbccamode,    zbccamodeSyntax,  zbccamodeHelp,    ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbccathresh",  0,  1,  zbccathresh,  "",               zbccathreshHelp,  ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbch",         0,  1,  zbch,         "",               zbchHelp,         ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbchfinepwr",  0,  1,  zbchfinepwr,  zbchpwrSyntax,    zbchpwrHelp,      ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbchpwr",      0,  1,  zbchpwr,      zbchpwrSyntax,    zbchpwrHelp,      ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbdirpeek",    1,  1,  zbdirpeek,    CLI::peekSyntax,  zbDirPeekHelp,    ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbdirpoke",    2,  2,  zbdirpoke,    CLI::pokeSyntax,  zbDirPokeHelp,    ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbdiv",        0,  1,  zbdiv,        zbdivSyntax,      zbdivHelp,        ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbed",         0,  0,  zbed,         "",               zbedHelp,         ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbindirpeek",  1,  1,  zbindirpeek,  CLI::peekSyntax,  zbInDirPeekHelp,  ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbindirpoke",  2,  2,  zbindirpoke,  CLI::pokeSyntax,  zbInDirPokeHelp,  ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbmem",        0,  0,  zbmem,        "",               zbmemHelp,        ( CLI::GROUP_DEV )                   },
      {  "zbpalna",      0,  1,  zbpalna,      zbpalnaSyntax,    zbpalnaHelp,      ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbpwr",        0,  1,  zbpwr,        zbpwrSyntax,      zbpwrHelp,        ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbfinepwr",    0,  1,  zbfinepwr,    zbfinepwrSyntax,  zbfinepwrHelp,    ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zbpwrmode",    0,  1,  zbpwrmode,    zbpwrmodeSyntax,  zbpwrmodeHelp,    ( CLI::GROUP_MFR | CLI::GROUP_DEV )  },
      {  "zigbee",       0,  1,  zigbee,       zigbeeSyntax,     zigbeeHelp,       ( CLI::GROUP_MFR | CLI::GROUP_DEV )  }
   };
   const uint32_t g_RadioCmdTableSize = sizeof( g_RadioCmdTable ) / sizeof( CliCmdEntry_t );
}
namespace RADIO
{
   /***************************************************************************//**
   * Register firmware update subsystem CLI commands
   *
   * @note
   *     None.
   *
   * @warning
   *     None.
   *******************************************************************************/
   void  RegisterRadioCliCommands( )
   {
      CLI::GetCliCommandInterface( )->AddCmdTable( g_RadioCmdTable, g_RadioCmdTableSize, "radio" );
   }
}
/***************************************************************************//**
* @file
*
*     This file implements the ZigBee radio subsystem.
*
* @note
*     None.
*
* @warning
*     None.
*******************************************************************************/
#include "ZigBeeRadioSubSys.hpp"
#include "assert.hpp"
#include "BspSubSys.hpp"
#include "DataSubsys.hpp"
#include "IModelController.hpp"
#include "NvMfr.hpp"
#include "RadioCliCmds.hpp"
#include "ZigBeeRadio.hpp"
namespace
{
   cZigBeeRadio*       m_pRadio = NULL;
}
/***************************************************************************//**
* Initializes the ZigBee radio subsystem.
*
* @note
*     None.
*
* @warning
*     None.
*******************************************************************************/
void  RADIO::Init( )
{
   if ( DATA::GetModelController()->IsEnhancedTransmitter( ) )
   {
      m_pRadio = new cZigBeeRadio( 0, NULL );
      /* one time operation to get the stack going */
      m_pRadio->InitStack();
   }
}

/***************************************************************************//**
* Start the tasks for the ZigBee radio.
*
* @note
*     None.
*
* @warning
*     None.
*******************************************************************************/
void  RADIO::Start( )
{
   if ( DATA::GetModelController()->IsEnhancedTransmitter( ) )
   {
      // Get EUI-64 from NV memory
      uint64_t nExtAddr = 0;
      if ( DATA::Mfr( ).GetEui( nExtAddr ) )
      {
         // Program EUI into radio
         m_pRadio->SetExtendedAddress( nExtAddr );
         // Start the zigbee network
         m_pRadio->StartNetwork( );
      }
      else
      {
         ASSERT_INFO( "Failed to get EUI! Can't start radio!" );
      }
      // Register ZigBee radio as background task
      BSP::RegisterBackgroundTask( *m_pRadio );
      // Register CLI for enhanced
      RegisterRadioCliCommands();
   }
}
/***************************************************************************//**
* Returns the Radio Control interface.
*
* @return       The address of the Radio Control Interface.
*
* @note          none
*
* @warning       none
*******************************************************************************/
IZigBeeRadioController* RADIO::GetRadioControlInterface()
{
   return m_pRadio;
}

/***************************************************************************//**
* Returns the Radio request sender interface.
*
* @return       The address of the Radio request sender interface.
*
* @note          none
*
* @warning       none
*******************************************************************************/
IZigBeeRequestSender* RADIO::GetRequestSenderInterface()
{
   return m_pRadio;
}
/***************************************************************************//**
* @file
*
*     This file contains the main() function.
*
* @note
*     None.
*
* @warning
*     None.
*******************************************************************************/
#include <stdint.h>
#include <stdio.h>
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
void  AppMain( );
static xTaskHandle AppMainHandle;

/***************************************************************************//**
* Temporary workaround for TX unaligned access across the SRAM regions.
*
* @note          none
*
* @warning       none
*******************************************************************************/
#include <stdlib.h>
uint32_t TxMallocWorkaroundSuccess = 0; // global variable
uint8_t * TxMallocWorkaroundPtr = NULL;
void TxMallocWorkaround(void)
{
   /* 
      The purpose of this function is to keep our application HEAP from using the
      memory right at the boundary of SRAM_U and SRAM_L (0x2000_0000). If the
      HEAP hands out memory crossing this boundary - we can't control how
      the application uses it. If it uses it in an unaligned manner, it will
      crash with a hard fault.
   
      This example code illustrates the problem:
   
      memcpy((void*)0x20000000, (void*)0x1FFF8000, 1024);   // OK:  copy from beginning of SRAM_L to beginning of SRAM_U
      memcpy((void*)0x1fffff00, (void*)0x1FFF8000, 1024);   // OK:  copy from beginning of SRAM_L to end of SRAM_L such that it crosses SRAM_U begin boundary
      memcpy((void*)0x20000001, (void*)0x1FFF8000, 1024);   // OK:  unaligned copy from beginning of SRAM_L to beginning of SRAM_U
      memcpy((void*)0x1fffff01, (void*)0x1FFF8000, 1024);   // BAD: unaligned  copy from beginning of SRAM_L to end of SRAM_L such that it crosses SRAM_U begin boundary
  
      The last memcpy will cause a hard-fault.  Again, if HEAP gives out
      a chunk of memory crossing the boundary - we can't guarantee how the
      application will use it (aligned, unaligned).  If the application uses
      it in an unaligned way, the processor will hard fault.
   
      There are some solutions to this in .icf - but this only works if the 
      HEAP memory is small enough to fit entirely in one of the regions.
      The WAP is a good example of that. 
   
      With the TX heap being larger than one single region, 
      we are left with only option of keeping it looking like a big region 
      and letting the HEAP take it.  But that leaves us exposed to HEAP
      handing out the problem area. What this function does is use 
      malloc to allocation memory up until the problem area, then we will
      allocate memory right across the problem area.  Once we do that, we can 
      free all the other memory except the problem area. We keep that allocated
      and never use it for any other purpose.  This is done in early application
      initialization before anyone else needs the HEAP.
   */
   
   const uint32_t BOUNDARY_ADDR = 0x20000000; // divider between SRAM_L and SRAM_U
   const uint32_t ALLOC_BYTES   = 64;        // enough to cross the boundary with some margin
   
   // The assumption here is dlmalloc allocates in contiguous fashion from lowest to highest.
   
   uint8_t * ptrA = malloc(4); // allocate a few bytes and see what address it gives us...
   
   uint32_t size = BOUNDARY_ADDR - (uint32_t)ptrA; // how much do we need to allocate right to the boundary.
   
   size -= (ALLOC_BYTES/2);  // a few bytes less
   
   uint8_t * ptrB = malloc (size); // this should allocate a chunk of memory almost right up to the boundary.
   
   TxMallocWorkaroundPtr = malloc (ALLOC_BYTES);  // should allocate right over the boundary, starting before and ending up after.
   
   // check to make sure that we allocated a chunk of memory that starts right near
   // the boundary and crosses it.
   if ( (uint32_t)TxMallocWorkaroundPtr < BOUNDARY_ADDR && (uint32_t)TxMallocWorkaroundPtr > (BOUNDARY_ADDR - ALLOC_BYTES ) )
   {
      TxMallocWorkaroundSuccess = 1; // good - set global variable.
   }
   
   free (ptrA);
   free (ptrB);
   // never free TxMallocWorkaroundPtr!!! we want to forever remove it from use.
}

/***************************************************************************//**
* First called task after scheduler is started. Calls AppMain within the context
* of this task.
*
* @param[in]     unused - unused argument to fulfill FreeRTOS signature
*
* @note          AppMain will be called within the context of this task. The
*                task stack size for AppMain() should be appropriate.
*
* @warning       none
*******************************************************************************/
static void usrAppInit(void * unused)
{
  (void) unused;
   AppMain( );
   vTaskDelete(NULL); /* When AppMain returns, MUST delete self after execution */
}

/***************************************************************************//**
* Application main entry point. Creates AppMain task and starts the scheduler.
*
* @note          none
*
* @warning       none
*******************************************************************************/
int main( )
{
   static const int APP_MAIN_STACK = 0x800;
   
   TxMallocWorkaround();
   // Create the main application OpenRTOS-supervised task.  It will be the first and
   // only task at this point.  It's created as "ready", but will not run until the
   // scheduler is started.
   //
   // According to OpenRTOS xTaskCreate documentation, "Tasks can be created both before
   // and after the scheduler has been started."
   xTaskCreate( usrAppInit, "tAppMain", (APP_MAIN_STACK / sizeof(uint32_t) ), 0, configMAX_PRIORITIES / 2, &AppMainHandle );

   // Now start the OpenRTOS scheduler, and thus the tAppMain task.
   vTaskStartScheduler();
   /* If all is well, the scheduler will now be running, and the following line
   will never be reached.  If the following line does execute, then there was
   insufficient FreeRTOS heap memory available for the idle and/or timer tasks
   to be created.  See the memory management section on the FreeRTOS web site
   for more details. */
   /* No actions to perform after this so wait forever */
   for( ;; );
}

/***************************************************************************//**
* FreeRTOS hook for stack overflow.
*
* @note          See configCHECK_FOR_STACK_OVERFLOW
*
* @warning       none
*******************************************************************************/
void vApplicationStackOverflowHook( xTaskHandle pxTask, signed char *pcTaskName )
{
  ( void ) pcTaskName;
  ( void ) pxTask;
  /* Run time stack overflow checking is performed if
     configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
     function is called if a stack overflow is detected. */
  taskDISABLE_INTERRUPTS();
  for( ; ; )
  {
  }
} /* vApplicationStackOverflowHook */

/***************************************************************************//**
* FreeRTOS hook for idle loop.
*
* @note          See configUSE_IDLE_HOOK
*
* @warning       none
*******************************************************************************/
void vApplicationIdleHook( void )
{
}
/***************************************************************************//**
* IAR dlmalloc hook.
*
* @return          None
*
* @note
*    Locks dlmalloc
*
* @warning
*    None
*******************************************************************************
*/
void __iar_Locksyslock_Malloc(void)
{
   vTaskSuspendAll();
}
/***************************************************************************//**
* IAR dlmalloc hook.
*
* @return          None
*
* @note
*    Unlocks dlmalloc
*
* @warning
*    None
*******************************************************************************
*/
void __iar_Unlocksyslock_Malloc(void)
{
   xTaskResumeAll();
}
/***************************************************************************//**
* IAR __exit hook.
*
* @return          None
*
* @note
*    Catches all calls to __exit
*
* @warning
*    None
********************************************************************************/
void __exit(int exit_code)
{
   printf("**** Task: %s Suspended because of __exit called with code 0x%x ****\n",
         pcTaskGetTaskName(NULL), exit_code);
   // Suspend this thread
   vTaskSuspend(NULL);
}
/***************************************************************************//**
* @file
*  Start is executed at reset vector. It defines boot procedure from reset vector
*  to main.
*
* @note
*    None
*
* @warning
*    None
*******************************************************************************/
#include <stdint.h>
#include "sysLib.h"
#include "FreeRTOS.h"
#include "task.h"
// External functions
extern void main( );
extern void __iar_data_init3( );
/***************************************************************************//**
* System startup. This function is to be called from reset vector. It initializes
* hardware resources and C/C++ runtime and then jumps to main.
*
* @note
*    None.
*
* @warning
*    None.
*******************************************************************************/
void Start( )
{
   // Shure low-level initialization of processor, clocks and addressable memory.
   // This just gets enough processor, clocks, and pin configurations performed so
   // that data segments defined by the load module may be initialized in the next step.
   SysLibInit( );
   // Call IAR specific startup which would automatically copy & initialize sections
   __iar_data_init3( );
   // Now that MCU, clocks, address space and program sections are in place,
   // initialize the MCU interfaces.
   // Shure low-level initialization of peripheral devices that must be done before
   // main app begins. E.G. pins may be configured for default states.
   SysLibInit2( );
   /* Jump to main process */
   main( );
   // NEVER returns.  (But if it does -- wait FOREVER)
   while ( 1 );
}


