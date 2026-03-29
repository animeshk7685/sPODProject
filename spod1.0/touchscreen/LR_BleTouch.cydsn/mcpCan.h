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
    
#include <project.h>
#include "mcpCanDefines.h"
#include <stdbool.h>
    
#define MAX_CHAR_IN_MESSAGE 8

struct MCP_CAN
{
    uint8   m_nExtFlg;                                                  /* identifier xxxID             */
                                                                        /* either extended (the 29 LSB) */
                                                                        /* or standard (the 11 LSB)     */
    uint32  m_nID;                                                      /* can id                       */
    uint8   m_nDlc;                                                     /* data length:                 */
    uint8   m_nDta[MAX_CHAR_IN_MESSAGE];                            	/* data                         */
    uint8   m_nRtr;                                                     /* rtr                          */
    uint8   m_nfilhit;
    uint8   SPICS;
};

typedef struct MCP_CAN mcpCan;

    void configureCan(void);
    uint8 mcpCanInit(uint8 speed);
    uint8 mcpCanInitMask(uint8 num, uint8 ext, uint32 ulData);           /* init Masks                   */
    uint8 mcpCanInitFilt(uint8 num, uint8 ext, uint32 ulData);           /* init filters                 */
    uint8 mcpCanSendMsgBuf(mcpCan *can, uint32 id, uint8 ext, uint8 len, uint8 *buf);  /* send buf                     */
    uint8 mcpCanReadMsgBuf(mcpCan *can, uint8 *len, uint8 *buf);                       /* read buf                     */
    uint8 mcpCanCheckReceive();                                       /* if something received        */
    uint8 mcpCanCheckError();                                         /* if something error           */
    uint32 mcpCanGetCanId(mcpCan *can);                                          /* get can id when receive      */
    void mcpCanClearError();
    
    uint8 mcpCanReadRegister(uint8 address);
    void mcpCanSleep(bool sleep);
    uint8 mcpCanSetCanCtrlMode(const uint8 newmode);
    void mcpCanReset(void);

#endif



/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
