//-----------------------------------------------------------------------------
//  button.h
//
//-----------------------------------------------------------------------------

#ifndef __BUTTON_FSM_H__
#define __BUTTON_FSM_H__

typedef struct {
    uint32_t press_hold;
    uint32_t press_time;
    uint32_t release_time;
    uint32_t color;
    uint8_t prev_index;
    uint8_t this_index;
    uint8_t activation_method;
} button_state_t;

extern button_state_t button[SWITCHES];

extern void button_fsm_init();
extern bool button_fsm_tick(uint8_t switch_index, switch_config_t* s, uint32_t* color);
extern bool button_release(uint8_t switch_index, switch_config_t* s, uint32_t* color);
extern bool button_press(uint8_t switch_index, switch_config_t* s, uint32_t* color);
extern uint8_t button_get_activation_method(uint8_t switch_index);
extern uint32_t button_get_color(uint8_t switch_index);
extern void button_set_color(uint8_t index, uint32_t color);
extern void button_fsm_clear_holds();
extern void button_fsm_clear(uint8_t bi, uint32_t color);
extern uint32_t button_get_press_hold(uint8_t index);

#endif
