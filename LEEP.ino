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

class Game
{
public:
  virtual ~Game()
  {/*empty destructor*/}

  // All games need to setup the button controller
  // and initialize the LEDs. I suppose I could write
  // a super simple one here, but I don't think that
  // any two games yet have the same setup flow, so
  // not really worth it.
  virtual void setup() = 0;
  // It's not a game unless you can interact with it,
  // so we should do something on user input.
  // Note that while this is running, inputs are ignored.
  // This can be quite useful and simplify interactions
  // but it's worth bearing in mind, in case you want your
  // interaction response to be interruptable
  virtual void inputsChanged() = 0;
  // loop is where we do things that happen in reponse to no user
  // stimulous. Primarily, I expect that this will be where you
  // trigger interactions in response to timers and the like.
  // For now, there is no external timer interface, so you have
  // to manage this yourself. In a more fully baked implementation,
  // I could imagine that this would be a service of the 'OS'.
  virtual void loop()
  {
    // it's fairly likely that we only want to do something on
    // input changes, so we can create a default implementation
    // of loop that is empty.
  }
};

// Basic board test -- lights go on when the button is pressed and go off
// when they are pressed again.
class BoardTest : public Game
{
public:
  void setup()
  {
    g_buttons.setNumStates(2);
    g_leds.setAllPixels(COLOR_NONE);
  }

  void inputsChanged()
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

class Drawing : public Game
{
public:
  void setup();

  void inputsChanged()
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

#define SNAKE_LENGTH          4
#define SNAKE_TRAVEL          16
#define SNAKE_TRAVEL_TIME_MS  4000
#define TIME_TILL_HINT_MS     15000
#define SNAKE_MOVE_DELAY_MS   (SNAKE_TRAVEL_TIME_MS/SNAKE_TRAVEL)

#define SNAKE_PEEK_COLOR      LIGHT_GREEN
#define SNAKE_COLOR           LIGHT_GREEN
#define SNAKE_WRONG_COLOR     YELLOW

class Snake : public Game
{
public:
  void setup()
  {
    randomSeed(micros());
    g_buttons.setNumStates(2);
    g_leds.setAllPixels(COLOR_NONE);
    m_nSnakePos = random(16);
    int_blinkSnakeHole(3, 500);
    int_runSnake();
    m_nLastTouched = millis();
  }

  void inputsChanged()
  {
    g_leds.beginUpdate();
    // Update our lights.
    const uint8_t target_row = int_rowForPos(m_nSnakePos);
    const uint8_t target_col = int_colForPos(m_nSnakePos);
    bool buttonOn = false;
    for (uint8_t row = 0; row < NUM_ROWS; row++)
    {
      for (uint8_t col = 0; col < NUM_COLS; col++)
      {
        uint32_t color = COLOR_NONE;
        if (g_buttons.isButtonPressed(row,col))
        {
          color = (row == target_row && col == target_col) ? SNAKE_PEEK_COLOR : SNAKE_WRONG_COLOR;
        }
        else if (g_buttons.isButtonOn(row, col))
        {
          buttonOn = true;
        }
        g_leds.setPixelColor(row, col, color);
      }
    }
    g_leds.endUpdate();

    // If we detect a button up.
    if (buttonOn)
    {
      // If the correct button is held down, don't do anything
      if (!g_buttons.isButtonPressed(target_row, target_col))
      {
        delay(150);
        // if the right hole is chosen, run the snake
        if (g_buttons.isButtonOn(target_row, target_col))
        {
          int_runSnake();
        }
        else
        {
          int_blinkSnakeHole(2,200); // show a hint if they're having problems.
        }
        g_buttons.reset();
      }
    }
    m_nLastTouched = millis();
  }

  void loop()
  {
    if (millis() - m_nLastTouched > TIME_TILL_HINT_MS)
    {
      int_blinkSnakeHole(2,100); // show a hint if they're having problems.
      m_nLastTouched = millis();
    }
  }

private:
  
  void int_blinkSnakeHole(uint8_t nBlinkCycles, uint16_t nBlinkDuration)
  {
    const uint8_t row = m_nSnakePos >> 2;
    const uint8_t col = m_nSnakePos & 0x03;
    for (uint8_t i = 0 ; i < nBlinkCycles ; ++i)
    {
      g_leds.beginUpdate();
      g_leds.setPixelColor(row, col, SNAKE_PEEK_COLOR);
      g_leds.endUpdate();
      delay(nBlinkDuration);
      g_leds.beginUpdate();
      g_leds.setPixelColor(row, col, COLOR_NONE);
      g_leds.endUpdate();
      delay(nBlinkDuration);
    }
  }

  enum SnakeDirection
  {
    NORTH,
    EAST,
    SOUTH,
    WEST
  };

  uint8_t int_rowForPos(uint8_t pos)
  {
      return pos / NUM_COLS;
  }
  uint8_t int_colForPos(uint8_t pos)
  {
      return pos % NUM_COLS;
  }
  void int_runSnake()
  {
    uint8_t anSnakeSegments[SNAKE_LENGTH];
    uint8_t nSnakeDirection = random(4); // start in a random direction
    uint8_t directionOptionsMap = 0x0F;
    uint8_t nWritePos = 0;
    const uint16_t wait = (SNAKE_TRAVEL_TIME_MS / SNAKE_TRAVEL);
    for (uint8_t i = 0 ; i < SNAKE_TRAVEL; ++i)
    {
      const uint8_t row = int_rowForPos(m_nSnakePos);
      const uint8_t col = int_colForPos(m_nSnakePos);

      g_leds.beginUpdate();
      // if we're eating our tail
      if (i >= SNAKE_LENGTH)
      {
        const uint8_t old_row = int_rowForPos(anSnakeSegments[nWritePos]);
        const uint8_t old_col = int_colForPos(anSnakeSegments[nWritePos]);
        g_leds.setPixelColor(old_row, old_col, COLOR_NONE);
      }
      anSnakeSegments[nWritePos] = m_nSnakePos;
      g_leds.setPixelColor(row, col, SNAKE_COLOR);
      g_leds.endUpdate();

      delay(wait);

      // on all but the last pass, figure out the next position
      if (i < SNAKE_TRAVEL - 1)
      {
        // collision detection:
        // first, populate our list of posibilities.
        uint8_t directionOptionsMap = 0x0F;
        // now eliminate the impossible
        if (row == 0)
        {
          // at the top, can't go north
          bitClear(directionOptionsMap, NORTH);
        }
        else if (row == NUM_ROWS - 1)
        {
          // at the bottom, can't go south
          bitClear(directionOptionsMap, SOUTH);
        }
  
        if (col == 0)
        {
          // at the left edge, can't go west
          bitClear(directionOptionsMap, WEST);
        }
        else if (col == NUM_COLS - 1)
        {
          // at the right edge, can't go east
          bitClear(directionOptionsMap, EAST);
        }
  
        const uint8_t nReadPos = i >= SNAKE_LENGTH ? (nWritePos + 1) % SNAKE_LENGTH : 0;
        // now eliminate any directions that crash into the existing snake
        uint8_t nNumPossibleDirections = 0;
        for (uint8_t dir = 0 ; dir < 4 ; ++dir)
        {
          // if the direction is possible
          if (bitRead(directionOptionsMap,dir))
          {
            ++nNumPossibleDirections;
            uint8_t pos = int_posAdjustedByDirection(m_nSnakePos, (SnakeDirection)dir);
  
            // see if this pos is in the list
            for (uint8_t check = nReadPos ; check != nWritePos ; )
            {
              if (anSnakeSegments[check] == pos)
              {
                bitClear(directionOptionsMap, dir);
                // just kidding
                --nNumPossibleDirections;
                break;
              }
              ++check;
              check %= SNAKE_LENGTH;
            }
          }
        }
        
        if (nNumPossibleDirections > 0)
        {
          uint8_t choice = random(nNumPossibleDirections);
          for (uint8_t dir = 0 ; dir < 4 ; ++dir)
          {
            // if the direction is possible
            if (bitRead(directionOptionsMap,dir))
            {
              if (choice-- == 0)
              {
                m_nSnakePos = int_posAdjustedByDirection(m_nSnakePos, (SnakeDirection)dir);
                break;
              }
            }
          }
        }
        else
        {
          // if we've painted ourselves into a corner, then just don't move.
          // This isn't possible with our starting values, but could be with a longer snake.
        }
      }
      // wait to increment this until we've done the check to make the math easier.
      ++nWritePos;
      nWritePos %= SNAKE_LENGTH;
    }

    // REVIEW: right now, this will only support snake lengths shorter than path lengths
    // If this is ever not the case, we need to add a delay loop here to eat some time,
    // followed by a slight adjustment to the final loop.

    // continue eating our tail
    for (uint8_t i = 0 ; i < SNAKE_LENGTH ; ++i)
    {
      g_leds.beginUpdate();
      const uint8_t old_row = int_rowForPos(anSnakeSegments[nWritePos]);
      const uint8_t old_col = int_colForPos(anSnakeSegments[nWritePos]);
      g_leds.setPixelColor(old_row, old_col, COLOR_NONE);
      g_leds.endUpdate();

      delay(wait);

      ++nWritePos;
      nWritePos %= SNAKE_LENGTH;
    }
  }

  static uint8_t int_posAdjustedByDirection(uint8_t pos, SnakeDirection dir)
  {
    switch (dir)
    {
    case NORTH:
      pos -= NUM_COLS;
      break;
    case EAST:
      pos += 1;
      break;
    case SOUTH:
      pos += NUM_COLS;
      break;
    case WEST:
      pos -= 1;
      break;
    }
    return pos;
  }
private:
  uint8_t m_nSnakePos;
  uint32_t m_nLastTouched;
  
} g_snake;

#define bitToggle(value, bit_no) (bitWrite(value, bit_no, !bitRead(value, bit_no)))

class LightsOut : public Game
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

  void inputsChanged()
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

class Simon : public Game
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

  void inputsChanged()
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
        // it feels weird to not see a silent board before the
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
  MODE_BOARD_TEST,
  MODE_SNAKE,
  MODE_DRAWING,
  MODE_LIGHTS_OUT,
  MODE_SIMON_SAYS,
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
    currentGame().inputsChanged();
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
  // always call the game's loop last, and call it regardless of whether
  // we have just told them to setup or that the inputs have changed.
  currentGame().loop();
}

class Game &currentGame()
{
  switch(g_mode)
  {
  case MODE_BOARD_TEST:
    return g_boardTest;
  case MODE_DRAWING:
    return g_drawing;
  case MODE_SNAKE:
    return g_snake;
  case MODE_SIMON_SAYS:
    return g_simon;
  case MODE_LIGHTS_OUT:
    return g_lightsOut;
  }
  // Create a defacto 'default' but leave it out of the switch,
  // so that the compiler can warn us when we add a new enum
  // value. I don't know if that's something that this compiler
  // does, but it could :)

  // shouldn't get here, but should do something reasonable if we do
  g_mode = MODE_BOARD_TEST;
  return g_boardTest;
}

void modeSpecificSetup()
{
  currentGame().setup();
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


