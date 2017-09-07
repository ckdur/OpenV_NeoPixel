
#include <inttypes.h>
void show(void);
#define OPENV_MAX_PINS 8

// **************** PARAMETERS ******************

#define pin 5
#define NUM_OF_LEDS 16

uint32_t pixels[NUM_OF_LEDS] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF};

void main(void)
{
  for(;;) show();
}

// **************** END_PARAMETERS **************

#ifndef F_CPU
#define F_CPU 25000000
#endif

unsigned long micros() {
	unsigned long m;
	
	unsigned int num_cycles, num_cyclesh;
	__asm__("rdcycle %0; rdcycleh %1;" : "=r"(num_cycles), "=r"(num_cyclesh));
	
	uint64_t cycles = ((uint64_t)num_cyclesh << 32) | (uint64_t)num_cycles;
	m = cycles * 1000000 ; 
	m /= F_CPU ; 
	return m;
}

uint32_t endTime;       // Latch timing reference
inline int
    canShow(void) { return (micros() - endTime) >= 300L; }

void show(void) {

  //if(!pixels) return; LOL always exists

  // Data latch = 300+ microsecond pause in the output stream.  Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed.  This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
  while(!canShow());
  // endTime is a private member (rather than global var) so that mutliple
  // instances on different pins can be quickly issued in succession (each
  // instance doesn't delay the next).

  // In order to make this code runtime-configurable to work with any pin,
  // SBI/CBI instructions are eschewed in favor of full PORT writes via the
  // OUT or ST instructions.  It relies on two facts: that peripheral
  // functions (such as PWM) take precedence on output pins, so our PORT-
  // wide writes won't interfere, and that interrupts are globally disabled
  // while data is being issued to the LEDs, so no other code will be
  // accessing the PORT.  The code takes an initial 'snapshot' of the PORT
  // state, computes 'pin high' and 'pin low' values, and writes these back
  // to the PORT register as needed.

#if defined(__RISCV_OPENV__)

// RISC-V Open-V  ---------------------------------------------------------

// NOTE: Copied from the SAMD21XXX
#if F_CPU == 25000000
  // Tried this with a timer/counter, couldn't quite get adequate
  // resolution.  So yay, you get a load of goofball NOPs...

  uint32_t  *ptr, *end, p, bitMask;
  uint32_t  pinMask;
  
  #define ADDR_GPIO ((uint32_t*)0x1040)
  
  if(pin >= OPENV_MAX_PINS) return;
  
  ptr     =  pixels;
  end     =  ptr + NUM_OF_LEDS;
  p       = *ptr++;
  bitMask =  0x800000;

  volatile uint32_t *port = ADDR_GPIO+pin;

#ifdef NEO_KHZ400 // 800 KHz check needed only if 400 KHz support enabled
  if(is800KHz) {
#endif
    for(;;) {
      *port = 0x3;
      asm("nop; nop; nop; nop; nop; nop; nop; nop;");
      if(p & bitMask) {
        asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop;");
        *port = 0x2;
      } else {
        *port = 0x2;
        asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop;");
      }
      if(bitMask >>= 1) {
        asm("nop; nop; nop; nop; nop; nop; nop; nop; nop;");
      } else {
        if(ptr >= end) break;
        p       = *ptr++;
        bitMask = 0x80;
      }
    }
#ifdef NEO_KHZ400
  } else { // 400 KHz bitstream
    for(;;) {
      *port = 0x3;
      asm("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
      if(p & bitMask) {
        asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop;");
        *port = 0x2;
      } else {
        *port = 0x2;
        asm("nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop; nop; nop; nop; nop; nop;"
            "nop; nop; nop;");
      }
      asm("nop; nop; nop; nop; nop; nop; nop; nop;"
          "nop; nop; nop; nop; nop; nop; nop; nop;"
          "nop; nop; nop; nop; nop; nop; nop; nop;"
          "nop; nop; nop; nop; nop; nop; nop; nop;");
      if(bitMask >>= 1) {
        asm("nop; nop; nop; nop; nop; nop; nop;");
      } else {
        if(ptr >= end) break;
        p       = *ptr++;
        bitMask = 0x80;
      }
    }
  }
#endif

#else
#error "Sorry, only 25 MHz is supported, please set Tools > CPU Speed to 48 MHz"
#endif // F_CPU == 25000000

// NO ARCHITECTURE  -------------------------------------------------------

#else 
#error Architecture not supported
#endif


// END ARCHITECTURE SELECT ------------------------------------------------

  endTime = micros(); // Save EOD time for latch on next call
}
