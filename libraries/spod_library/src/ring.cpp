#include <Arduino.h>
#include "typedef.h"
#include "ring.h"


void ring_reset(ring_t* ring)
{
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    ring->head = 0;
    ring->tail = 0;
    portEXIT_CRITICAL(&mux);
}


uint16_t ring_avail(ring_t* ring)
{
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    uint16_t avail = r_ring_avail(ring);
    portEXIT_CRITICAL(&mux);
    return (avail - 1U);
}


bool ring_put(ring_t* ring, uint8_t data)
{
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    bool result = r_ring_put(ring, data);
    portEXIT_CRITICAL(&mux);
    return result;
}


bool ring_get(ring_t* ring, uint8_t* data)
{
    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    bool result = r_ring_get(ring, data); 
    portEXIT_CRITICAL(&mux);
    return result;
}


bool ring_write(ring_t* ring, const uint8_t* data, uint16_t leng, bool* empty)
{
    uint16_t n, tail, avail;

    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    if (empty != NULL) {
        *empty = ring_empty(ring);
    }
    avail = r_ring_avail(ring);
    if (leng >= avail) { // the >= is correct here, refer to r_ring_avail()!
        portEXIT_CRITICAL(&mux);
        return FALSE;
    }

    tail = ring->tail;
    n = ((ring->head > tail) ? ring->head : ring->size) - tail;

    if (n > leng) {
        n = leng;
    }
    memcpy(&ring->data[tail], data, n);
    tail = RING_ADD(tail, n, ring->size);
    uint16_t l = leng - n;
    if (l != 0U) {
        memcpy(&ring->data[tail], &data[n], l);
    }
    ring->tail = RING_ADD(tail, l, ring->size);
    portEXIT_CRITICAL(&mux);
    return TRUE;
}


bool ring_read(ring_t* ring, uint8_t* data, uint16_t leng, bool consume)
{
    uint16_t n, head;

    if (leng == 0) return TRUE;

    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);
    if (leng > ring_valid(ring)) {
        portEXIT_CRITICAL(&mux);
        return FALSE;
    }

    head = ring->head;
    n = ((head > ring->tail) ? ring->size : ring->tail) - head;
    portEXIT_CRITICAL(&mux);

    if (n > leng) {
        n = leng;
    }
    memcpy(data, &ring->data[head], n);
    head = RING_ADD(head, n, ring->size);
    uint16_t l = leng - n;
    if (l != 0U) {
        memcpy(&data[n], &ring->data[head], l);
    }

    if (consume) {
        ring->head = RING_ADD(head, l, ring->size);
    }

    return TRUE;
}
