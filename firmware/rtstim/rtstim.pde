/* 
** 
**  Author: Gianluigi Rubino
**  http://www.grsoft.org
*/

//define version and welcome message here:
#define WELCOME_MESSAGE "rtstim v0.1 - ready"
#define DEFAULT_TIMEOUT 10000
/*
** Simple protocol
** - PROGRAM: start the definition of stimuli
** - STIM: start the real stimulation
** - STOP: stop the current stimulation, set mode to WAITING
*/

enum commands_t { PROGRAM = 'P', STIM = 'S', STOP = 'A', OK = 'O', CODE = 'C', ITI = 'I' };

typedef struct {
    int code;    //which pin to write 
    int iti;     //inter trial interval
} stimulus_t;

// constants won't change. Used here to 
// set pin numbers:
const int ledPin =  7;      // the number of the LED pin
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

void setup() {
  //ensure mode at startup is WAITING for commands
  mode = WAITING;
  // set the digital pin as output:
  pinMode(ledPin, OUTPUT);      

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
     return Serial.read();
  }
}
bool wait_serial_timeout(int timeout)
{
  int count = 0;
  while (Serial.available() <= 0 && count <= timeout) {
    delay(100);
    count += 100;
  }
  return count <= timeout ? true : false;
}
//non-reentrant code !
char *read_serial_string()
{
  bool stop = false;
  static char buf[128];
  int i = 0;
  int timeout = millis();

  buf[sizeof(buf)-1] = 0;
  while (!stop && i < sizeof(buf)-1)
  {
    if (Serial.available() > 0)
    {
      buf[i] = Serial.read();
      if (buf[i] == '\n') 
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


int check_command_mode()
{
  int cmd = read_command();
  if (cmd != NONE)
  {
    //set the device to programming mode
    if (cmd == PROGRAM) return PROGRAMMING;
    if (cmd == STIM) return STIMULATING;
  }
  return WAITING;
}

int do_programming()
{
  Serial.println("Start programming mode");
  if (wait_serial_timeout(DEFAULT_TIMEOUT))
  {
    if (!read_serial_int(&num_stimuli)) return WAITING;
    for (int i=0; i<num_stimuli; i++)
    {
      int tmp = 0;
      if (!read_serial_int(&tmp)) return WAITING;
      stimuli[i].code = tmp;

      if (!read_serial_int(&tmp)) return WAITING;
      stimuli[i].iti = tmp;
    }
    return READY;
  }
}


//
// the critical part !!
//
int do_stimulation()
{
  //yet to start...
}

void loop()
{
  switch (mode)
  {
    case WAITING:
    case READY:
      mode = check_command_mode();
      break;
    case PROGRAMMING:
      mode = do_programming();
      if (mode == WAITING) 
        Serial.println("Programming mode aborted.");
      else if (mode == READY) 
        Serial.println("Programming OK, ready to stimulate");
      break;

    case STIMULATING:
      break;
    default:
      Serial.println("Unknow mode. switching to WAITING");
      mode = WAITING;
  };
}


