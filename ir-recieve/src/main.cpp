// #define USE_ONKYO_PROTOCOL    // Like NEC, but take the 16 bit address and command each as one 16 bit value and not as 8 bit normal and 8 bit inverted value.
// #define USE_FAST_PROTOCOL // Use FAST protocol instead of NEC / ONKYO
#define IR_RECEIVE_PIN 35
// dont use pin 13 its fucked up
#include "TinyIRReceiver.hpp"

void setup()
{
  Serial.begin(115200);
  while (!Serial)
    ; // Wait for Serial to become available. Is optimized away for some cores.

  initPCIInterruptForTinyReceiver(); // Enables the interrupt generation on change of IR input signal

#if defined(USE_FAST_PROTOCOL)
  Serial.print("Using FAST: ");
#else
  Serial.print("Using NEC: ");

#endif
  Serial.println("Ready to receive");
}

int i = 0;
void loop()
{
  if (TinyIRReceiverData.justWritten)
  {
    Serial.println("Got something");
    TinyIRReceiverData.justWritten = false;
#if !defined(USE_FAST_PROTOCOL)
    // We have no address at FAST protocol
    Serial.print(F("Address=0x"));
    Serial.print(TinyIRReceiverData.Address, HEX);
    Serial.print(' ');
#endif
    Serial.print(F("Command=0x"));
    Serial.print(TinyIRReceiverData.Command, HEX);
    if (TinyIRReceiverData.Flags == IRDATA_FLAGS_IS_REPEAT)
    {
      Serial.print(F(" Repeat"));
    }
    if (TinyIRReceiverData.Flags == IRDATA_FLAGS_PARITY_FAILED)
    {
      Serial.print(F(" Parity failed"));
#if !defined(USE_EXTENDED_NEC_PROTOCOL) && !defined(USE_ONKYO_PROTOCOL)
      Serial.print(F(", try USE_EXTENDED_NEC_PROTOCOL or USE_ONKYO_PROTOCOL"));
#endif
    }
    Serial.println();
  }

  // i++;
  // if (i > 9999)
  // {
  //   i = 0;
  // }

  // if (i % 500 == 0)
  // {
  //   Serial.print("loop:");
  //   Serial.println(i);
  // }
}
