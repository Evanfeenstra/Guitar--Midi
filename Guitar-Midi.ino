/*
 
  Guitar -> MIDI monophonic pitch detection
  Also plays piched samples directly from SD card

  Audio library by Paul Stoffregen http://www.pjrc.com/teensy/td_libs_Audio.html
  Tuner library: https://github.com/duff2013/AudioTuner, using the YIN algorithm:
  http://recherche.ircam.fr/equipes/pcm/cheveign/pss/2002_JASA_YIN.pdf
 
 */
#include <math.h>
#include <AudioTuner.h>
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>

AudioInputI2S             input;        
AudioOutputI2S            dac;       
AudioTuner                tuner;
AudioPlaySdWav           playSd;

AudioConnection patchCord1(input,  0, tuner, 0);
AudioConnection patchCord3(playSd, 0, dac, 0);
//AudioConnection patchCord3(input, 0, dac, 0); //uncomment for passthru audio
AudioControlSGTL5000     sgtl5000_1;

int base_a4 = 440;
int midi=0;
int oldmidi=0;

void setup() {
    // put your setup code here, to run once:
    AudioMemory(5);
    tuner.set_threshold( .05f );

    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);
  
    SPI.setMOSI(7);
    SPI.setSCK(14);
    if (!(SD.begin(10))) {
      // stop here, but print a message repetitively
      while (1) {
        //Serial.println("Unable to access the SD card");
        delay(5);
      }
    }
}

void loop() {
    // put your main code here, to run repeatedly:
    if (tuner.available()) {
        float note = tuner.read();
        // interval ratio away from A 440hZ
        float ratio = note / 400;
        // float prob = tuner.probability(); // guitar ~ 99% probability
        // Serial.printf("Note: %3.2f | Probility: %.2f\n", note, prob);
        oldmidi=midi;
        midi = round(12*(log(note/base_a4)/log(2))+69);
        if(oldmidi!=midi){

          //send midi notes 
          usbMIDI.sendNoteOff(oldmidi, 127, 1);
          usbMIDI.sendNoteOn(midi, 127, 1);

          //or play samples directly from SD card
          //with Mozzi library we could pitch shift samples to match guitar:
          //sound1.setFreq(440*ratio);
          //playFile("sound1.wav");
          //with Paul's Audio library, we will have to load in samples for each note:
          char string = "sound" + String(midi) + ".wav";
          playFile(string);
        }
        //Serial.println(round(12*(log(note/base_a4)/log(2))+69));
    }   
}


void playFile(const char *filename)
{
  //Serial.print("Playing file: ");
  //Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playSd.play(filename);

  // A brief delay for the library read WAV info
  delay(5);

  // Simply wait for the file to finish playing.
  while (playSd.isPlaying()) {

    //volume pot
    float vol = analogRead(15);
    vol = vol / 1024;
    sgtl5000_1.volume(vol);
  }
}
