#ifndef _RING_H_
#define _RING_H_

#include "typedef.h"

#if defined(__cplusplus)
extern "C" {
#endif

/// RING_ADD() performs fast modulo function relying on size being a power of two
#define RING_ADD(index, amount, size)   (((index) + (amount)) & ((size)-1U))
#define RING_INC(index, size)           RING_ADD(index, 1U, size)


typedef struct {
    uint16_t size;   ///< Size of buffer. NOTE: must be a power of two (refer to RING_ADD() above!)
    volatile uint16_t head;   ///< Head of ring (offset)
    volatile uint16_t tail;   ///< Tail of ring (offset)
    uint8_t* data;   ///< pointer to data buffer
} ring_t;

extern void ring_reset(ring_t* ring);
extern uint16_t ring_avail(ring_t* ring);
extern bool ring_put(ring_t* ring, uint8_t data);
extern bool ring_get(ring_t* ring, uint8_t* data);


static inline bool ring_empty(ring_t* ring)
{
    return ring->head == ring->tail;
}


static inline bool ring_full(ring_t* ring)
{
    return RING_INC(ring->tail, ring->size) == ring->head;
}


static inline uint16_t ring_valid_contig(ring_t* ring)
{
    return ((ring->head <= ring->tail) ? 
        (ring->tail - ring->head) : (ring->size - ring->head));
}


static inline uint16_t ring_valid(ring_t* ring)
{
    return ((ring->head <= ring->tail) ? 
        (ring->tail - ring->head) : ((ring->size - ring->head) + ring->tail));
}


static inline void ring_consume(ring_t* ring, uint16_t leng)
{
    ring->head = RING_ADD(ring->head, leng, ring->size);
}


static inline uint16_t r_ring_avail(ring_t* ring)
{
    return ring->size - ring_valid(ring);
}


static inline bool r_ring_put(ring_t* ring, uint8_t data)
{
    uint16_t next = RING_INC(ring->tail, ring->size);
    if (next == ring->head)
        return FALSE;
    ring->data[ring->tail] = data;
    ring->tail = next;
    return TRUE;
}


static inline bool r_ring_get(ring_t* ring, uint8_t* data)
{
    uint16_t next = ring->head;
    if (next == ring->tail) {
        return FALSE;
    }
    *data = ring->data[next];
    ring->head = RING_INC(next, ring->size);
    return TRUE;
}


extern bool ring_write(ring_t* ring, const uint8_t* data, uint16_t leng, bool* empty);
extern bool ring_read(ring_t* ring, uint8_t* data, uint16_t leng, bool consume);

#if defined(__cplusplus)
}
#endif

#endif
