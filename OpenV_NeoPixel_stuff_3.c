
#include <inttypes.h>
void show(void);
uint64_t cycles();
uint64_t endTime;       // Latch timing reference
#define OPENV_MAX_PINS 8
#define ADDR_GPIO ((uint32_t*)0x1040)

// **************** PARAMETERS ******************

#define pin 6
#define pin2 5
#define NUM_OF_LEDS 15
#define WS2812

uint32_t pixels[NUM_OF_LEDS] = {0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 0xFFFFFF, 
                                0xFFFFFF, 0xFFFFFF, 0xFFFFFF};

void setup(void)
{
  endTime = cycles();
}

uint32_t ind = 0;
uint32_t on = 0xFFFFFF;
void loop(void)
{
  uint32_t i;
  for(i = 0; i < 50000; i++);
  pixels[ind++] = on;
  if(ind >= NUM_OF_LEDS) {ind = 0; on ^= 0xFFFFFF;}
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

#define canShow()  ((cycles() - endTime) >= 6000L) 

void show(void) {

//#if defined(__RISCV_OPENV__)

  volatile uint32_t *port = ADDR_GPIO+pin;
  volatile uint32_t *port2 = ADDR_GPIO+pin2;
  //*port = 0x7;

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

#ifdef WS2812B 
	__asm__ __volatile__(
	"addi t0, zero, 0x7;"
	"for_inf_WS2812B:"
	"sw t0, 0(%1);"
	"sw t0, 0(%5);"
	"nop;"
	"and t1, %2, %3;"
	"beq t1, zero, if_zero;"
	  "srli %3, %3, 1;"
	  "bne %3, zero, if_zero_2_end;"
	    "bgeu %0, %4, for_inf_end_WS2812B;"
	    "lw %2, 0(%0);"
	    "addi %0, %0, 4;"
	    "li %3, 0x800000;"
	  "if_zero_2_end:"
	  // ----- Dummy here
	  "nop; nop; nop;nop; nop; nop;nop;"
	  // ----- Dummy here
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "j if_zero_end;"
	"if_zero:"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  // ----- Dummy here
	  "nop; nop; nop;nop; nop; nop;nop; nop; nop;"
	  // ----- Dummy here
	  "srli %3, %3, 1;"
	  "bne %3, zero, if_zero_3_end;"
	    "bgeu %0, %4, for_inf_end_WS2812B;"
	    "lw %2, 0(%0);"
	    "addi %0, %0, 4;"
	    "li %3, 0x800000;"
	  "if_zero_3_end:"
	"if_zero_end:"
	"nop; nop;"
	"j for_inf_WS2812B;"
	"for_inf_end_WS2812B:"
	"beq t1, zero, if_zero_affor;"
	  "nop; nop; nop;nop; nop;"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "j if_zero_affor_end;"
	"if_zero_affor:"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "nop; nop; nop;nop; nop;"
	"if_zero_affor_end:"
	//    %0          %1       %2         %3           %4       %5
	: : "r"(ptr), "r"(port), "r"(p), "r"(bitMask), "r"(end), "r"(port2) : "t0", "t1" ); 
#endif // WS2812B

#ifdef   WS2812
	__asm__ __volatile__(
	"addi t0, zero, 0x7;"
	"for_inf_WS2812:"
	"sw t0, 0(%1);"
	"sw t0, 0(%5);"
	"nop;"
	"and t1, %2, %3;"
	"beq t1, zero, if_zero;"
	  "srli %3, %3, 1;"
	  "bne %3, zero, if_zero_2_end;"
	    "bgeu %0, %4, for_inf_end_WS2812;"
	    "lw %2, 0(%0);"
	    "addi %0, %0, 4;"
	    "li %3, 0x800000;"
	  "if_zero_2_end:"
	  // ----- Dummy here
	  "nop; nop; nop; nop;"
	  // ----- Dummy here
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "j if_zero_end;"
	"if_zero:"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  // ----- Dummy here
	  "nop; nop; nop;nop;"
	  // ----- Dummy here
	  "srli %3, %3, 1;"
	  "bne %3, zero, if_zero_3_end;"
	    "bgeu %0, %4, for_inf_end_WS2812;"
	    "lw %2, 0(%0);"
	    "addi %0, %0, 4;"
	    "li %3, 0x800000;"
	  "if_zero_3_end:"
	"if_zero_end:"
	"nop; nop; nop; nop; nop; nop;"
	"j for_inf_WS2812;"
	"for_inf_end_WS2812:"
	"beq t1, zero, if_zero_affor;"
	  "nop; nop;"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "j if_zero_affor_end;"
	"if_zero_affor:"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "nop; nop;"
	"if_zero_affor_end:"
	//    %0          %1       %2         %3           %4       %5
	: : "r"(ptr), "r"(port), "r"(p), "r"(bitMask), "r"(end), "r"(port2) : "t0", "t1" ); 
#endif // WS2812

#ifdef   WS2811
	__asm__ __volatile__(
	"addi t0, zero, 0x7;"
	"for_inf_WS2811:"
	"sw t0, 0(%1);"
	"sw t0, 0(%5);"
	"nop; nop; nop;"
	"and t1, %2, %3;"
	"beq t1, zero, if_zero;"
	  "srli %3, %3, 1;"
	  "bne %3, zero, if_zero_2_end;"
	    "bgeu %0, %4, for_inf_end_WS2811;"
	    "lw %2, 0(%0);"
	    "addi %0, %0, 4;"
	    "li %3, 0x800000;"
	  "if_zero_2_end:"
	  // ----- Dummy here
	  "nop; nop; nop; nop; nop; nop; nop; nop; nop;"
	  // ----- Dummy here
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "j if_zero_end;"
	"if_zero:"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  // ----- Dummy here
	  "nop; nop; nop;nop; nop; nop;nop; nop; nop;"
	  // ----- Dummy here
	  "srli %3, %3, 1;"
	  "bne %3, zero, if_zero_3_end;"
	    "bgeu %0, %4, for_inf_end_WS2811;"
	    "lw %2, 0(%0);"
	    "addi %0, %0, 4;"
	    "li %3, 0x800000;"
	  "if_zero_3_end:"
	"if_zero_end:"
	"nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; nop; "
	"j for_inf_WS2811;"
	"for_inf_end_WS2811:"
	"beq t1, zero, if_zero_affor;"
	  "nop; nop; nop;nop; nop;"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "j if_zero_affor_end;"
	"if_zero_affor:"
	  "sw zero, 0(%1);"
	  "sw zero, 0(%5);"
	  "nop; nop; nop;nop; nop;"
	"if_zero_affor_end:"
	//    %0          %1       %2         %3           %4       %5
	: : "r"(ptr), "r"(port), "r"(p), "r"(bitMask), "r"(end), "r"(port2) : "t0", "t1" ); 
#endif // WS2811

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
