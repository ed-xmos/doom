#ifndef XDAC_H_
#define XDAC_H_
#include <stdio.h>
#include <xccompat.h>
#include <xcore/channel.h>

class Dac final {
  private:
    static chanend c_midi;
    static int32_t left;
    static int32_t right;

  public:
    static void setup(chanend c_midi_pcm) {
      c_midi = c_midi_pcm;
    }

    static void set0to7(int16_t out) {
      left = out << 16;
    }

    static void set8toF(int16_t out) {
      right = out << 16;
      int32_t mix = (left >> 1) + (right >> 1);
      chan_out_word(c_midi, mix + (left >> 1));
      chan_out_word(c_midi, mix + (right >> 1));
    }
};


#endif