//#include "heatshrink_encoder.h"
#include "heatshrink_decoder.h"
#include <HardwareSerial.h>
#define HEATSHRINK_DEBUG

static heatshrink_decoder hsd;

static void decompress(uint8_t *input,
                       uint32_t input_size,
                       uint8_t *output,
                       uint32_t *output_size
                      );
#ifdef HEATSHRINK_DEBUG
static void dump_buf(const char *name, uint8_t *buf, uint16_t count);
#endif
