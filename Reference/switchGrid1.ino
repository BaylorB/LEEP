



//---  Reads the state of a switch array
//
//  > Wire an LED from Digital Pin 4 (pin 6 on the chip) thru a 330 ohm resistor to ground
//  > Wire 16 switches as per diagram - 4x4 grid, switches across columns and rows
//  > columns will be Digital Pins 5-8, Rows use Digital Pins 9-12  


int ledPin = 4;   

int rows = 4;            // how many rows and columns - we can use 20 pins on an Arduino
int columns = 4;         // 4 rows and 4 columns which is 16 switches total
int rowOffset = 9;       // digital pins not starting at 0 - columns are Digital Pins 9-12 (IC Pins 15-18)
int columnOffset = 5;    // digital pins not starting at 0 - rows are Digital Pins 5-8 (IC Pins 11-14)

// 2 dimensional array to hold switch data
int switchData[4][4] = {
  { 
    0, 0, 0, 0       }
  ,
  { 
    0, 0, 0, 0       }
  ,
  { 
    0, 0, 0, 0       }
  ,
  { 
    0, 0, 0, 0       }
};

int buttonState = 0;        // variable for storing "state" of the button


void setup() {                   // do once at startup
  Serial.begin(9600);         // initialize serial port
  
  // using 2 FOR loops to set states of pins rather than listing them separately
  for(int i=0; i<columns; i++) {   // set Digital Pins 9-12 as OUTPUTS
    pinMode(i+columnOffset, OUTPUT);
    Serial.print("digital pin ");
    Serial.print(i+columnOffset);
    Serial.println(" set as output");
  }
  Serial.println();

  for(int i=0; i<rows; i++) {      // set Digital Pins 5-8 (IC Pins 11-14) as INPUTS pulled high
    pinMode(i+rowOffset, INPUT_PULLUP);
    pinMode(i+columnOffset, OUTPUT);
    Serial.print("digital pin ");
    Serial.print(i+rowOffset);
    Serial.println(" set as input");  
  }
  Serial.println();
  Serial.println();
  
  pinMode(ledPin, OUTPUT);    // initialize digital pin as an output (for LED)
}

void loop()  {                  // repeat forever
  // all columns (outputs) are set high to start
  for(int i=0; i<columns; i++) {
    digitalWrite(i+columnOffset, HIGH);
  }
  digitalWrite(ledPin, HIGH);  // light LED as we start reading data

  // scan across columns - make each column go low, one at a time
  for(int i=0; i<columns; i++) {
    digitalWrite(i+columnOffset, LOW);
    // scan across rows testing switches and putting data in array
    for(int j=0; j<rows; j++) {
      switchData[i][j] = digitalRead(j+rowOffset); // "Read" the voltage present on the pin (check switch)
      delay(20);  
    }  // end row FOR loop
    digitalWrite(i+columnOffset, HIGH);  // raise column back HIGH
  }  // end column FOR loop
        
  digitalWrite(ledPin, LOW);  // turn off LED - done reading data    
  dataDump();
  delay(1000);           // use to test loop speed    
}

void dataDump() {
  // scan across columns
  for(int i=0; i<columns; i++) {
    // scan across rows
    for(int j=0; j<rows; j++) {
      Serial.print("column ");
      Serial.print(i);
      Serial.print(" row ");
      Serial.print(j);
      Serial.print(" switch ");
      Serial.println(switchData[i][j]);
    }  // end row FOR loop
  }  // end column FOR loop
      Serial.println();
}


void dataDump2() {
  for(int i=0; i<columns; i++) {  
    for(int j=0; j<rows; j++) {
      Serial.print(switchData[i][j] + "  x ");
    }
    Serial.println();
    delay(250);
  }
  Serial.println();     
}

