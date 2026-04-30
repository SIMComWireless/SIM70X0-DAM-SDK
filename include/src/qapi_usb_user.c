/*==============================================================================

qapi_USB_user_cb_dispatcher

GENERAL DESCRIPTION
This file contains definition of User callback dispatcher.


Copyright (c) 2020  by Qualcomm Technologies, Inc.  All Rights Reserved.
==============================================================================*/

/*==============================================================================

EDIT HISTORY FOR MODULE

This section contains comments describing changes made to the module.
Notice that changes are listed in reverse chronological order.

$Header: //components/rel/core.tx/5.0/wiredconnectivity/hsusb/driver/src/adapt_layers/qapi_usb_user.c#1 $
$DateTime: 2020/03/12 06:06:49 $ $Author: pwbldsvc $
2009/01/13 04:17:02 $ $Author: pwbldsvc $

when      who     what, where, why
--------  ---     ------------------------------------------------------------
03/04/20  cs      Initial version

==============================================================================*/

#include "qapi_usb.h"


#ifdef QAPI_TXM_MODULE

/* CALLBACK DISPATCHER */
int qapi_USB_user_cb_dispatcher(UINT cb_id, void *app_cb,
                                     UINT cb_param1, UINT cb_param2,
                                     UINT cb_param3, UINT cb_param4,
                                     UINT cb_param5, UINT cb_param6,
                                     UINT cb_param7, UINT cb_param8)
{

/*SIMCOM mengxiangning added for qapi usb does not work @20180424 begin*/
  #ifdef  __SIMCOM_QAPI_USB__
  void (*pfn_app_cb1) (uint16_t number);
  #else
/*SIMCOM mengxiangning added for qapi usb does not work @20180424 end*/
  void (*pfn_app_cb1) (void);
/*SIMCOM mengxiangning added for qapi usb does not work @20180424 begin*/
  #endif
/*SIMCOM mengxiangning added for qapi usb does not work @20180424 end*/
  void (*pfn_app_cb2) (void);

  if(cb_id == TXM_QAPI_USB_RX_CB) // app_rx_cb
  {
/*SIMCOM mengxiangning added for qapi usb does not work @20180424 begin*/
  #ifdef  __SIMCOM_QAPI_USB__
      pfn_app_cb1 = (void (*)(uint16_t number))app_cb;
      (pfn_app_cb1)(cb_param1);
  #else
/*SIMCOM mengxiangning added for qapi usb does not work @20180424 begin*/
      pfn_app_cb1 = (void (*)(void))app_cb;
      (pfn_app_cb1)();
/*SIMCOM mengxiangning added for qapi usb does not work @20180424 begin*/
  #endif
/*SIMCOM mengxiangning added for qapi usb does not work @20180424 end*/
  }
  else if(cb_id == TXM_QAPI_USB_DISC_CB) // app_disconnect_cb
  {
      pfn_app_cb2 = (void (*)(void))app_cb;
      (pfn_app_cb2)();
  }

  return 0;
}

#endif
