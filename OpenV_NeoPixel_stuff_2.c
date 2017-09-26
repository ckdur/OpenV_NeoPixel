
#include <inttypes.h>
void show(void);
uint64_t cycles();
uint64_t endTime;       // Latch timing reference
#define OPENV_MAX_PINS 8
#define ADDR_GPIO ((uint32_t*)0x1040)

// **************** PARAMETERS ******************

#define pin 6
#define NUM_OF_LEDS 16

#ifdef NEO_KHZ400
int32_t is800KHz = 0;
#endif
uint32_t pixels[NUM_OF_LEDS] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF};

void setup(void)
{
  endTime = cycles();
}

void loop(void)
{
  show();
}

// **************** END_PARAMETERS **************

#ifndef F_CPU
#define F_CPU 50000000
#endif

uint64_t cycles() {
	
	uint32_t num_cycles, num_cyclesh;
	__asm__("rdcycle %0; rdcycleh %1;" : "=r"(num_cycles), "=r"(num_cyclesh));
	
	uint64_t cycles = ((uint64_t)num_cyclesh << 32) | (uint64_t)num_cycles;
	return cycles;
}

#define canShow()  ((cycles() - endTime) >= 1500L) 

void show(void) {

//#if defined(__RISCV_OPENV__)

  volatile uint32_t *port = ADDR_GPIO+pin;
  //*port = 0x7;

  //if(!pixels) return; LOL always exists

  // Data latch = 300+ microsecond pause in the output stream.  Rather than
  // put a delay at the end of the function, the ending time is noted and
  // the function will simply hold off (if needed) on issuing the
  // subsequent round of data until the latch time has elapsed.  This
  // allows the mainline code to start generating the next frame of data
  // rather than stalling for the latch.
  while(!canShow());

  //*port = 0x0;
  //endTime = cycles(); // Save EOD time for latch on next call
  //while(!canShow());
  
  //endTime = cycles(); // Save EOD time for latch on next call
  //return;
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

// RISC-V Open-V  ---------------------------------------------------------

// NOTE: Copied from the SAMD21XXX
#if F_CPU == 50000000
  // Tried this with a timer/counter, couldn't quite get adequate
  // resolution.  So yay, you get a load of goofball NOPs...

  uint32_t  *ptr, *end, p, bitMask;
  
  if(pin >= OPENV_MAX_PINS) return;
  
  ptr     =  pixels;
  end     =  ptr + NUM_OF_LEDS;
  p       = *ptr++;
  bitMask =  0x800000;

#ifdef NO_NEOPIXEL_ASSEMBLY  
  for(;;) {
    *port = 0x7;
    __asm__("nop; nop; nop; nop; nop; nop; nop;");
    if(p & bitMask) {
      __asm__("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
    *port = 0x0;
    } else {
    *port = 0x0;
      __asm__("nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;");
    }
    __asm__("nop; nop; nop; nop; nop; nop; nop;");
    if(bitMask >>= 1) {
    __asm__("nop; nop; nop; nop; nop; nop; nop;");
    } else {
      if(ptr >= end) break;
      p       = *ptr++;
      bitMask = 0x800000;
    }
  }
#else  
	__asm__ __volatile__(
	"addi t0, zero, 0x7;"
	"for_inf:"
    "sw t0, 0(%1);"
    "nop; nop; nop; nop; nop; nop; nop; nop;"
    "nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop;"
    "sw zero, 0(%1);"
    "nop; nop; nop; nop; nop; nop; nop; nop;"
	  "j for_inf;"
	"for_inf_end:"
	//    %0          %1       %2         %3           %4
	: "=r"(ptr) : "r"(port), "r"(p), "r"(bitMask), "r"(end) : "t0", "t1", "t2", "t3" );
#endif // NEOPIXEL_ASSEMBLY

#else
#error "Sorry, only 5 MHz is supported, please set Tools > CPU Speed to 5 MHz"
#endif // F_CPU == 50000000

// NO ARCHITECTURE  -------------------------------------------------------

//#else 
//#error Architecture not supported
//#endif


// END ARCHITECTURE SELECT ------------------------------------------------

  endTime = cycles(); // Save EOD time for latch on next call
}
