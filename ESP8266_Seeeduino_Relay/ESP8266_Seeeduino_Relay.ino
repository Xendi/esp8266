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


#define RESET 7            // CH_PD pin
#define GPIO0 6            // GPIO0
#define RST 5              // RST
#define SERIAL_IN 8        // Pin 8 connected to ESP8266 TX pin
#define SERIAL_OUT 9       // Pin 9 connected to ESP8266 RX pin




#include <AltSoftSerial.h>

AltSoftSerial mySerial;



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
  
  Serial.println("ESP8266/Seeeduino Relay");

  Serial.println("Enabling Module");
  
//  pinMode(RESET, OUTPUT);
//  pinMode(RST, OUTPUT);
//  pinMode(GPIO0, OUTPUT);
  digitalWrite(RST, 1);       // Enable RST pin
  digitalWrite(GPIO0, 1);     // Enable GPIO1 pin to prevent bootloader mode
  digitalWrite(RESET, 1);     // Enable CH_PD pin
  
//  delay(2000);                // wait for chip enable
  
  Serial.println("Chip Enabled");
  
//  reset();
//  delay(1000);                             // wait for reset to complete
  
  Serial.println("Module is ready.");
  

}

//                               ******** LOOP ********
void loop() 
{
  
  if (Serial.available() > 0) {char ch = Serial.read(); mySerial.print(ch);}
  
  if (mySerial.available() > 0) {char ch = mySerial.read(); Serial.print(ch);}
  
}



