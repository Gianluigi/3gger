/* vi:ts=2 sw=2: 
** 
**  Author: Gianluigi Rubino
**  http://www.grsoft.org
*/

// hard coded constants
#define DEFAULT_TIMEOUT 1000
#define MAX_STIMULI     100

// Error messages
#define ERR_NOT_READY           "400 Error: Not in ready status, do programming first !"
#define ERR_PROGRAMMING_ABORT   "401 Error: Programming aborted !"
#define ERR_MODE_UNKNOWN        "402 Error: Unknow mode, swithcing to WAITING!"
#define ERR_STIMULATION_ABORT   "403 Error: Stimulation aborted !"
#define ERR_TOO_MANY_STIM       "404 Error: too many stimuli defined !"

// Info messages
#define PROGRAMMING_OK          "201 Programming OK, Ready to stimulate"
#define WELCOME_MESSAGE         "200 rtstim v0.1 - ready"

//#define _DEBUG
#ifdef _DEBUG
  #define DEBUG_PRINT_MSG(m) Serial.println(m)
  #define DEBUG_PRINT(m, c) {Serial.print(m); Serial.println(c);}
#else
  #define DEBUG_PRINT_MSG(m) {}
  #define DEBUG_PRINT(m, c) {}
#endif  
/*
** Simple protocol
** - PROGRAM: start the definition of stimuli
** - STIM: start the real stimulation
** - STOP: stop the current stimulation, set mode to WAITING
*/

enum commands_t { PROGRAM = 'P', STIM = 'S', STOP = 'A', OK = 'O', CODE = 'C', ITI = 'I', VER = 'V' };

typedef struct {
  char code_low;    //which pin to write to PORTB 
  char code_high;    //which pin to write to PORTD
  int iti;     //inter trial interval
} stimulus_t;

// constants won't change. Used here to 
// set pin numbers:
const int ledPin =  2;      // the number of the LED pin
const int pushPin =  3;      // the number of the push button pin
const int num_ports = 8;


//WAITING: waiting for programming stimuli
//PROGRAMMING: the definition of the stimuli is in progress
//READY: everything ready for the stimulation
//STIMULATING: the stimulation is being fired
typedef enum { WAITING, READY, STIMULATING, PROGRAMMING, NONE=-1} modes_t;
int mode = WAITING;

// Variables will change:
int ledState = LOW;             // ledState used to set the LED
long previousMillis = 0;        // will store last time LED was updated

int num_stimuli = 0;
stimulus_t stimuli[100];
long trigger_len = 500; //us not ms
long repetition_count = 0;
long repetition_time = 0;

void setup() {
  //ensure mode at startup is WAITING for commands
  mode = WAITING;
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);      
  digitalWrite(ledPin, HIGH);
  //set the ports used for trigering to OUTPUT mode
  DDRB = B00111111;
  DDRD = DDRD | B11000000;
  Serial.begin(9600);
  Serial.println(WELCOME_MESSAGE);
}

//
// Serial utility functions
//
int read_command()
{
  if (Serial.available() > 0)
  {
     int cmd = Serial.read();
     //clear newline char
     Serial.flush();
     return cmd;
  }
  return NONE;
}
//
// Wait for the Serial 
bool wait_serial_timeout(int timeout)
{
  int count = 0;
  while (Serial.available() <= 0 && count <= timeout) {
    delay(100);
    count += 100;
  }
  return count <= timeout ? true : false;
}
//WARNING: non-reentrant code !
char *read_serial_string()
{
  bool stop = false;
  static char buf[128];
  int i = 0;
  long timeout = millis();

  buf[sizeof(buf)-1] = 0;
  while (!stop && i < sizeof(buf)-1)
  {
    if (Serial.available() > 0)
    {
      buf[i] = Serial.read();
      if (buf[i] == '\n' || buf[i] == '.') 
      {
        buf[i] = 0;
        stop = true;
      }
      i++;
    }
    else
    {
      if (timeout + DEFAULT_TIMEOUT < millis())
      {
        stop = true;
        Serial.flush();
        buf[0] = 0;
      }
    }
  }
  return buf;
}
bool read_serial_int(int *out)
{
  char *str = read_serial_string();
  if (str[0] == 0) //empty string
    return false;
  else
  {
    *out = atoi(str);
    return true;
  }
}

bool read_serial_long(long *out)
{
  char *str = read_serial_string();
  if (str[0] == 0) //empty string
    return false;
  else
  {
    *out = atol(str);
    return true;
  }
}

int check_command_mode(int previous)
{
  int cmd = read_command();
  if (cmd != NONE)
  {
    //set the device to programming mode
    if (cmd == PROGRAM) return PROGRAMMING;
    else if (cmd == STIM && mode == READY) 
      return STIMULATING;
    else if (cmd == VER)
      Serial.println(WELCOME_MESSAGE);
    else if (cmd == STIM && mode != READY) 
      Serial.println(ERR_NOT_READY);
  }
  return previous;
}

int do_programming()
{
  Serial.println("200 Start programming mode");
  if (wait_serial_timeout(DEFAULT_TIMEOUT))
  {
    if (!read_serial_int(&num_stimuli)) return WAITING;
    DEBUG_PRINT("200 Total stimuli to program: ", num_stimuli);

    if (num_stimuli > MAX_STIMULI)
    {
      num_stimuli = 0;
      Serial.println(ERR_TOO_MANY_STIM);
      Serial.flush();
      return WAITING;
    }
    for (int i=0; i<num_stimuli; i++)
    {
      int tmp = 0;

      DEBUG_PRINT("200 Please Input code for stim #", i+1);
      if (!read_serial_int(&tmp)) return WAITING;
      stimuli[i].code_low = char(tmp & B00111111);
      stimuli[i].code_high = char(tmp & B11000000);
      
      DEBUG_PRINT("Code: ", tmp);
      DEBUG_PRINT("200 Please Input length for stim #", i+1);
      if (!read_serial_int(&tmp)) return WAITING;
      stimuli[i].iti = tmp;
      DEBUG_PRINT("ITI: ", tmp);
    }
    DEBUG_PRINT_MSG("200 Please enter Num of repetitions:"); 
    if (!read_serial_long(&repetition_count)) return WAITING;
    DEBUG_PRINT_MSG("200 Please enter time of repetitions:"); 
    if (!read_serial_long(&repetition_time)) return WAITING;

    Serial.println("200 Programmin finished.");

    return READY;
  }
}


//
// the critical part !!
//
int do_stimulation()
{
  for (int r=0; r<repetition_count; r++)
  {
    long target = micros() + repetition_time*100;
    fire_stimulus();
    while (target > micros() && repetition_count > 1)
    {
      if (Serial.available() > 0)
      {
        if (Serial.read() == 'A')
        {
          Serial.println(ERR_STIMULATION_ABORT);
          return READY;
        }
      }
    }
  }
  Serial.println("\n200 Stimulation Finished. Ready");
  return READY;
}

void fire_stimulus()
{
  digitalWrite(ledPin, LOW);
  for (int i=0; i<num_stimuli; i++)
  {
    //TODO: handle overflow of micros() function
    long start = micros(); 
    PORTB = PORTB | stimuli[i].code_low;
    PORTD = PORTD | stimuli[i].code_high;
    long target = start + trigger_len;
    
    while (target > micros())
      /* WAIT TRIGGER LENGTH */;

    //set the port to LOW
    PORTB = PORTB & B11000000;
    PORTD = PORTD & B00111111;
    
    DEBUG_PRINT("Stimulus fired: ", i+1);
    target = start + long(stimuli[i].iti) * 100;

    DEBUG_PRINT("Waiting for :", target);
    DEBUG_PRINT("Now is: ", micros());
    while (target > micros())
      /* WAIT ITI TIME */;
      
  }
  digitalWrite(ledPin, HIGH);
  Serial.print("*");

}

void loop()
{
  switch (mode)
  {
    case WAITING:
    case READY:
      mode = check_command_mode(mode);
      break;
    case PROGRAMMING:
      mode = do_programming();
      if (mode == WAITING) 
        Serial.println(ERR_PROGRAMMING_ABORT);
      else if (mode == READY) 
        Serial.println(PROGRAMMING_OK);
      break;

    case STIMULATING:
      mode = do_stimulation();
      if (mode == WAITING) 
        Serial.println(PROGRAMMING_OK);

      break;
    default:
      Serial.println(ERR_MODE_UNKNOWN);
      mode = WAITING;
  };
}


