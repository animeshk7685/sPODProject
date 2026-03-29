/*
 * image_data.h
 *
 *  Created on: Sep 27, 2014
 *      Author: Jason
 */

#ifndef image_data_H_
#define image_data_H_

#include "common.h"

#define SPOD_RAWDATA_LENGTH 5009
#define SPOD_ADDRS_1_LENGTH 7236
#define SPOD_ADDRS_2_LENGTH 8252
#define SPOD_ADDRS_3_LENGTH 8191
#define SPOD_ADDRS_4_LENGTH 8269

#define SPOD_PRO_RAWDATA_LENGTH 1806
    
#define BUTTON_RAWDATA_LENGTH (512)   
    
#define BUTTON_FLASH_RAWDATA_LENGTH (512)    

extern const tBitmap_header sPod_Bitmap_Header[];

extern const uint8_t sPod_Bitmap_RawData[SPOD_RAWDATA_LENGTH];

extern const tBitmap_header sPod_Pro_Bitmap_Header[];

extern const uint8_t sPod_Pro_Bitmap_RawData[SPOD_PRO_RAWDATA_LENGTH];

extern tBitmap_header button_Bitmap_Header[];

extern uint8_t button_Bitmap_RawData[BUTTON_FLASH_RAWDATA_LENGTH * 64];

#endif /* image_data_H_ */