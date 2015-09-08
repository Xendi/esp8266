// Cactus Micro with ESP-11. Set jumper for hardware serial!

// modified from code by TM at http://hackaday.io/project/3072-esp8266-retro-browser
// will work only with ESP8266 firmware 0.9.2.2 or higher
// needs AltSoftSerial library for reliable comms http://www.pjrc.com/teensy/td_libs_AltSoftSerial.html



#define SSID        ""
#define PASS        "" // My luggage has the same combination!
#define DEST_HOST   "retro.hackaday.com"
#define DEST_IP     "192.254.235.21"
#define TIMEOUT     5000 // mS
#define CONTINUE    false
#define HALT        true
#define RESET 13            // CH_PD pin
#define GPIO0 12            // GPIO0
#define RST 5              // RST
#define SERIAL_IN 8        // Pin 8 connected to ESP8266 TX pin
#define SERIAL_OUT 9       // Pin 9 connected to ESP8266 RX pin




// #define ECHO_COMMANDS // Un-comment to echo AT+ commands to serial monitor

// Print error message and loop stop.
void errorHalt(String msg)
{
  Serial.println(msg);
  Serial.println("HALT");
  while (true) {};
}

// Read characters from WiFi module and echo to serial until keyword occurs or timeout.
boolean echoFind(String keyword)
{
  byte current_char   = 0;
  byte keyword_length = keyword.length();

  // Fail if the target string has not been sent by deadline.
  long deadline = millis() + TIMEOUT;
  while (millis() < deadline)
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
{
  while (Serial1.available()) Serial.write(Serial1.read());
}

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
        errorHalt(cmd + " failed"); // Critical failure halt.
      else
        return false;            // Let the caller handle it.
  return true;                   // ack blank or ack found
}

// Connect to the specified wireless network.
boolean connectWiFi()
{
  String cmd = "AT+CWJAP=\""; cmd += SSID; cmd += "\",\""; cmd += PASS; cmd += "\"";
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
  digitalWrite(RESET, LOW);

  delay(1000);
  digitalWrite(RESET, HIGH);
  delay(5000);
}

// ******** SETUP ********
void setup()
{
  Serial.begin(9600);         // Communication with PC monitor via USB
  Serial1.begin(9600);        // Communication with ESP8266 (3V3!)

  Serial.println("ESP8266 Demo");

  Serial.println("Enabling Module");

  pinMode(RESET, OUTPUT);
  pinMode(RST, OUTPUT);
  pinMode(GPIO0, OUTPUT);
  digitalWrite(RST, 1);
  digitalWrite(GPIO0, 1);

  reset();

  echoCommand("AT+CSYSWDTENABLE", "WDT Enabled", HALT);
  delay(500);

  //  Serial1.setTimeout(TIMEOUT);

  echoCommand("AT+RST", "ready", HALT);    // Reset & test if the module is ready
  Serial.println("Module is ready.");
  delay(1000);

  echoCommand("AT+GMR", "OK", CONTINUE);   // Retrieves the firmware ID (version number) of the module.
  delay(1000);

  echoCommand("AT+CWMODE?", "OK", CONTINUE); // Get module access mode.
  delay(1000);

  echoCommand("AT+CWLAP", "OK", CONTINUE); // List available access points - DOESN't WORK FOR ME
  delay(1000);

  echoCommand("AT+CWMODE=3", "OK", HALT);    // Station mode
  delay(1000);

  echoCommand("AT+CIPCLOSE", "", CONTINUE);

  echoCommand("AT+CIPMUX=1", "OK", HALT);    // Allow multiple connections (we'll only use the first).
  delay(1000);
  //connect to the wifi
  boolean connection_established = false;
  for (int i = 0; i < 5; i++)
  {
    if (connectWiFi())
    {
      connection_established = true;
      break;
    }
  }
  if (!connection_established) errorHalt("Connection failed");

  delay(5000);

  //echoCommand("AT+CWSAP=?", "OK", CONTINUE); // Test connection
  echoCommand("AT+CIFSR", "OK", HALT);         // Echo IP address. (Firmware bug - should return "OK".)
  //echoCommand("AT+CIPMUX=0", "", HALT);      // Set single connection mode
}

// ******** LOOP ********
void loop()
{
  
  // Establish TCP connection
  String cmd = "AT+CIPSTART=1,\"TCP\",\""; cmd += DEST_IP; cmd += "\",80";
  //  Handle connection errors
  while (!echoCommand(cmd, "OK", CONTINUE)) {
    reset();
    delay(5000);

  }

  // Get connection status
  if (!echoCommand("AT+CIPSTATUS", "OK", CONTINUE)) return;

  // Build HTTP request.
  cmd = "GET / HTTP/1.1\r\nHost: "; cmd += DEST_HOST; cmd += ":80\r\n\r\n";

  // Ready the module to receive raw data
  if (!echoCommand("AT+CIPSEND=1," + String(cmd.length()), ">", CONTINUE))
  {
    echoCommand("AT+CIPCLOSE", "", CONTINUE);
    Serial.println("Connection timeout.");
    return;
  }

  // Send the raw HTTP request
  echoCommand(cmd, "OK", CONTINUE);  // GET

  // Loop forever echoing data received from destination server.
  while (true)
    while (Serial1.available())
      Serial.write(Serial1.read());

  errorHalt("ONCE ONLY");
}

