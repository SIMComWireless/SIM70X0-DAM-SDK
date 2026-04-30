/**
 * @file gps_demo.h
 * @brief Public APIs for GPS demo helpers.
 *
 * Declares the exported functions for GNSS tracking, batching and
 * geofence management implemented in gps_demo.c.
 *
 * @author SIMCom OpenSDK Team
 * @copyright Copyright (c) 2022 SIMCom Wireless. All rights reserved.
 */
#ifndef __GPS_DEMO_H__
#define __GPS_DEMO_H__

/*-------------------------------------------------------------------------
 * Include Files
 *-----------------------------------------------------------------------*/
#include "../application/ThreadX.h"

/*-------------------------------------------------------------------------
 * Function Declarations and Documentation
 *-----------------------------------------------------------------------*/
/**
 * @brief Initialize GPS demo client and allocate resources.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_init(VOID);

/**
 * @brief Perform a single-shot tracking request.
 * @param[in] minInterval Minimum interval between fixes.
 * @param[in] minDistance Minimum distance between fixes.
 * @return DAM_Status_t DAM_STATUS_SUCCESS if a fix was obtained; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t tracking_test_singleshot(uint32_t minInterval,uint32_t minDistance);

/**
 * @brief Start continuous tracking with provided options.
 * @param[in] minInterval Minimum interval between fixes.
 * @param[in] minDistance Minimum distance between fixes.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_start_tracking(uint32_t minInterval,uint32_t minDistance);

/**
 * @brief Stop active tracking session.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_stop_tracking(VOID);

/**
 * @brief Update options for an active tracking session.
 * @param[in] minInterval New minimum interval between fixes.
 * @param[in] minDistance New minimum distance between fixes.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_update_tracking_option(uint32_t minInterval,uint32_t minDistance);

/**
 * @brief Start GNSS batching.
 * @param[in] minInterval Minimum interval for batched fixes.
 * @param[in] minDistance Minimum distance for batched fixes.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_start_batching(uint32_t minInterval,uint32_t minDistance);

/**
 * @brief Stop GNSS batching.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_stop_batching(VOID);

/**
 * @brief Update batching options.
 * @param[in] minInterval New minimum interval for batched fixes.
 * @param[in] minDistance New minimum distance for batched fixes.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_update_batching_option(uint32_t minInterval,uint32_t minDistance);

/**
 * @brief Retrieve batched location count.
 * @param[in] count Number of batched locations to fetch.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_get_batching_loc(size_t count);

/**
 * @brief Add a geofence with specified center and radius.
 * @param[in] latitude Center latitude.
 * @param[in] longitude Center longitude.
 * @param[in] radius Radius in meters.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_Add_Geofences(double latitude, double longitude,double radius);

/**
 * @brief Remove previously added geofences (demo helper).
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_remove_Geofences(VOID);

/**
 * @brief Pause all configured geofences.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_pause_Geofences(VOID);

/**
 * @brief Resume paused geofences.
 * @return DAM_Status_t DAM_STATUS_SUCCESS on success; DAM_STATUS_ERROR otherwise.
 */
DAM_Status_t gps_demo_resume_Geofences(VOID);

#endif

