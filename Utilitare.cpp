#include "Utilitare.hpp"
//#define HEATSHRINK_DEBUG

static void decompress(uint8_t *input,
                       uint32_t input_size,
                       uint8_t *output,
                       uint32_t *output_size
                      ) {
  heatshrink_decoder_reset(&hsd);
#ifdef HEATSHRINK_DEBUG
  Serial.print(F("\n^^ DECOMPRESSING\n"));
  dump_buf("input", input, input_size);
#endif
  size_t   count  = 0;
  uint32_t sunk   = 0;
  uint32_t polled = 0;
  while (sunk < input_size) {
    //ASSERT(heatshrink_decoder_sink(&hsd, &comp[sunk], input_size - sunk, &count) >= 0);
    heatshrink_decoder_sink(&hsd, &input[sunk], input_size - sunk, &count);
    sunk += count;
#ifdef HEATSHRINK_DEBUG
    Serial.print(F("^^ sunk "));
    Serial.print(count);
    Serial.print(F("\n"));
#endif
    if (sunk == input_size) {
      //ASSERT_EQ(HSDR_FINISH_MORE, heatshrink_decoder_finish(&hsd));
      heatshrink_decoder_finish(&hsd);
    }

    HSD_poll_res pres;
    do {
      pres = heatshrink_decoder_poll(&hsd, &output[polled],
                                     *output_size - polled, &count);
      //ASSERT(pres >= 0);
      polled += count;
#ifdef HEATSHRINK_DEBUG
      Serial.print(F("^^ polled "));
      Serial.print(polled);
      Serial.print(F("\n"));
#endif
    } while (pres == HSDR_POLL_MORE);
    //ASSERT_EQ(HSDR_POLL_EMPTY, pres);
    if (sunk == input_size) {
      HSD_finish_res fres = heatshrink_decoder_finish(&hsd);
      //ASSERT_EQ(HSDR_FINISH_DONE, fres);
    }
    if (polled > *output_size) {
#ifdef HEATSHRINK_DEBUG
      Serial.print(F("FAIL: Exceeded the size of the output buffer!"));
#endif
    }
  }
#ifdef HEATSHRINK_DEBUG
  Serial.print(F("in: "));
  Serial.print(input_size);
  Serial.print(F(" decompressed: "));
  Serial.print(polled);
  Serial.print(F(" \n"));
#endif
  //update the output size
  *output_size = polled;

#ifdef HEATSHRINK_DEBUG
  dump_buf("output", output, *output_size);
#endif
}

#ifdef HEATSHRINK_DEBUG
static void dump_buf(const char *name, uint8_t *buf, uint16_t count) 
{
    for (int i=0; i<count; i++) {
        uint8_t c = (uint8_t)buf[i];
        //printf("%s %d: 0x%02x ('%c')\n", name, i, c, isprint(c) ? c : '.');
        Serial.print(name);
        Serial.print(F(" "));
        Serial.print(i);
        Serial.print(F(": 0x"));
        Serial.print(c, HEX);
        Serial.print(F(" ('"));
        Serial.print(isprint(c) ? c : '.');
        Serial.print(F("')\n"));
    }
}
#endif
