/*
 * NOTE: The following is just the Summary section for documentation purpose
 * and may be updated accordingly to the code modification.
 * 
 * * Mapping Summary: * 
 * Activity   | Code (from App)   | Indicator Pin
 * Wellbeing  |     0             |     5
 * TimeMgt    |     1             |     6
 * Budget     |     2             |     7
 * 
 * NOTE:  The colors attachment to the LEDs pins is flexible.
 * E.g GREEN lED can be attached to pin 5 to represent Wellbeing or PURPLE LED to pin 6 to mean TimeMgt
 * 
 * Other Pins
 * Device           | Pin
 * Pulse Sensor     | 0
 * Pulse Indicator  | 13
 */
/** End of Summary!**/

/** The main code starts below!**/
 
// Pins definitions
// Activities Indicators
#define WELLBEING_LED 5
#define TIME_MGT_LED 6
#define BUDGET_LED 7
// Pulse Sensor purple wire connected to analog pin 0
#define PULSE_SENSOR 0    
// Pulsating LED Indicator             
#define PULSE_INDICATOR 13

// Incoming serial data
int activityCode = 0;
enum class Activity { Wellbeing, TimeMgt, Budget };
Activity state = Activity::Wellbeing;

// Normal BPM after which alarm is raised
const int NORMAL_BPM = 90;    // Set normal BPM
boolean LED_PULSATING = true;   // Should LED resonate with heartbeats or simulate deep breaths
const int DEEP_BREATH_RATE = 8;    // Number of deep breaths per minutes

// Volatile Variables, used in the interrupt service routine!
volatile int BPM;                   // int that holds raw Analog in 0. updated every 2mS
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // int that holds the time interval between beats! Must be seeded!
volatile boolean Pulse = false;     // "True" when User's live heartbeat is detected. "False" when not a "live beat".
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

unsigned long timer; // the timer

void setup(){
  pinMode(PULSE_INDICATOR, OUTPUT);         // pin that will blink to your heartbeat!
  Serial.begin(115200);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS
   // IF YOU ARE POWERING The Pulse Sensor AT VOLTAGE LESS THAN THE BOARD VOLTAGE,
   // UN-COMMENT THE NEXT LINE AND APPLY THAT VOLTAGE TO THE A-REF PIN
  //   analogReference(EXTERNAL);

  timer = millis();   // Start the timer
}


//  Where the Magic Happens
void loop(){

  // If Activity input from serial
  if(Serial.available()){
    // Process incoming data
    decodeInput();

    // Activate activity state
    activateState();
  }
  
  
  if(LED_PULSATING){
    // Pulsate at the pace of BPM
    pulsate(Pulse);
  }
  else{
    // Take deep breath
    deepBreath(DEEP_BREATH_RATE);
  }
  
  if (QS){
    // A Heartbeat Was Found, BPM and IBI have been Determined
    // Quantified Self "QS" true when arduino finds a heartbeat
    // TODO: action here...

    // Send BPM to Serial
    Serial.println(BPM);
    
    // If BPM is below threshold, LED pulsates with heartbeats 
    if(BPM < NORMAL_BPM){
      LED_PULSATING = true;
    }
    // Otherwise, stop resonating with heartbeats, instead take deep breaths
    else{
      LED_PULSATING = false;
    }

    // Reset the Quantified Self flag for next time
    QS = false;
  }

  delay(20);  //  delay
}

void pulsate(boolean pulse){
  if(pulse){
      digitalWrite(PULSE_INDICATOR, HIGH); 
    }
    else{
      digitalWrite(PULSE_INDICATOR, LOW);
    }
}

void deepBreath(int rate){
  // Pulsate at 8 breaths per minute (8 breaths per 60 secs)
  // 60/8 = 7.5 sec per breath
  int interval = (60 * 100) / rate;
  
  if((millis() - timer) > interval){
    digitalWrite(PULSE_INDICATOR, HIGH);
    delay(2000);  // ON (Breath) for 2 secs, then OFF
    digitalWrite(PULSE_INDICATOR, LOW);

    // Reset timer
    timer = millis();
  }
}

void activateState(){
  // First turn OFF all LEDs
  digitalWrite(WELLBEING_LED, LOW);
  digitalWrite(TIME_MGT_LED, LOW);
  digitalWrite(BUDGET_LED, LOW);
  
  switch(state){
    case Activity::Wellbeing:
      digitalWrite(WELLBEING_LED, HIGH);
      break;
    case Activity::TimeMgt:
      digitalWrite(TIME_MGT_LED, HIGH);
      break;
    case Activity::Budget:
      digitalWrite(BUDGET_LED, HIGH);
      break;
    default:
      break;
  }
}

void decodeInput(){
  // Read the activity code
    activityCode = Serial.read();

    // Decode the input
    switch(activityCode){
        case '0':
          state = Activity::Wellbeing;
          break;
        case '1':
          state = Activity::TimeMgt;
          break;
        case '2':
          state = Activity::Budget;
          break;
        default:
          Serial.println("Invalid activity code!");
          break;
    }
}

