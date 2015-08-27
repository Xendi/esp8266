// Cactus Micro

// modified from code by TM at http://hackaday.io/project/3072-esp8266-retro-browser
// will work only with ESP8266 AT firmware 0.9.2.2 or higher
// 
// Uses a rewrite of the IRLib library, by cyborg5 (http://tech.cyborg5.com/irlib/)
//
//

#include <IRLib.h>

#define SSID        "SSID"
#define PASS        "PASS" // My luggage has the same combination!
#define TIMEOUT     5000 // mS
#define CONTINUE    false
#define HALT        true
#define RESET 13            // CH_PD pin
#define GPIO0 6            // GPIO0
#define RST 5              // RST
#define SERIAL_IN 8        // Pin 8 connected to ESP8266 TX pin
#define SERIAL_OUT 9       // Pin 9 connected to ESP8266 RX pin


// An IR LED must be connected to Cactus Micro PWM pin 9, or pin 3 for other Arduinos.

IRsend sender;

char cmd = ' ';
String help = "Makerspace AC Control\r\np: Power On/Off\r\nu: Temp Up\r\nd: Temp down\r\nl: Fan slower\r\nr: Fan faster\r\nc: Cool\r\ne: Energy saver\r\nf: Fan only\r\n";


//#define ECHO_COMMANDS // Un-comment to echo AT+ commands to serial monitor

//                                     ********* FUNCTIONS ***********

// Print error message and loop stop.
void errorHalt(String msg)
{
  Serial.println(msg);
  Serial.println("HALT");
  reset();
}

// Read characters from WiFi module and echo to serial until keyword occurs or timeout.
boolean echoFind(String keyword)
{
  byte current_char   = 0;
  byte keyword_length = keyword.length();
  
  // Fail if the target string has not been sent by deadline.
  unsigned long deadline = millis() + TIMEOUT;
  while(millis() < deadline)
  {
    if (Serial1.available())
    {
      char ch = Serial1.read();
      Serial.write(ch);

      if (ch == keyword[current_char])
        if (++current_char == keyword_length)
        {
          Serial.println();
          return true;
        }
    }
  }
  return false;  // Timed out
}

// Read and echo all available module output.
// (Used when we're indifferent to "OK" vs. "no change" responses or to get around firmware bugs.)
void echoFlush()
  {while(Serial1.available()) Serial.write(Serial1.read());}
  
// Echo module output until 3 newlines encountered.
// (Used when we're indifferent to "OK" vs. "no change" responses.)
void echoSkip()
{
  echoFind("\n");        // Search for nl at end of command echo
  echoFind("\n");        // Search for 2nd nl at end of response.
  echoFind("\n");        // Search for 3rd nl at end of blank line.
}

// Send a command to the module and wait for acknowledgement string
// (or flush module output if no ack specified).
// Echoes all data received to the serial monitor.
boolean echoCommand(String cmd, String ack, boolean halt_on_fail)
{
  Serial1.println(cmd);
  #ifdef ECHO_COMMANDS
    Serial.print("--"); Serial.println(cmd);
  #endif
  
  // If no ack response specified, skip all available module output.
  if (ack == "")
    echoSkip();
  else
    // Otherwise wait for ack.
    if (!echoFind(ack))          // timed out waiting for ack string 
      if (halt_on_fail)
        errorHalt(cmd+" failed");
        // Critical failure halt.
      else
        return false;            // Let the caller handle it.
  return true;                   // ack blank or ack found
}

// Connect to the specified wireless network.
boolean connectWiFi()
{
  String cmd = "AT+CWJAP=\"" SSID "\",\"" PASS "\"";
  if (echoCommand(cmd, "OK", CONTINUE)) // Join Access Point
  {
    Serial.println("Connected to WiFi.");
    return true;
  }
  else
  {
    Serial.println("Connection to WiFi failed.");
    return false;
  }
}



 
void reset()
{
  digitalWrite(RESET,LOW);
  delay(1000);
  digitalWrite(RESET,HIGH);
  delay(1000);
}



//                                         ******** SETUP ********
void setup()  {

  Serial.begin(9600);         // Communication with PC monitor via USB
  Serial1.begin(9600);       // Communication with ESP8266 (3V3!)
  
  Serial.println("ESP8266 IR Remote");

  Serial.println("Enabling Module");
  
//  pinMode(RESET, OUTPUT);
//  pinMode(RST, OUTPUT);
//  pinMode(GPIO0, OUTPUT);
  digitalWrite(RST, 1);       // Enable RST pin
  digitalWrite(GPIO0, 1);     // Enable GPIO1 pin to prevent bootloader mode
  digitalWrite(RESET, 1);     // Enable CH_PD pin
  
  delay(2000);                // wait for chip enable
  

  
  
  delay(5000);                             // wait for reset to complete

  echoCommand("AT+RST", "Ready", HALT);    // Reset & test if the module is ready  
  delay(5000);
  echoCommand("AT+CSYSWDTENABLE", "WDT Enabled", HALT);
  delay(500);
  Serial.println("Module is ready.");
  
  // Set up connection modes
  
  echoCommand("AT+GMR", "OK", CONTINUE);   // Retrieves the firmware ID (version number) of the module. 
  echoCommand("AT+CWMODE?","OK", CONTINUE);// Get module access mode. 
  echoCommand("AT+CWMODE=3", "OK", CONTINUE);    // Station mode
  echoCommand("AT+CIPCLOSE", "OK", CONTINUE);    // Allow > one connection
  echoCommand("AT+CIPMUX=1", "OK", CONTINUE);    // Allow > one connection
  echoCommand("AT+CWLAP", "OK", CONTINUE);    // List APs  
  delay(2000);
  
  //connect to the wifi
  boolean connection_established = false;
  for(int i=0;i<5;i++)
  {
    if(connectWiFi())
    {
      connection_established = true;
      break;
    }
  }
  if (!connection_established) errorHalt("Connection failed");
  
  delay(5000);

  echoCommand("AT+CIFSR", "OK", CONTINUE);           // Echo IP address. 

  delay(1000);

  while(!echoCommand("AT+CIPSERVER=1,8888", "OK", CONTINUE)) {}; //establish telnet server
  delay(5000);

}


// ******** LOOP ********


void loop() {
    if (Serial1.available() > 0) {
        cmd = Serial1.read();
        Serial.print("received: ");
        Serial.println(cmd);
        switch(cmd) {
            case 'p':
                sender.send(NEC,0x10AF8877, 32);
                Serial.println("power");
                break;
            case 'u':
                sender.send(NEC,0x10AF708F, 32);
                Serial.println("temperature up");
                break;
            case 'd':
                sender.send(NEC,0x10AFB04F, 32);
                Serial.println("temperatue down");
                break;
            case 'l':
                sender.send(NEC,0x10AF20DF, 32);
                Serial.println("fan slower");
                break;
            case 'r':
                sender.send(NEC,0x10AF807F, 32);
                Serial.println("fan faster");
                break;
            case 'c':
                sender.send(NEC,0x10AF906F, 32);
                Serial.println("cool");
                break;
            case 'e':
                sender.send(NEC,0x10AF40BF, 32);
                Serial.println("energy saver");
                break;
            case 'f':
                sender.send(NEC,0x10AFE01F, 32);
                Serial.println("fan only");
                break;
            case 'h':
                echoCommand("AT+CIPSEND=0,"+String(help.length()), ">", CONTINUE);
                echoCommand(help,"",CONTINUE);
                break;  
            case 'L':
                delay(1000);
                echoCommand("AT+CIPSEND=0,"+String(help.length()), ">", CONTINUE);
                echoCommand(help,"",CONTINUE);
                break;
            Serial.println("bad command");
        }
    }
}
