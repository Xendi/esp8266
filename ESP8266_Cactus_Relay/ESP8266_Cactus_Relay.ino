// Cactus Micro

// modified from code by TM at http://hackaday.io/project/3072-esp8266-retro-browser
// will work only with ESP8266 firmware 0.9.2.2 or higher
// needs AltSoftSerial library for reliable comms http://www.pjrc.com/teensy/td_libs_AltSoftSerial.html



#define SSID        "Bergen-Guest"
#define PASS        "GuestDevice" // My luggage has the same combination!
#define DEST_HOST   "retro.hackaday.com"
#define DEST_IP     "192.254.235.21"
#define TIMEOUT     5000 // mS
#define CONTINUE    false
#define HALT        true
#define RESET 13            // CH_PD pin
#define GPIO0 6            // GPIO0
#define RST 5              // RST
#define SERIAL_IN 8        // Pin 8 connected to ESP8266 TX pin
#define SERIAL_OUT 9       // Pin 9 connected to ESP8266 RX pin






void reset()
{
  digitalWrite(RESET,LOW);

  delay(1000);
  digitalWrite(RESET,HIGH);
  delay(5000);
}

// ******** SETUP ********
void setup()  
{
  Serial.begin(9600);         // Communication with PC monitor via USB
  Serial1.begin(9600);        // Communication with ESP8266 (3V3!)
  while (!Serial) {};
//  Serial.println("ESP8266 Demo");

  Serial.println("Enabling Module");
  
  pinMode(RESET, INPUT_PULLUP);
  pinMode(RST, INPUT);
  pinMode(GPIO0, OUTPUT);
  digitalWrite(RST, 1);
  digitalWrite(GPIO0, 1);
  
  reset();

  Serial.println("Module Enabled");
}  

// ******** LOOP ********
void loop() 
{

   /* send everything received from the hardware uart to usb serial & vv */

  if (Serial1.available() > 0) {
    char ch = Serial1.read();
    Serial.print(ch);
  }

  if (Serial.available() > 0) {
    char ch = Serial.read();
    Serial1.print(ch);
  }


    
}

