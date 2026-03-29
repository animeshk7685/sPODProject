#ifndef _DAB_H_
#define _DAB_H_

#include <sys/types.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern int dab_acknak_cb(uint8_t result, uint8_t seqno);
extern int dab_output(uint8_t ptype, uint8_t* data, uint8_t nbytes,
               int (*acknak_cb)(uint8_t result, uint8_t seqno));
extern int  dab_firmware_download(char* filename, 
                                  int major_version, 
                                  int minor_version, 
                                  int build_number, 
                                  bool is64k,
                                  board_type_t board_type);
extern bool dab_queued(void);
extern bool dab_txq_full();
extern void dab_process(void);
extern void dab_show_status(void);
extern void dab_register_input(int (*cb)(uint8_t seqno, uint8_t type, uint8_t* data, uint8_t nbytes));
extern void dab_unregister_input(void);
extern void dab_cb_init(uint8_t expected_input_type, uint8_t* data, uint8_t leng);
extern void dab_cb_wait(uint32_t timeout, bool* ack, bool* input);
extern void dab_setmac(uint8_t mac[3]);
extern void dab_init(int dab_fd, uint8_t mac[3], bool include_mac);
extern bool dab_get_faults(board_type_t btype, uint16_t* faults);

#ifdef __cplusplus
}
#endif

#endif
