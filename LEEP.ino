// Includes
#include <Adafruit_NeoPixel.h>

////////////////////////
// DEFINED CONSTANTS////
////////////////////////
#define NUM_ROWS         4
#define NUM_COLS         4

#define NUM_NEOPIXELS    (NUM_ROWS * NUM_COLS)

#define COLOR_NONE      0x000000
#define RED             0xFF0000
#define RED_DIM         0x2F0000
#define GREEN           0x00FF00
#define GREEN_DIM       0x002F00
#define BLUE            0x0000FF
#define BLUE_DIM        0x00002F
#define CYAN            0x00FFFF
#define MAGENTA         0xFF00FF
#define YELLOW          0xC0FF00
#define YELLOW_DIM      0x182000

#define LIGHT_BLUE      0x1772FF
#define LIGHT_GREEN     0x0AFF4F

#define LONG_PRESS_MS   1000

// Blue Board LED
const int ledPin = 13;

// This class manages the button board.
class ButtonHandler
{
public:
  ButtonHandler()
  : m_buttons() // initialize the buttons to off/up.
  , m_nMaxStates(2)
  {
  }

  void initialize()
  {
    // set Digital Pins 9-12 as OUTPUTS
    for(uint8_t col = 0; col < NUM_COLS; col++)
    {
      pinMode(ms_colToPinNumber[col], OUTPUT);
    }
  
    // set Digital Pins 5-8 (IC Pins 11-14) as INPUTS pulled high
    for(uint8_t row = 0; row < NUM_ROWS; row++)
    {
      pinMode(ms_rowToPinNumber[row], INPUT_PULLUP);
    }
    
    // all columns (outputs) are set high to start
    for(uint8_t col = 0; col < NUM_COLS; col++)
    {
      digitalWrite(ms_colToPinNumber[col], HIGH);
    }
  }

  void reset()
  {
    for (uint8_t row = 0 ; row < NUM_ROWS ; ++row)
    {
      for (uint8_t col = 0 ; col < NUM_COLS ; ++col)
      {
        m_buttons[row][col].state = 0;
      }
    }
  }

  void setNumStates(uint8_t nNumStates)
  {
    m_nMaxStates = nNumStates;
    reset();
  }

  bool updateButtonState()
  {
    bool bButtonStateChanged = false;

    // scan across columns - make each column go low, one at a time
    for(uint8_t col = 0; col < NUM_COLS; col++)
    {
      // set the column low
      digitalWrite(ms_colToPinNumber[col], LOW);
      // wait for the voltage to stabilize
      delay(5);  //button reaction delay//
  
      // scan across rows testing switches and putting data in array
      for (int row = 0; row < NUM_ROWS; row++)
      {
        // Read the pin (check switch) -- we're pressed if switchData is 0.
        bool bPressed = !digitalRead(ms_rowToPinNumber[row]);

        // if the state has changed
        if (m_buttons[row][col].pressed != bPressed)
        {
          // Update button pressed state. 
          m_buttons[row][col].pressed = bPressed;
          // if it's going from up to down
          if (bPressed)
          {
            // on the press, let's update our button to flip.
            if (++m_buttons[row][col].state >= m_nMaxStates) {
              m_buttons[row][col].state = 0;
            }
          }
          // note that one of our buttons has changed state.
          bButtonStateChanged = true;
        }
      }
      
      // raise column back HIGH
      digitalWrite(ms_colToPinNumber[col], HIGH);
    }
    return bButtonStateChanged;
  }

  bool isButtonPressed(uint8_t row, uint8_t col)
  {
//    assert(row < NUM_ROWS);
//    assert(col < NUM_COLS);
    return m_buttons[row][col].pressed;
  }

  bool isButtonOn(uint8_t row, uint8_t col)
  {
//    assert(row < NUM_ROWS);
//    assert(col < NUM_COLS);
    return m_buttons[row][col].state;
  }

  uint8_t buttonState(uint8_t row, uint8_t col)
  {
//    assert(row < NUM_ROWS);
//    assert(col < NUM_COLS);
    return m_buttons[row][col].state;
  }

private:
  // Button struct for keeping track of button/light state.
  typedef struct
  {
    bool pressed;
    uint8_t state;
  } Button;
  
  Button m_buttons [NUM_ROWS][NUM_COLS];
  static const uint8_t ms_rowToPinNumber[];
  static const uint8_t ms_colToPinNumber[];
  uint8_t m_nMaxStates;
} g_buttons;

// The input wires for the rows and columns of the grid. Top row is purple striped wire pin 12,
// left column is brown wire pin 8.
const uint8_t ButtonHandler::ms_rowToPinNumber[] = {9, 10, 11, 12};
const uint8_t ButtonHandler::ms_colToPinNumber[] = {5, 6, 7, 8}; 

#define NEO_PIN 4
class LedHandler
{
public:
  LedHandler()
  : m_strip(NUM_NEOPIXELS, NEO_PIN, NEO_GRB | NEO_KHZ800)
  {
    
  }
  ~LedHandler()
  {
    
  }

  void initialize()
  {
    m_strip.begin();
    setAllPixels(0x0);
  }

  void beginUpdate()
  {
    // do nothing here for now
  }
  
  void setPixelColor(uint8_t row, uint8_t col, uint32_t color)
  {
    uint16_t index = colRowToNeopixelIndex(col,row);
    m_strip.setPixelColor(index, color);
  }

  void setAllPixels(uint32_t color)
  {
    for(uint16_t neopixel = 0; neopixel < NUM_NEOPIXELS; neopixel++)
    {
      m_strip.setPixelColor(neopixel, color);
    }
    m_strip.show();
  }

  void endUpdate()
  {
    // REVIEW: Should we make this conditional? (I don't think so, but TBD)
    m_strip.show();
  }
private:

Adafruit_NeoPixel m_strip;

  uint16_t colRowToNeopixelIndex(uint8_t col, uint8_t row)
  {
    uint16_t index = 0;
    index = row + ((NUM_COLS - 1 - col) * NUM_ROWS);
    
    return index;
  }
} g_leds;

void setup() 
{
  // initialize serial port
  Serial.begin(9600);
  // initialize digital pin as an output (for LED)
  pinMode(ledPin, OUTPUT);

  // Setup Buttons
  g_buttons.initialize();
  // Initialize NeoPixels
  g_leds.initialize();

  delay(100);
  
  rainbow(2);
  // light LED to show we've booted.
  digitalWrite(ledPin, HIGH);

  modeSpecificSetup();
}

// Basic board test -- lights go on when the button is pressed and go off
// when they are pressed again.
class BoardTest
{
public:
  void setup()
  {
    g_buttons.setNumStates(2);
    g_leds.setAllPixels(COLOR_NONE);
  }

  void loop()
  {
    g_leds.beginUpdate();
    // Update our lights.
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      for (uint8_t col = 0; col < NUM_COLS; col++)
      {
        uint32_t color = COLOR_NONE;
        
        if (g_buttons.isButtonPressed(row,col))
        {
          color = MAGENTA;
        }
        else if (g_buttons.isButtonOn(row,col))
        {
          color = BLUE;
        }
  
        g_leds.setPixelColor(row, col, color);
      }
    }
    g_leds.endUpdate();
  }
} g_boardTest;

class Drawing
{
public:
  void setup();

  void loop()
  {
    g_leds.beginUpdate();
    // Update our lights.
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      for (uint8_t col = 0; col < NUM_COLS; col++)
      {
        uint32_t color = ms_colorCycle[g_buttons.buttonState(row,col)];
  
        g_leds.setPixelColor(row, col, color);
      }
    }
    g_leds.endUpdate();
  }
public:
  static const uint32_t ms_colorCycle[];
} g_drawing;

const uint32_t Drawing::ms_colorCycle[] = {COLOR_NONE,BLUE,CYAN,GREEN,YELLOW,RED,MAGENTA};

void Drawing::setup()
{
  g_buttons.setNumStates(sizeof(ms_colorCycle) / sizeof(ms_colorCycle[0]));
  g_leds.setAllPixels(COLOR_NONE);
}

#define bitToggle(value, bit_no) (bitWrite(value, bit_no, !bitRead(value, bit_no)))

class LightsOut
{
public:
  LightsOut()
  {
    
  }
  void setup()
  {
    randomSeed(micros());
    g_buttons.setNumStates(2);

    // make all the colors dance
    lightDance();
    
    // now randomize the lights
    setupRandomBoard();
  }

  void loop()
  {
    // Update our lights.
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      for (uint8_t col = 0; col < NUM_COLS; col++)
      {
        if (g_buttons.isButtonOn(row,col))
        {
          pressButton(row,col);
        }
      }
    }

    // we've handled these buttons, so clear them.
    g_buttons.reset();
    
    updateDisplay();
    
    // check for lights
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      // if any lights are on, and all lights are not on
      if (m_litLeds[row] && (m_litLeds[row] != (1 << NUM_COLS) - 1))
      {
        // just return
        return;
      }
    }

    // If we get here, all the lights are out or all lights are on, so celebrate
    delay(500);

    if (m_litLeds[0]) {
      // All lights are on, I think this should be an easteregg win condition, so do something fun
      blueRainbow(2);
    } else {
      // Normal win condition
      lightDance();
      lightDance();
    }
    delay(500);

    // And start again
    setupRandomBoard();
  }
private:

  void pressButton(uint8_t row, uint8_t col)
  {
    if (row > 0)
    {
      bitToggle(m_litLeds[row - 1], col);
    }
    if (row < NUM_ROWS - 1)
    {
      bitToggle(m_litLeds[row + 1], col);
    }
    if (col > 0)
    {
      bitToggle(m_litLeds[row], col - 1);
    }
    if (col < NUM_COLS - 1)
    {
      bitToggle(m_litLeds[row], col + 1);
    }
    bitToggle(m_litLeds[row], col);
  }

  void setupRandomBoard()
  {
    // make sure the board is blank
    memset(m_litLeds, 0, sizeof(m_litLeds));
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      for (uint8_t col = 0; col < NUM_COLS; col++)
      {
        // choose a random value to decide whether to push the button or not.
        bool bOn = random(2);
        if (bOn)
        {
          pressButton(row, col);
        }
      }
    }
    updateDisplay();
  }

  void updateDisplay()
  {
    g_leds.beginUpdate();
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      for (uint8_t col = 0; col < NUM_COLS; col++)
      {
        // choose a random color from the cycle
        bool bOn = bitRead(m_litLeds[row], col);
        uint32_t color = bOn ? LIGHT_BLUE : COLOR_NONE;
        g_leds.setPixelColor(row, col, color);
      }
    }
    g_leds.endUpdate();
  }

  void lightDance()
  {
    // reuse Drawing's list of colors for this
    const uint8_t numColors = (sizeof(Drawing::ms_colorCycle) / sizeof(Drawing::ms_colorCycle[0]));
    const uint8_t numIterations = 8;
    for (uint8_t i = 0 ; i < numIterations ; ++i)
    {
      // For each button
      g_leds.beginUpdate();
      for (uint8_t row = 0; row < NUM_ROWS; row++)
      {
        for (uint8_t col = 0; col < NUM_COLS; col++)
        {
          // choose a random color from the cycle
          uint32_t color = Drawing::ms_colorCycle[random(numColors)];
          g_leds.setPixelColor(row, col, color);
        }
      }
      g_leds.endUpdate();
      delay(200);
    }
  }

  void blueRainbow(uint8_t wait) {
    uint8_t i = 0x72;
    uint32_t color = LIGHT_BLUE;
    // ramp up (lightBlue-blue)
    do
    {
      color -= 1 << 8; // decrement green.
      if ((color >> 16) * 5 > i)
      {
        // every five, decrement red as well (since that's the ratio of 0x17 to 0x72)
        color -= 1 << 16;
      }
      g_leds.setAllPixels(color);
    delay(1); // minimum delay on start, since this is just ramping the leds up.
    } while (--i > 0);
  
    // blue-green
    do
    {
      color = (uint32_t)(255 - i) | (uint32_t)i << 8; // cycle blue - green.
      g_leds.setAllPixels(color);
      delay(wait);
    } while (++i > 0);
  
    // green-red
    do
    {
      color = (uint32_t)(255 - i) << 8 | (uint32_t)i << 16; // cycle green - red.
      g_leds.setAllPixels(color);
      delay(wait);
    } while (++i > 0);
  
    // ramp down (red)
    do
    {
      color = 255 - (uint32_t)i << 16; // fade down blue.
      g_leds.setAllPixels(color);
    } while (++i > 0);
  }

private:
  uint8_t m_litLeds[NUM_ROWS];
  
} g_lightsOut;

#define MAX_SIMON_SEQUENCE 16

class Simon
{
public:
  Simon()
  {
    
  }
  void setup()
  {
    randomSeed(micros());
    g_buttons.setNumStates(2);

    // make all the colors dance
    lightDance(500);

    delay(1000);

    // clear any cruft
    resetSequence();
    
    // now start the sequence
    advanceAndShowSequence();
  }

  void showCurrentState()
  {
    uint8_t positionsToShowPressed = 0;
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      for (uint8_t col = 0; col < NUM_COLS; col++)
      {
        uint8_t currPos = posFromRowCol(row,col);
        if (g_buttons.isButtonPressed(row,col))
        {
          bitSet(positionsToShowPressed, currPos);
        }
      }
    }
    updatePressedStates(positionsToShowPressed);
  }

  void loop()
  {
    bool bButtonUpDetected = false, bPressIsCorrect;
    uint8_t positionsToShowPressed = 0;
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      for (uint8_t col = 0; col < NUM_COLS; col++)
      {
        uint8_t currPos = posFromRowCol(row,col);
        if (g_buttons.isButtonPressed(row,col))
        {
          bitSet(positionsToShowPressed, currPos);
        }
        else if (!bButtonUpDetected && g_buttons.isButtonOn(row,col))
        {
          // Let's break on the first touch-up detected
          bButtonUpDetected = true;
          bPressIsCorrect = checkAndAdvance(currPos);
        }
      }
    }

    updatePressedStates(positionsToShowPressed);

    if (bButtonUpDetected)
    {
      if (bPressIsCorrect)
      {
        if (sequenceComplete())
        {
          if (sequenceReachedMaxLength())
          {
//            showWin();
            lightDance(300);
            delay(1000);
        
            // clear any cruft
            resetSequence();
          }
          else
          {
            delay(200);
          }
          
          // now show the next item in the sequence
          advanceAndShowSequence();
        }
      }
      else
      {
        // it feels wierd to not see a silent board before the
        // sad trombone
        delay(200);
        // show sad trombone
        g_leds.setAllPixels(RED);
        delay(500);
        // give them another try?
        resetAttempt();
        // show them the correct sequence again.
        showCurrentSequence();
      }
      
      // we've handled these buttons, so clear them.
      g_buttons.reset();
    }
  }

private:

  void lightDance(uint16_t wait)
  {
    // first, circle
    updatePressedStates(0x1);
    delay(wait);
    updatePressedStates(0x2);
    delay(wait);
    updatePressedStates(0x8);
    delay(wait);
    updatePressedStates(0x4);
    delay(wait);
    // then alternate
    for (uint8_t i = 0 ; i < 2 ; ++i)
    {
      updatePressedStates(0x9);
      delay(wait);
      updatePressedStates(0x6);
      delay(wait);
    }
    // then flash
    for (uint8_t i = 0 ; i < 2 ; ++i)
    {
      updatePressedStates(0xF);
      delay(wait);
      updatePressedStates(0x0);
      delay(wait/2); // wait less time for the dim
    }

    // leave it dim
  }

  void resetSequence()
  {
    m_nCurrentSequenceLength = 0;
  }

  void resetAttempt()
  {
    m_nCurrentSequencePosition = 0;
  }

  void updatePressedStates(uint8_t pressedMap)
  {
    g_leds.beginUpdate();
    for (uint8_t row = 0 ; row < NUM_ROWS ; ++row)
    {
      for (uint8_t col = 0 ; col < NUM_COLS ; ++col)
      {
        uint8_t currPos = posFromRowCol(row,col);
        uint32_t color = bitRead(pressedMap, currPos) ? ms_colorsBright[currPos] : ms_colors[currPos];
        g_leds.setPixelColor(row, col, color);
      }
    }
    g_leds.endUpdate();
  }
  
  void advanceAndShowSequence()
  {
    m_nCurrentSequencePosition = 0;
    m_sequence[m_nCurrentSequenceLength++] = random(4);
    showCurrentSequence();
  }

  void showCurrentSequence()
  {
    for (uint8_t i = 0 ; i < m_nCurrentSequenceLength ; ++i)
    {
      uint8_t pos = (1 << m_sequence[i]);
      updatePressedStates(pos);
      delay(500);
      setAllDim();
      delay(200);
    }
  }

  bool checkAndAdvance(uint8_t currPos)
  {
    bool bRet = false;
    if (m_nCurrentSequencePosition < m_nCurrentSequenceLength)
    {
      bRet = m_sequence[m_nCurrentSequencePosition++] == currPos;
    }
    return bRet;
  }
  bool sequenceComplete() { return m_nCurrentSequencePosition == m_nCurrentSequenceLength; }
  bool sequenceReachedMaxLength() { return m_nCurrentSequenceLength == MAX_SIMON_SEQUENCE; }
  
  uint8_t posFromRowCol(uint8_t row, uint8_t col)
  {
    return row & 0x2 | col >> 1;
  }
  void setAllDim()
  {
    updatePressedStates(0x0);
  }
private:
  uint8_t m_sequence[MAX_SIMON_SEQUENCE];
  uint8_t m_nCurrentSequenceLength, m_nCurrentSequencePosition;
  static const uint32_t ms_colors[];
  static const uint32_t ms_colorsBright[];
} g_simon;

const uint32_t Simon::ms_colors[] = {RED_DIM, BLUE_DIM, YELLOW_DIM, GREEN_DIM};
const uint32_t Simon::ms_colorsBright[] = {RED, BLUE, YELLOW, GREEN};

enum Mode
{
  MODE_SIMON_SAYS,
  MODE_BOARD_TEST,
  MODE_DRAWING,
  MODE_LIGHTS_OUT,
  NUM_MODES
};

uint8_t g_mode;
uint32_t lastChange = 0;

void loop()
{
//  // light LED as we start reading data
//  digitalWrite(ledPin, HIGH);

  bool bButtonsChanged = g_buttons.updateButtonState();

//  // turn off LED - done reading data
//  digitalWrite(ledPin, LOW);

  if (bButtonsChanged)
  {
    lastChange = millis();
    switch(g_mode)
    {
    case MODE_BOARD_TEST:
      g_boardTest.loop();
      break;
    case MODE_DRAWING:
      g_drawing.loop();
      break;
    case MODE_SIMON_SAYS:
      g_simon.loop();
      break;
    case MODE_LIGHTS_OUT:
      g_lightsOut.loop();
      break;
    default:
      // shouldn't get here, but should do something reasonable if we do
      g_mode = MODE_BOARD_TEST;
      g_boardTest.loop();
      break;
    }
  } else if (g_buttons.isButtonPressed(0,0)) {
    int32_t now = millis();
    // If someone long-pressed the 0 button, switch to a new game
    if (lastChange && ((now - lastChange) > LONG_PRESS_MS)) {
      // set lastChange to zero to prevent another button down detection
      lastChange = 0;
      if (++g_mode >= NUM_MODES)
      {
        g_mode = 0; // using 0 instead of the name, since this is easier if the order changes
      }

      modeSpecificSetup();
    }
  }
}

void modeSpecificSetup()
{
  switch(g_mode)
  {
  case MODE_BOARD_TEST:
    g_boardTest.setup();
    break;
  case MODE_DRAWING:
    g_drawing.setup();
    break;
  case MODE_SIMON_SAYS:
    g_simon.setup();
    break;
  case MODE_LIGHTS_OUT:
    g_lightsOut.setup();
    break;
  default:
    // shouldn't get here, but should do something reasonable if we do
    g_mode = MODE_BOARD_TEST;
    g_boardTest.setup();
    break;
  }
}

//
//uint32_t Wheel(byte WheelPos) {
//  if(WheelPos < 85) {
//   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
//  } else if(WheelPos < 170) {
//   WheelPos -= 85;
//   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
//  } else {
//   WheelPos -= 170;
//   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
//  }
//}

void rainbow(uint8_t wait) {
  uint8_t i = 0;

  // ramp up (green)
  do
  {
    uint32_t color = (uint32_t)i << 8; // start up green.
    g_leds.setAllPixels(color);
//    delay(1); // minimum delay on start, since this is just ramping the leds up.
  } while (++i > 0);

  // green-red
  do
  {
    uint32_t color = (uint32_t)(255 - i) << 8 | (uint32_t)i << 16; // cycle green - red.
    g_leds.setAllPixels(color);
    delay(wait);
  } while (++i > 0);

  // red-blue
  do
  {
    uint32_t color = (uint32_t)(255 - i) << 16 | (uint32_t)i; // cycle red - blue.
    g_leds.setAllPixels(color);
    delay(wait);
  } while (++i > 0);

  // blue-green
  do
  {
    uint32_t color = (uint32_t)(255 - i) | (uint32_t)i << 8; // cycle blue - green.
    g_leds.setAllPixels(color);
    delay(wait);
  } while (++i > 0);

  // ramp down (green)
  do
  {
    uint32_t color = 255 - (uint32_t)i << 8; // fade up green.
    g_leds.setAllPixels(color);
//    delay(1); // minimum delay on start, since this is just ramping the leds up.
  } while (++i > 0);
}


