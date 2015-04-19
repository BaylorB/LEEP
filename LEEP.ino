////////////////////////
// DEFINED CONSTANTS////
////////////////////////
#define NUM_ROWS         4
#define NUM_COLS         4

// Inputs are what the wires go into, one for each row and column. Used together
// we can see what individual button is active.
#define NUM_INPUTS       NUM_ROWS+NUM_COLS      // 4 rows, 4 columns

// Each button, and each button has a light indexed in serial.
#define NUM_BUTTONS      NUM_ROWS * NUM_COLS    // 16 buttons

#define SERIAL9600



// The input wires for the rows and columns of the grid. Top row is purple striped wire pin 12,
// left column is brown wire pin 8.
byte rowToPinNumber[NUM_ROWS] = {12, 11, 10, 9};
byte minRowPinNumber = 9;
byte maxRowPinNumber = 12;
byte colToPinNumbers[NUM_COLS] = {8, 7, 6, 5}; 
byte minColPinNumber = 5;
byte maxColPinNumber = 8;



// MakeyMakey struct for keeping track of input state.
typedef struct
{
  byte pinNumber;
  int keyCode;
  int timePressed;
  float movingAverage;
  boolean pressed;
  boolean prevPressed;
} MakeyMakeyInput;
MakeyMakeyInput inputs[NUM_INPUTS];

int inputToPinNumber[NUM_INPUTS] =
{        
  12, 11, 10, 9, // Rows      
  8, 7, 6, 5  // Cols
};



// Button struct for keeping track of button/light state.
typedef struct
{
  boolean pressed;
  boolean state;
  boolean highlight;
} Button;
Button buttons [NUM_BUTTONS];


#define NEO_PIN 4
#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_BUTTONS, NEO_PIN, NEO_GRB + NEO_KHZ800);


// Variables

// colors
uint32_t ledColor = 0;
uint32_t ledColorNone = 255;
uint32_t ledHighlightColor = 255;
uint32_t ledHighlightOnColor = 255;

// MakeyMakey
float movingAverageFactor = 1;
float pressThreshold = 4.5;
float releaseThreshold = 3.6;
int triggerThresh = 200;
boolean inputChanged;

// LED that indicates when key is pressed
const int outputK = 13;
byte ledCycleCounter = 0;

// timing
int loopTime = 0;
int prevTime = 0;
int loopCounter = 0;



void initializeArduino();
void initializeInputs(); 
void initializeNeopixels();
void updateMovingAverage();
void updateInputStates();
void addDelay();
void updateOutLED();
void updateNeopixels();
void clearNeopixels();


void setup() 
{
  initializeArduino();
  initializeInputs();
  initializeNeopixels();
  delay(100);
}

void loop() 
{ 
  checkSerialInput();
  updateMovingAverage();
  updateInputStates();
  updateOutLED();
}

void initializeArduino()
{

  Serial.begin(9600);  
  while (!Serial) 
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  
  /* Set up input pins 
   DEactivate the internal pull-ups, since we're using external resistors */
  for(int input=0; input < NUM_INPUTS; input++)
  {
    pinMode(inputToPinNumber[input], INPUT);
    digitalWrite(inputToPinNumber[input], LOW);
  }
  
  // Set up the led that indicates when key is pressed
  pinMode(outputK, OUTPUT);
  digitalWrite(outputK, LOW);

#ifdef DEBUG
  delay(4000); // allow us time to reprogram in case things are freaking out
#endif
}

void initializeInputs()
{
  for(int input = 0; input < NUM_INPUTS; input++)
  {
    inputs[input].pinNumber = inputToPinNumber[input];
//    inputs[input].keyCode = keyCodes[i]; // TODO what?
    inputs[input].movingAverage = 0;
    
    inputs[input].pressed = false;
    inputs[input].prevPressed = false;

#ifdef DEBUG
    Serial.println(input);
#endif

  }
}

void initializeNeopixels()
{
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  clearNeopixels();

  ledColor = strip.Color(0, 255, 0); // green
  ledColorNone = strip.Color(0, 0, 0);
  ledHighlightColor = strip.Color(23, 114, 255);
  ledHighlightOnColor = strip.Color(10, 255, 79);
}


// TODO not declared?
void checkSerialInput() {

  /*
    Incoming byte is 0 - 255. We find out what range its in and then what
    button it corresponds to. Here's the ranges:
  */
  byte const noIncomingSerialData = 0;
  byte const baseTurnLEDOn = 1; // -> NUM_BUTTONS
  byte const baseTurnLEDOff = baseTurnLEDOn + NUM_BUTTONS + 1; // -> NUM_BUTTONS
  byte const clearAll = baseTurnLEDOff + NUM_BUTTONS + 1;
  byte const baseHighlightColumn = clearAll + 1; // -> NUM_COLS
  byte const baseColorChange = baseHighlightColumn + NUM_COLS + 1;

  /*
    noIncomingSerialData
			no incoming serial data
    baseTurnLEDOn -> baseTurnLEDOn+NUM_BUTTONS
			strip.setPixelColor(i, ledColor) where i is between 0-NUM_BUTTONS (turn LED on)
    baseTurnLEDOff -> baseTurnLEDOff+NUM_BUTTONS
			strip.setPixelColor(i, 0) where i is between 0-NUM_BUTTONS (turn LED off)
    clearAll
			clear monome
    baseColorChange -> baseColorChange+NUM_COLS
			highlight columns 0-NUM_COLS
    baseColorChange -> 
			corresponds to a color change of the LEDs
  */

  if (Serial.available() > 0) {
		
  // get incoming byte
  byte inByte = Serial.read();
      
  // Turn LED on?
  if(inByte >= baseTurnLEDOn && inByte <= baseTurnLEDOn + NUM_BUTTONS)
  {
    int button = inByte - baseTurnLEDOn; // 0 = the first button
    buttons[button].state = true;
    //updateNeopixels;
  }
  // Turn LED off?
  else if(inByte >= baseTurnLEDOff && inByte <= baseTurnLEDOff + NUM_BUTTONS)
  {
    int button = inByte - baseTurnLEDOn; // 0 = the first button
    buttons[button].state = false;
    //updateNeopixels();
  }
  // Clear monome?
  else if (inByte == clearAll)
  {
    clearNeopixels();
  }
/*
  // Highlight column?
  else if (inByte >= baseHighlightColumn && inByte <= baseHighlightColumn + NUM_COLS)
  {
    int column = inByte - baseHighlightColumn; // 0 = the first column
    highlightColumn(column);
  }
  // Change ledColor?
  else 
  {
    int ledColor = inByte - baseColorChange; // 0 = first color ??
  }
*/
  }
}



/*
  For each input, update the moving average in inputs[] with the new value
  blended with the old value.
*/
void updateMovingAverage()
{
  for(int input = 0; input < NUM_INPUTS; input++) 
  {
    int cycles = readCapacitivePin(inputToPinNumber[input]);
    int mave = inputs[input].movingAverage;
    inputs[input].movingAverage = mave * (1.0 - movingAverageFactor) + cycles * movingAverageFactor;
  }
}

void updateInputStates()
{
  inputChanged = false;
  for(int input = 0; input < NUM_INPUTS; input++) 
  {
    // Store previous value (only used for mouse buttons)
    inputs[input].prevPressed = inputs[input].pressed; 

    // If we're pressed, and our average is below the releaseThreshold, mark us unpressed.
    if(inputs[input].pressed) 
    {
      if(inputs[input].movingAverage < releaseThreshold)
      {
        inputChanged = true;
        inputs[input].pressed = false;
      }
    }
    // If we weren't pressed, and our average is above the pressThreshold, mark us pressed
    else if(!inputs[input].pressed) 
    {
      if(inputs[input].movingAverage > pressThreshold) 
      {
        inputChanged = true;
        inputs[input].pressed = true; 

      }
    }

    updateNeopixels(); 
  } // for input

#ifdef DEBUG3
  if (inputChanged) {
    Serial.println("change");
  }
#endif
}


void updateNeopixels()
{
  for (int button = 0; button < NUM_BUTTONS; button++)
  {
    int stripIndex = buttonToStripIndex(button);
    uint32_t color = 0;
    
    
    if(buttons[button].state)
    {
      if(buttons[button].highlight) 
      {
        color = ledHighlightOnColor;
      }
      else
      {
        color = ledColor;
      }
    }
    else
    {
      if(buttons[button].highlight)
      {
        color = ledHighlightColor;
      }
      else
      {
        color = ledColorNone;
      }
    }

    strip.setPixelColor(stripIndex, color);
  }
  strip.show();
}


// since we zig zag the Neopixel strip, test if this is an even or odd row
// and return the strip index that corresponds to the button index
int buttonToStripIndex(int button)
{
  int row = button / NUM_ROWS;
  
  // Get the row. If it's the first or every other row, just use the button index.
  if(row % 2 == 0)
  {
    return button;
  }
  
  // Otherwise we're on a row that is backwards.
  int col = button % NUM_COLS;
  int flippedCol = NUM_COLS-1 - col; 
  return (row * NUM_COLS) + flippedCol;
}

void clearNeopixels()
{
  for(int button = 0; button < NUM_BUTTONS; button++)
  {
    strip.setPixelColor(button, ledColorNone);
  }
  strip.show();
}


void updateOutLED()
{
  boolean keyPressed = false;
  for(int input = 0; input < NUM_INPUTS; input++) 
  {
    if (inputs[input].pressed) 
    {
      
      keyPressed = 1;
#ifdef DEBUG
      Serial.print("Key ");
      Serial.print(i);
      Serial.println(" pressed");
#endif
    }
  }

  if (keyPressed)
  {
    digitalWrite(outputK, HIGH);
  }
  else
  {       
    digitalWrite(outputK, LOW);
  }
}


uint8_t readCapacitivePin(int pinToMeasure)
{
  // Variables used to translate from Arduino to AVR pin naming
  volatile uint8_t* port;
  volatile uint8_t* ddr;
  volatile uint8_t* pin;
  // Here we translate the input pin number from
  //  Arduino pin number to the AVR PORT, PIN, DDR,
  //  and which bit of those registers we care about.
  byte bitmask;
  port = portOutputRegister(digitalPinToPort(pinToMeasure));
  ddr = portModeRegister(digitalPinToPort(pinToMeasure));
  bitmask = digitalPinToBitMask(pinToMeasure);
  pin = portInputRegister(digitalPinToPort(pinToMeasure));
  // Discharge the pin first by setting it low and output
  *port &= ~(bitmask);
  *ddr  |= bitmask;
  delay(1);
  // Prevent the timer IRQ from disturbing our measurement
  noInterrupts();
  // Make the pin an input with the internal pull-up on
  *ddr &= ~(bitmask);
  *port |= bitmask;

  // Now see how long the pin to get pulled up. This manual unrolling of the loop
  // decreases the number of hardware cycles between each read of the pin,
  // thus increasing sensitivity.
  uint8_t cycles = 17;
       if (*pin & bitmask) { cycles =  0;}
  else if (*pin & bitmask) { cycles =  1;}
  else if (*pin & bitmask) { cycles =  2;}
  else if (*pin & bitmask) { cycles =  3;}
  else if (*pin & bitmask) { cycles =  4;}
  else if (*pin & bitmask) { cycles =  5;}
  else if (*pin & bitmask) { cycles =  6;}
  else if (*pin & bitmask) { cycles =  7;}
  else if (*pin & bitmask) { cycles =  8;}
  else if (*pin & bitmask) { cycles =  9;}
  else if (*pin & bitmask) { cycles = 10;}
  else if (*pin & bitmask) { cycles = 11;}
  else if (*pin & bitmask) { cycles = 12;}
  else if (*pin & bitmask) { cycles = 13;}
  else if (*pin & bitmask) { cycles = 14;}
  else if (*pin & bitmask) { cycles = 15;}
  else if (*pin & bitmask) { cycles = 16;}

  // End of timing-critical section
  interrupts();

  // Discharge the pin again by setting it low and output
  //  It's important to leave the pins low if you want to 
  //  be able to touch more than 1 sensor at a time - if
  //  the sensor is left pulled high, when you touch
  //  two sensors, your body will transfer the charge between
  //  sensors.
  *port &= ~(bitmask);
  *ddr  |= bitmask;

  return cycles;
}

