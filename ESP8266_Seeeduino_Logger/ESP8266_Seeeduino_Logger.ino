/* ESP8266 connected to Seeeduino in 3V3 mode

// ESP8266:Arduino -> RST:5, GPIO0:6, CH_PD:7, UTXD:8, URXD:9

// ESP8266 pinout (from above, antenna to right:
// UTXD    GND
// CH_PD   GPIO2
// RST     GPIO0
// VCC     URXD

// will work only with ESP8266 firmware 0.9.2.2 or higher
// needs AltSoftSerial library for reliable comms http://www.pjrc.com/teensy/td_libs_AltSoftSerial.html

   
  Adapted from:
  'ESP8266 Retro Browser' - [TM] (http://hackaday.io/project/3072-esp8266-retro-browser)
  'Wifi meat thermometer' - bluesunit (http://www.reddit.com/r/arduino/comments/2kmgvg/wifi_meat_thermometer_with_trinket_esp8266/)

  David Reeves 2015
  CC NC-SA
  

  
  
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 */

#define SSID        "Bergen-Guest"
#define PASS        "GuestDevice"      // My luggage has the same combination!
#define DEST_HOST   "data.sparkfun.com"
#define TIMEOUT     5000    // mS
#define CONTINUE    false
#define HALT        true
#define PUBLIC_KEY "MGgojzqpnaf3ydgDWQzA" //data.sparkfun.com public key
#define PRIVATE_KEY "nz9ydg6kbmsVk1wKoX8W"      //data.sparkfun.com private key
#define RESET 7            // CH_PD pin
#define GPIO0 6            // GPIO0
#define RST 5              // RST
#define SERIAL_IN 8        // Pin 8 connected to ESP8266 TX pin
#define SERIAL_OUT 9       // Pin 9 connected to ESP8266 RX pin




#include <AltSoftSerial.h>

AltSoftSerial mySerial;

//#define ECHO_COMMANDS // Un-comment to echo AT+ commands to serial monitor

//                                     ********* FUNCTIONS ***********

// Print error message and loop stop.
void errorHalt(String msg)
{
  Serial.println(msg);
  Serial.println("HALT");
  while(true){};
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
    if (mySerial.available())
    {
      char ch = mySerial.read();
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
  {while(mySerial.available()) Serial.write(mySerial.read());}
  
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
  mySerial.println(cmd);
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
        errorHalt(cmd+" failed");// Critical failure halt.
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


boolean connectService(String service, int port) {  
  String serviceConnect = "AT+CIPSTART=\"TCP\",\"" + service + "\"," + port;
//  Serial.println(serviceConnect);
//  Handle connection errors
  if (!echoCommand(serviceConnect, "Linked", CONTINUE)) {          
    if (echoCommand(serviceConnect, "ALREAY CONNECT", CONTINUE)){
      echoCommand("AT+CIPCLOSE", "", CONTINUE);
      delay(2000);
    }
    if (echoCommand(serviceConnect, "busy p...", CONTINUE)) {
      reset();
      delay(5000);
    }
    delay(2000);
    return false;
  }
  delay(2000);
  return true;
 
}
 
 
boolean addToStream(String temp) {

  Serial.println(temp);

  
  if (!connectService("data.sparkfun.com",80)) return false;

// Get connection status 
  if (!echoCommand("AT+CIPSTATUS", "OK", CONTINUE)) return false;

 
// Build HTTP request.
  String toSend = "GET /input/" PUBLIC_KEY "?private_key=" PRIVATE_KEY;
  toSend += "&temp_c=" + temp;  
  //toSend += "&temp_f=" + temperature2_chr;
  //toSend += "&temp_c2=" + c_temp2;
  //toSend += "&temp_f2=" + f_temp2;
  
  toSend +=" HTTP/1.0\r\n\r\n";
  //toSend += " HTTP/1.1\r\n\Host: ";
  //toSend += DEST_HOST;
  //toSend += "\r\n\r\n";
 

// Ready the module to receive raw data
  if (!echoCommand("AT+CIPSEND="+String(toSend.length()), ">", CONTINUE))
  {
    echoCommand("AT+CIPCLOSE", "", CONTINUE);
    Serial.println("Connection timeout.");
    return false;
  }
  
// Send the raw HTTP request
  if(!echoCommand(toSend,"1 Success", CONTINUE)) return false;  // GET
  
  Serial.println("Data posted!");

//  echoCommand("AT+CIPCLOSE", "", CONTINUE);
   
  return true;
}
 
String ftoa(float number, uint8_t precision, uint8_t size) {
  // Based on mem,  16.07.2008
  // http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num = 1207226548/6#6

  // prints val with number of decimal places determine by precision
  // precision is a number from 0 to 6 indicating the desired decimial places
  // example: printDouble(3.1415, 2); // prints 3.14 (two decimal places)

  // Added rounding, size and overflow #
  // ftoa(343.1453, 2, 10) -> "    343.15"
  // ftoa(343.1453, 4,  7) -> "#      "
  // avenue33, April 10th, 2010

  String s = "";

  // Negative 
  if (number < 0.0)  {
    s = "-";
    number = -number;
  }

  double rounding = 0.5;
  for (uint8_t i = 0; i < precision; ++i)    rounding /= 10.0;

  number += rounding;
  s += String(uint16_t(number));  // prints the integer part

  if(precision > 0) {
    s += ".";                     // prints the decimal point
    uint32_t frac;
    uint32_t mult = 1;
    uint8_t padding = precision -1;
    while(precision--)     mult *= 10;

    frac = (number - uint16_t(number)) * mult;

    uint32_t frac1 = frac;
    while(frac1 /= 10)    padding--;
    while(padding--)      s += "0";

    s += String(frac,DEC) ;  // prints the fractional part
  }

  if (size>0)                // checks size
    if (s.length()>size)        return("#");
    else while(s.length()<size) s = "0"+s;

  return s;
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
  mySerial.begin(9600);       // Communication with ESP8266 (3V3!)
  
  Serial.println("ESP8266/Seeeduino DataLogger");

  Serial.println("Enabling Module");
  
//  pinMode(RESET, OUTPUT);
//  pinMode(RST, OUTPUT);
//  pinMode(GPIO0, OUTPUT);
  digitalWrite(RST, 1);       // Enable RST pin
  digitalWrite(GPIO0, 1);     // Enable GPIO1 pin to prevent bootloader mode
  digitalWrite(RESET, 1);     // Enable CH_PD pin
  
  delay(2000);                // wait for chip enable
  
  Serial.println("ESP8266 A0 Monitor");
  
  
  delay(5000);                             // wait for reset to complete

  echoCommand("AT+RST", "Ready", HALT);    // Reset & test if the module is ready  
  delay(5000);
  echoCommand("AT+CSYSWDTENABLE", "WDT Enabled", HALT);
  delay(500);
  Serial.println("Module is ready.");
  
  // Set up connection modes
  
  echoCommand("AT+GMR", "OK", CONTINUE);   // Retrieves the firmware ID (version number) of the module. 
  echoCommand("AT+CWMODE?","OK", CONTINUE);// Get module access mode. 
  echoCommand("AT+CWMODE=1", "", HALT);    // Station mode
  echoCommand("AT+CIPMUX=0", "", HALT);    // Allow one connection

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

  echoCommand("AT+CIFSR", "", HALT);           // Echo IP address. 
  
}



//                               ******** LOOP ********
void loop() 
{

  reset();                                // reset esp8266 each time around
  delay(5000);
 
  float tempC;
  tempC = analogRead(0);                  // Takes analog value from A0 pin
  
  Serial.print(tempC);                    // Print Temperature to serial port
  Serial.println(" C, ");
  delay(1000);  
  
    
  // Construct output string   

  String temperature_str = "";                        //Celsius  
  temperature_str += ftoa(tempC, 0, 3);
  temperature_str += "C";
  char temperature_chr[temperature_str.length()+1];   //create char buffer
  temperature_str.toCharArray(temperature_chr, temperature_str.length()+1); //convert to char
  

  
  // Send Data to Internet
   
  while(!addToStream(temperature_chr));   

  delay(50000);
 
}



