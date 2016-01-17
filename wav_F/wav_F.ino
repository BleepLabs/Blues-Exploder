/*
The Jon Spencer Blues Exploder 001
by Bleep Labs
wave player chip code

Thanks to Adafruit and co for waveHC
https://code.google.com/p/wavehc/

This uses a slightly modifeid waveHC lib that adds a stutter and crush effect.

*/

#include <FatReader.h>
#include <SdReader.h>
#include <avr/pgmspace.h>
#include "WaveUtil.h"
#include "WaveHC.h"

SdReader card;    // This object holds the information for the card
FatVolume vol;    // This holds the information for the partition on the card
FatReader root;   // This holds the information for the filesystem on the card
FatReader f;      // This holds the information for the file we're play
int gp;
WaveHC wave;      // This is the only wave (audio) object, since we will only play one at a time
byte pressed;
uint16_t count1, seek1, random1, random2, pot2, pot1b;
uint32_t cpos, ppos;
int p8, p7, p6, p5, p14, p9, c, read1;
byte t7, t8, t6, t5, t14, t9, p, pmode, cmode;
int pot1, en;
unsigned int  capsamples = 10;
unsigned int jmax = 5000;
uint8_t gb;
byte threshold = 8;
long current, prev;
byte b1, b2, b3, b4, b5, b6, b7, b8, b9;
byte a1, a2, a3, a4, a5, a6, a7, a8, a9;
byte latchbutton, platchbutton;
byte buttonbank[8] = {
  14, 8, 15, 7, 16, 6, 17, 18
};
byte pbutton[8];
byte button[8];
byte latch = 0;
byte act[8];
byte pact[8];

byte br, pbr;
void sdErrorCheck(void)
{
  if (!card.errorCode()) return;
  putstring("\n\rSD I/O error: ");
  Serial.print(card.errorCode(), HEX);
  putstring(", ");
  Serial.println(card.errorData(), HEX);
  while (1);
}

void setup() {
  Serial.begin(9600);
  delay(10);
  p8 = cPin_8.readPin(capsamples);
  p7 = cPin_7.readPin(capsamples);
  delay(10);
  byte i;
  // set up serial port
  randomSeed(2);
  // Set the output pins for the DAC control. This pins are defined in the library
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);


  pinMode(6, INPUT);
  digitalWrite(6, HIGH);
  pinMode(7, INPUT);
  digitalWrite(7, HIGH);
  pinMode(8, INPUT);
  digitalWrite(8, HIGH);
  pinMode(9, INPUT);
  digitalWrite(9, HIGH);
  pinMode(14, INPUT);
  digitalWrite(14, HIGH);
  pinMode(15, INPUT);
  digitalWrite(15, HIGH);
  pinMode(16, INPUT);
  digitalWrite(16, HIGH);
  pinMode(17, INPUT);
  digitalWrite(17, HIGH);
  pinMode(18, INPUT);
  digitalWrite(18, HIGH);



  //  if (!card.init(true)) { //play with 4 MHz spi if 8MHz isn't working for you
  if (!card.init()) {         //play with 8 MHz spi (default faster!)
    putstring_nl("Card init. failed!");  // Something went wrong, lets print out why
    sdErrorCheck();
    while (1);                           // then 'halt' - do nothing!
  }

  // enable optimize read - some cards may timeout. Disable if you're having problems
  card.partialBlockRead(true);

  // Now we will look for a FAT partition!
  uint8_t part;
  for (part = 0; part < 5; part++) {     // we have up to 5 slots to look in
    if (vol.init(card, part))
      break;                             // we found one, lets bail
  }
  if (part == 5) {                       // if we ended up not finding one  :(
    putstring_nl("No valid FAT partition!");
    sdErrorCheck();      // Something went wrong, lets print out why
    while (1);                           // then 'halt' - do nothing!
  }

  // Lets tell the user about what we found
  putstring("Using partition ");
  Serial.print(part, DEC);
  putstring(", type is FAT");
  Serial.println(vol.fatType(), DEC);    // FAT16 or FAT32?

  // Try to open the root directory
  if (!root.openRoot(vol)) {
    putstring_nl("Can't open root dir!"); // Something went wrong,
    while (1);                            // then 'halt' - do nothing!
  }

  // Whew! We got past the tough parts.
  putstring_nl("Ready!");

  TCCR2A = 0;
  TCCR2B = 1 << CS22 | 0 << CS21 | 0 << CS20;

  //Timer2 Overflow Interrupt Enable
  TIMSK2 |= 1 << TOIE2;


}

SIGNAL(TIMER2_OVF_vect) {
}

void ctrlup() {
  /*
    Serial.print(act[0]);

    Serial.print(act[1]);

    Serial.print(act[2]);

    Serial.print(act[3]);

    Serial.print(act[4]);

    Serial.print(act[5]);

    Serial.print(act[6]);

    Serial.println(act[7]);
  */


  latch = digitalRead(9);

  for (br = 0; br < 8; br++) {
    button[br] = digitalRead(buttonbank[br]);

    if (latch == 1) {
      act[br] = !button[br]  ;
    }

    if (latch == 0) {

      if (button[br] != pbutton[br] && button[br] == 0) {

        //////  "for" to restet was going to slow...


        if (act[br] == 0) {
          //    Serial.println(" zero  ");
          act[0] = 0; act[1] = 0;  act[2] = 0; act[3] = 0; act[4] = 0; act[5] = 0; act[7] = 0; act[6] = 0;
          act[br] = 1;
          //pact[br]=act[br];
        }

        else {
          //    Serial.println(" one  ");
          act[0] = 0; act[1] = 0;  act[2] = 0; act[3] = 0; act[4] = 0; act[5] = 0; act[7] = 0; act[6] = 0;
        }
      }


    }

    pbutton[br] = button[br];

  }


  cpos = (wave.getSize()) - (wave.remainingBytesInChunk);


  // pot2=analogRead(5);


  pot1 = analogRead(5);
  pmode = cmode;


  if (pot1 < 595 && pot1 > 490) {
    cmode = 0;
    wave.glitch(0, pot1b);

  }

  if (pot1 >= 595) {
    cmode = 1;
    wave.glitch(0, pot1b);

  }

  if (pot1 <= 490) {
    cmode = 2;
    pot1b = ((pot1 >> 1) - 245) * -1;
    wave.glitch(1, pot1b);
  }

  if (cmode == 1 && pmode != 1) {
    ppos = cpos;
  }

  if (cmode == 1 && pmode == 1) {
    gp = (pot1 - 595);
    count1 = wave.crush((gp));
    if (count1 == 1) {
      seek1 = pot1 >> 3;
      wave.seek(seek1 + ppos);
    }
  }

}

void loop() {
  ctrlup();

  if (act[0]) {
    playfile("1.WAV");
    while (wave.isplaying && act[0] == 1) {
      ctrlup();
    }
    wave.stop();
  }

  if (act[1]) {
    playfile("2.WAV");
    while (wave.isplaying && act[1] == 1) {
      ctrlup();
    }
    wave.stop();
  }
  if (act[2]) {
    playfile("3.WAV");
    while (wave.isplaying && act[2] == 1) {
      ctrlup();
    }
    wave.stop();
  }
  if (act[3]) {
    playfile("4.WAV");
    while (wave.isplaying && act[3] == 1) {
      ctrlup();
    }
    wave.stop();
  }
  if (act[4]) {
    playfile("5.WAV");
    while (wave.isplaying && act[4] == 1) {
      ctrlup();
    }
    wave.stop();
  }
  if (act[5]) {
    playfile("6.WAV");
    while (wave.isplaying && act[5] == 1) {
      ctrlup();
    }
    wave.stop();
  }
  if (act[6]) {
    playfile("7.WAV");
    while (wave.isplaying && act[6] == 1) {
      ctrlup();
    }
    wave.stop();
  }
  if (act[7]) {
    playfile("8.WAV");
    while (wave.isplaying && act[7] == 1) {
      ctrlup();
    }
    wave.stop();
  }


}


// Plays a full file from beginning to end with no pause.
void playcomplete(char *name) {
  // call our helper to find and play this name
  playfile(name);
  while (wave.isplaying) {
    // do nothing while its playing
  }
  // now its done playing
}

void playfile(char *name) {
  // see if the wave object is currently doing something
  if (wave.isplaying) {// already playing something, so stop it!
    wave.stop(); // stop it
  }
  // look in the root directory and open the file
  if (!f.open(root, name)) {
    putstring("Couldn't open file ");
    Serial.print(name);
    return;
  }
  // OK read the file and turn it into a wave object
  if (!wave.create(f)) {
    putstring_nl("Not a valid WAV");
    return;
  }

  // ok time to play! start playback
  wave.play();
}











