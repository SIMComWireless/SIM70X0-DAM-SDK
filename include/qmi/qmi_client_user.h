#ifndef QMI_CLIENT_USER_H
#define QMI_CLIENT_USER_H 
/******************************************************************************
  @file    qmi_client_user.h
  @brief   The QMI client Dam User space Header file.

  DESCRIPTION
  QMI Client Dam user space routines.  Clients needs to call below functions
  before entering and after exiting Dam user applications if the application
  needs to use QMI Client apis from DAM user space application.

  INITIALIZATION AND SEQUENCING REQUIREMENTS
  qmi_client_user_module_init - Should be called when an application is started
  qmi_client_user_module_deinit - Should be called when an application is exited

  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.  All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.
  ---------------------------------------------------------------------------
*******************************************************************************/
/*===========================================================================

                           EDIT HISTORY FOR FILE

$Header: //components/rel/core.tx/5.0/api/mproc/qmi_client_user.h#1 $

when       who     what, where, why
--------   ---     ----------------------------------------------------------
01/23/17   ab      Initial Version
===========================================================================*/


//Note: To be compiled with TX DAM user space code
#ifdef QAPI_TXM_MODULE


/*===========================================================================
  FUNCTION  qmi_client_user_module_init
===========================================================================*/
/*!
@brief
  Initializes required internal data structures for qmi client user application


@return
  QMI_NO_ERR if function is successful,
  error code otherwise.


@note
  - Dependencies
    - This function should be called once before using any other qmi client 
      functions from qmi_client.h

  - Side Effects
    - None
*/
/*=========================================================================*/
qmi_client_error_type qmi_client_user_module_init(void);


/*===========================================================================
  FUNCTION  qmi_client_user_module_deinit
===========================================================================*/
/*!
@brief
  De-initializes allocated internal data structures while init. 
  Should be called while exiting Dam user application.


@return
  QMI_NO_ERR if function is successful,
  error code otherwise.


@note
  - Dependencies
    - Should release all qmi handles opened in application
    - Then, this function should be called while exiting user application

  - Side Effects
    - Releases all qmi client handles if not released already
*/
/*=========================================================================*/
qmi_client_error_type qmi_client_user_module_deinit(void);

#endif /* QAPI_TXM_MODULE */

#endif /* QMI_CLIENT_USER_H */
