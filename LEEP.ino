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
#define GREEN           0x00FF00
#define BLUE            0x0000FF
#define CYAN            0x00FFFF
#define MAGENTA         0xFF00FF
#define YELLOW          0xFFFF00

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
      // if any lights are on
      if (m_litLeds[row])
      {
        // just return
        return;
      }
    }

    // If we get here, all the lights are out, so celebrate
    delay(500);
    lightDance();
    lightDance();
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
  

private:
  uint8_t m_litLeds[NUM_ROWS];
  
} g_lightsOut;
//
//class BoardTest
//{
//  void setup()
//  {
//    g_buttons.setNumStates(2);
//  }
//
//  // Basic board test -- lights go on when the button is pressed and go off
//  // when they are pressed again.
//  void loop()
//  {
//    g_leds.beginUpdate();
//    // Update our lights.
//    for (uint8_t row = 0; row < NUM_ROWS; row++)
//    {
//      for (uint8_t col = 0; col < NUM_COLS; col++)
//      {
//        uint32_t color = COLOR_NONE;
//        
//        if (g_buttons.isButtonPressed(row,col))
//        {
//          color = MAGENTA;
//        }
//        else if (g_buttons.isButtonOn(row,col))
//        {
//          color = BLUE;
//        }
//  
//        g_leds.setPixelColor(row, col, color);
//      }
//    }
//    g_leds.endUpdate();
//  }
//} g_boardTest;
//
enum Mode
{
  MODE_BOARD_TEST,
  MODE_DRAWING,
  MODE_SIMON_SAYS,
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
      ++g_mode;
      // fall through -- NYI
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

      switch(g_mode)
      {
      case MODE_BOARD_TEST:
        g_boardTest.setup();
        break;
      case MODE_DRAWING:
        g_drawing.setup();
        break;
      case MODE_SIMON_SAYS:
        ++g_mode;
        // fall through -- NYI
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


