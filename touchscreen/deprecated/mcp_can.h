/*
  mcp_can.h
  2012 Copyright (c) Seeed Technology Inc.  All right reserved.

  Author:Loovee
  Contributor: Cory J. Fowler
  2014-1-16
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-
  1301  USA
*/
#ifndef _MCPCAN_H_
#define _MCPCAN_H_
    
#include "mcp_can_defines.h"
#include <stdbool.h>
    
#define MAX_CHAR_IN_MESSAGE 8

struct MCP_CAN
{
    uint8_t   m_nExtFlg;                                                  /* identifier xxxID             */
                                                                        /* either extended (the 29 LSB) */
                                                                        /* or standard (the 11 LSB)     */
    uint32_t  m_nID;                                                      /* can id                       */
    uint8_t   m_nDlc;                                                     /* data length:                 */
    uint8_t   m_nDta[MAX_CHAR_IN_MESSAGE];                            	/* data                         */
    uint8_t   m_nRtr;                                                     /* rtr                          */
    uint8_t   m_nfilhit;
    uint8_t   SPICS;
};

typedef struct MCP_CAN mcpCan;

    uint8_t mcpCanInit(uint8_t speed);
    uint8_t mcpCanInitMask(uint8_t num, uint8_t ext, uint32_t ulData);           /* init Masks                   */
    uint8_t mcpCanInitFilt(uint8_t num, uint8_t ext, uint32_t ulData);           /* init filters                 */
    uint8_t mcpCanSendMsgBuf(mcpCan *can, uint32_t id, uint8_t ext, uint8_t len, uint8_t *buf);  /* send buf                     */
    uint8_t mcpCanReadMsgBuf(mcpCan *can, uint8_t *len, uint8_t *buf);                       /* read buf                     */
    uint8_t mcpCanCheckReceive();                                       /* if something received        */
    uint8_t mcpCanCheckError();                                         /* if something error           */
    uint32_t mcpCanGetCanId(mcpCan *can);                                          /* get can id when receive      */
    void mcpCanClearError();
    
    uint8_t mcpCanReadRegister(uint8_t address);
    void mcpCanSleep(bool sleep);
    uint8_t mcpCanSetCanCtrlMode(const uint8_t newmode);
    void mcpCanReset(void);

#endif



/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
