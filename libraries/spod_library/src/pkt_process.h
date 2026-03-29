//-----------------------------------------------------------------------------
//  pkt_process.h
//
//-----------------------------------------------------------------------------

#ifndef __PKT_PROCESS_H__
#define __PKT_PROCESS_H__

#if defined(__cplusplus)
extern "C" {
#endif

extern void process_ota_data_packet(uint8_t* data, uint8_t leng);
extern void process_ota_ctrl_packet(uint8_t mac[3], uint8_t* data, uint8_t pleng, void (*send_ack_packet)(uint8_t mac[3], uint8_t* data, uint8_t leng));
extern void pkt_process(uint8_t owner);
extern void pkt_register_callback(void (*packet_callback)(uint8_t ptype, uint8_t* data, uint8_t leng));
extern void pkt_process_init(const char version[32], 
    void (*send_ota_ctrl_packet)(uint8_t mac[3], uint8_t* data, uint8_t leng), 
    void (*packet_callback)(uint8_t ptype, uint8_t* data, uint8_t leng));
extern void send_heartbeat_pkt(uint8_t pcm_addr);
extern void send_arbiter_rebooted_pkt();

#if defined(__cplusplus)
}
#endif

#endif
