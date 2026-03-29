#ifndef __XI_H__
#define __XI_H__


#define PCM_INT1        0   // NIPUT  -- upstream PCM connected (if true, not PCM1)
#define PCM_INT2        1   // OUTPUT -- set to 1 by PCM to notify downstream PCM
#define BT_PAIR         2   // INPUT  -- active low, looking for falling edge -- when detected PCM goes into Bluetooth pairing mode
#define BYPASS_CTRL     3   // OUTPUT -- disables turning off when low voltage is detected
#define INPUT5_CTRL     4   // OUTPUT -- set to 1 if active low, 0 if active high, if the INPUTx is active low, look for a rising edge as the trigger,
                            //             could be just a pulse, next pulse would then be a toggle to turn the output off
#define INPUT4_CTRL     5   // OUTPUT -- 
#define IGN_SENSE       6   // INPUT  -- 
#define KEY_ON          7   // INPUT  -- this is INPUT6
#define INPUT1          8   // INPUT  -- this could be a pulse and is either active high or active low as set by INPUTx_CTRL
#define INPUT2          9   // INPUT  --
#define INPUT3          10  // INPUT  --
#define INPUT4          11  // INPUT  --
#define INPUT5          12  // INPUT  --
#define INPUT6          KEY_ON
#define INPUT3_CTRL     13  // OUTPUT --
#define INPUT2_CTRL     14  // OUTPUT --
#define INPUT1_CTRL     15  // OUTPUT --


extern void xi_init();
extern void xi_write(uint8_t port, bool on);
extern bool xi_read(uint8_t port);

extern bool pcm_int1();
extern void pcm_int2(bool on);
extern bool key_on();
extern bool bt_pair();
extern bool ign_sense();
extern void bypass_ctrl(bool on);
extern uint16_t get_inputs();
extern bool get_input(uint8_t which);
extern void set_input_ctrl(uint8_t which, bool on);

#endif
