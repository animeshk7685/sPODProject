#include "spod_library.h"
#include "crc16.h"
#include "config.h"
#include "button_fsm.h"

static const char* TAG = "BTN";

#define OFF   0x000000
#define RED   0xFF0000
#define GREEN 0x00FF00
#define BLUE  0x0000FF

#define DOUBLE_TIME     750   // double time in milliseconds (have to press button second time before this)
#define LONG_TIME       2250  // long press in milliseconeds (have to hold key down for this amount of time)
#define TOGGLE_TIME     600   // ~48ms steps = 576ms


button_state_t button[SWITCHES];


void button_fsm_init()
{
    memset((uint8_t*)&button, 0, sizeof(button));
}


void button_fsm_clear_holds()
{
    for (uint8_t bi = 0; bi < SWITCHES; ++bi) {
        button[bi].press_hold = 0;
    }
}


void button_fsm_clear(uint8_t bi, uint32_t color)
{
    memset(button+bi, 0, sizeof(button[0]));
    button[bi].color = color;
}


bool button_fsm_tick(uint8_t index, switch_config_t* s, uint32_t* color)
{
    uint32_t now = millis();
    button_state_t* b = &button[index];

    if (b->color == RED && b->press_hold == 0) {
        if (s->inputs & TOGGLE_INPUT) {
            if (now - b->press_time > DOUBLE_TIME) {
                *color = b->color = GREEN;
                b->activation_method = TOGGLE_INPUT;
                LOGD(TAG, "%d -- %s(index=%d, color=%x).%d TOGGLE_INPUT\r\n", now, __FUNCTION__, index, *color, __LINE__);
                return true;
            }
        } else 
        if (s->inputs & DOUBLE_INPUT) {
            if (now - b->press_time > DOUBLE_TIME) {
                *color = b->color = OFF;
                b->press_time = 0;	
                b->activation_method = 0;
                return true;			
            }
        } else
        if (s->inputs & LONG_INPUT) {
            *color = b->color = OFF;
            b->press_time = 0;
            b->activation_method = 0;
            return true;
        }
    }

    return false;
}


bool button_release(uint8_t index, switch_config_t* s, uint32_t* color)
{
    button_state_t* b = &button[index];

    if (b->press_hold != 0) {
        if (index == b->this_index) {
            LOGD(TAG, "%d -- %s(si=%d, color=%x): inputs=%x, press_hold=%d, b->color=%x, prev_index=%d, press_time=%d, release_time=%d, delta=%d\r\n", 
                millis(), __FUNCTION__, index, *color, s->inputs, b->press_hold, b->color, b->prev_index, b->press_time, b->release_time, b->press_time - b->release_time);
            b->this_index = 0xFF;
            b->press_hold = 0;
            if (s->inputs & MOMENTARY_INPUT) {
                *color = b->color = OFF;
                b->activation_method = 0;
                return true;
            }
            if (b->color == RED) {
                bool double_enabled = s->inputs & DOUBLE_INPUT;
                if (double_enabled) {
                    if (b->prev_index == index && b->press_time - b->release_time < 750) {
                        *color = b->color = BLUE;
                        b->activation_method = DOUBLE_INPUT;
                        return true;
                    }
                }
                if (s->inputs & TOGGLE_INPUT) {
                    if (!double_enabled || b->press_hold > 1) {
                        *color = b->color = GREEN;
                        b->activation_method = TOGGLE_INPUT;
                        LOGD(TAG, "%d -- %s(index=%d, color=%x).%d TOGGLE_INPUT\r\n", millis(), __FUNCTION__, index, *color, __LINE__);
                        return true;                
                    }
                }
                b->prev_index = index;
                b->release_time = millis();
                *color = b->color = OFF;
                b->activation_method = 0;
                return true;
            }
        }
        b->press_hold = 0;
    }

    return false;
}


// this routine is designed to be called repeatedly when a button is
// held down (either touchscreen or switch) -- the touchscreen will
// call this routine approximately every 48ms, we try to mimick that
// behaviour with the switch -- note the FSM triggers off of press_hold
// which is incremented every time this routine is called and only set
// back to zero once the button is released

bool button_press(uint8_t index, switch_config_t* s, uint32_t* color)
{
    button_state_t* b = &button[index];
    uint32_t now = millis();

    LOGD(TAG, "%d -- %s(si=%d, color=%x): press_hold=%d\r\n", millis(), __FUNCTION__, index, *color, b->press_hold);
    b->press_hold += 1;
    if (b->press_hold == 1) {
        if (b->color == GREEN || b->color == BLUE) {
            *color = b->color = OFF;
            b->press_time = 0;
            b->prev_index = 0;
            b->activation_method = 0;
            return true;
        }

        b->press_time = now;
        b->this_index = index;

        if (s->inputs & MOMENTARY_INPUT) {
            *color = b->color = GREEN;
            b->activation_method = MOMENTARY_INPUT;
            return true;
        }

        *color = b->color = RED;
        LOGD(TAG, "%d -- %s(index=%d, color=%x)\r\n", now, __FUNCTION__, index, *color);
    } else {
        if (b->color == RED) {
            if (s->inputs & LONG_INPUT) {
                if (now - b->press_time > LONG_TIME) {
                    LOGD(TAG, "%d -- %s(index=%d, color=%x).%d LONG_INPUT -- press_time=%d, delta=%d\r\n", now, __FUNCTION__, index, *color, __LINE__, b->press_time, now - b->press_time);
                    *color = b->color = BLUE;
                    b->activation_method = LONG_INPUT;
                    return true;
                }
            } else
            if (s->inputs & TOGGLE_INPUT) {
                if (now - b->press_time > TOGGLE_TIME) {
                    *color = b->color = GREEN;
                    b->activation_method = TOGGLE_INPUT;
                    LOGD(TAG, "%d -- %s(index=%d, color=%x).%d TOGGLE_INPUT\r\n", now, __FUNCTION__, index, *color, __LINE__);
                    return true;
                }
            }
        }
    }

    return false;
}


uint8_t button_get_activation_method(uint8_t index)
{
    return button[index].activation_method;
}


uint32_t button_get_color(uint8_t index)
{
    return button[index].color;
}


uint32_t button_get_press_hold(uint8_t index)
{
    return button[index].press_hold;
}


void button_set_color(uint8_t index, uint32_t color)
{
    button[index].color = color;
}