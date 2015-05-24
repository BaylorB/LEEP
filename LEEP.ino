////////////////////////
// DEFINED CONSTANTS////
////////////////////////
#define NUM_ROWS         4
#define NUM_COLS         4

#define NUM_NEOPIXELS    NUM_ROWS * NUM_COLS

#define SERIAL9600

//#define DEBUG 1

// The input wires for the rows and columns of the grid. Top row is purple striped wire pin 12,
// left column is brown wire pin 8.
byte rowToPinNumber[NUM_ROWS] = {9, 10, 11, 12};
byte colToPinNumber[NUM_COLS] = {5, 6, 7, 8}; 

// LED that indicates when key is pressed
const int ledPin = 13;

// Button struct for keeping track of button/light state.
typedef struct
{
  boolean pressed;
  boolean highlight;
  boolean handled;
} Button;
Button buttons [NUM_ROWS][NUM_COLS];


#define NEO_PIN 4
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_NEOPIXELS, NEO_PIN, NEO_GRB + NEO_KHZ800);

// colors
uint32_t ledColor = 0;
uint32_t ledColorNone = 255;
uint32_t ledHighlightColor = 255;
uint32_t ledHighlightOnColor = 255;


int colRowToNeopixelIndex(int col, int row)
{
#ifdef DEBUG
  Serial.print("colRowToNeopixelIndex ");
  Serial.print(col);
  Serial.print(" ");
  Serial.print(row);
#endif
 
  int index = 0;
  index = row + ((NUM_COLS-1 - col) * NUM_ROWS);
  
#ifdef DEBUG
  Serial.print(" - index ");
  Serial.println(index);
#endif

  return index;
}



void setup() 
{
  // initialize serial port
  Serial.begin(9600);
  
  // set Digital Pins 9-12 as OUTPUTS
  for(int col = 0; col < NUM_COLS; col++)
  {
    pinMode(colToPinNumber[col], OUTPUT);
#ifdef DEBUG
    Serial.print("digital pin ");
    Serial.print(colToPinNumber[col]);
    Serial.println(" set as output");
#endif
  }

  // set Digital Pins 5-8 (IC Pins 11-14) as INPUTS pulled high
  for(int row = 0; row < NUM_ROWS; row++)
  {
    pinMode(rowToPinNumber[row], INPUT_PULLUP);
#ifdef DEBUG
    Serial.print("digital pin ");
    Serial.print(rowToPinNumber[row]);
    Serial.println(" set as input");  
#endif
  }
  
  // initialize digital pin as an output (for LED)
  pinMode(ledPin, OUTPUT);


  // Initialize button data.
  for(int row = 0; row < NUM_ROWS; row++)
  {
    for(int col = 0; col < NUM_COLS; col++)
    {
      buttons[row][col].pressed = false;
      buttons[row][col].highlight = false;
      buttons[row][col].handled = false;
    }
  }
  

  // Initialize NeoPixel colors
  ledColor = strip.Color(0, 255, 0); // green
  ledColorNone = strip.Color(0, 0, 0);
  ledHighlightColor = strip.Color(23, 114, 255);
  ledHighlightOnColor = strip.Color(10, 255, 79);

  // Initialize NeoPixels
  strip.begin();
  for(int neopixel = 0; neopixel < NUM_NEOPIXELS; neopixel++)
  {
    strip.setPixelColor(neopixel, ledColorNone);
  }
  strip.show();

  delay(100);
}

void loop() 
{ 
  // all columns (outputs) are set high to start
  for(int col = 0; col < NUM_COLS; col++)
  {
    digitalWrite(colToPinNumber[col], HIGH);
  }
  // light LED as we start reading data
  digitalWrite(ledPin, HIGH);

  // scan across columns - make each column go low, one at a time
  for(int col = 0; col < NUM_COLS; col++)
  {
    digitalWrite(colToPinNumber[col], LOW);
    
    // scan across rows testing switches and putting data in array
    for(int row = 0; row < NUM_ROWS; row++)
    {
      // "Read" the voltage present on the pin (check switch)
      int switchData = digitalRead(rowToPinNumber[row]);

      // Update button pressed state. We're pressed if switchData is 0.
      buttons[row][col].pressed = (switchData == 0);
      
#ifdef DEBUG
      if(switchData == 0)
      {
        Serial.print("Pressing col row ");
        Serial.print(col);
        Serial.print(" ");
        Serial.println(row);
      }
#endif
      
      delay(20);  
    }
    
    // raise column back HIGH
    digitalWrite(colToPinNumber[col], HIGH);
  }
        
  // turn off LED - done reading data
  digitalWrite(ledPin, LOW);
  
  // use to test loop speed
#ifdef DEBUG
  delay(1000);
#endif

  // Update our lights.
  for (int row = 0; row < NUM_ROWS; row++)
  {
    for(int col = 0; col < NUM_COLS; col++)
    {
      uint32_t color = 0;
      
      if(buttons[row][col].pressed)
      {
        // We're being pressed, have we handled this press yet? If not
        if(!buttons[row][col].handled)
        {
          // toggle highlight state.
          buttons[row][col].highlight = !buttons[row][col].highlight;
          
          // Set our handled flag so we don't toggle again this press.
          buttons[row][col].handled = true;
        }
        color = ledHighlightOnColor;
      }
      else
      {
        // Clear handled flag since we're not being pressed any more.
        buttons[row][col].handled = false;

        if(buttons[row][col].highlight) 
        {
          color = ledHighlightColor;
        }
        else
        {
          color = ledColorNone;
        }
      }

      int neopixelIndex = colRowToNeopixelIndex(col, row);
      strip.setPixelColor(neopixelIndex, color);
    }
  }
  strip.show();
}


