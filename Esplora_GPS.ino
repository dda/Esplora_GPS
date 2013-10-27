// link between the computer and the SoftSerial Shield
//at 9600 bps 8-N-1
//Computer is connected to Hardware UART
//SoftSerial Shield is connected to the Software UART:D2&D3 
 
#include <SoftwareSerial.h>
#include <Wire.h>
#include <SPI.h>
#include <Esplora.h>
#include <TFT.h>  // Arduino LCD library
 
/* Already defined, just a cheat sheet
#define	ST7735_BLACK   0x0000
#define	ST7735_BLUE    0x001F
#define	ST7735_RED     0xF800
#define	ST7735_GREEN   0x07E0
#define ST7735_CYAN    0x07FF
#define ST7735_MAGENTA 0xF81F
#define ST7735_YELLOW  0xFFE0  
#define ST7735_WHITE   0xFFFF
*/

#define RxD 11
#define TxD  3 // Not important. In this config we're not using it anyway.

SoftwareSerial SoftSerial(11, 3);
char *buffer="                                                                                                                                ";
// buffer array for data recieve over serial port
int count=0;     // counter for buffer array 
void setup() {
  delay(2000);
  SoftSerial.begin(9600); // the SoftSerial baud rate   
  Serial.begin(57600); // the Serial port of Arduino baud rate.
  EsploraTFT.begin();
  EsploraTFT.background(0, 0, 0);
  
  // Draw splash screen text
  EsploraTFT.setTextColor(ST7735_YELLOW, ST7735_BLACK);
  EsploraTFT.setTextSize(2);
  EsploraTFT.setCursor(18, 12);
  EsploraTFT.print("GPS Tester");
  EsploraTFT.setTextSize(1);
}

void loadUpToDollar() {
  char c=SoftSerial.read();
  while(c!='$') {
    c=SoftSerial.read();
  }
}

int skipToNext(char *bf, char x, int pos) {
  int ix=pos;
  while(bf[ix]!=x) {
    ix+=1;
  }
  return ix;
}

void loop() {
  if (SoftSerial.available()) {
    loadUpToDollar();
    while (SoftSerial.available()) {
      // reading data into char array
      char c=SoftSerial.read();
      buffer[count++]=c; // write data into array
      if(count == 128)break;
      if(c==10)break;
      if(c==13)break;
    }
    if (strncmp (buffer, "GPRMC", 5) == 0) {
      buffer[count]=0;
      Serial.write("\n$");
      Serial.write(buffer); // if no data transmission ends, write buffer to hardware serial port
      int ix=skipToNext(buffer, ',', 0)+1;
      String s=String(buffer[ix++])+String(buffer[ix++])+":"+String(buffer[ix++])+String(buffer[ix++])+":"+String(buffer[ix++])+String(buffer[ix++])+" UTC";
      EsploraTFT.setCursor(12, 36);
      EsploraTFT.print(s);
      ix=skipToNext(buffer, ',', ix)+1;
      char c=buffer[ix++];
      if(c=='A') {
        ix++;
        EsploraTFT.print("  [ok]");
        Serial.println("  [ok]");
        // Valid
        int yx=skipToNext(buffer, '.', ix);
        int i;
        s="";
        for(i=ix;i<yx;i++) {
          s=s+String(buffer[i]);
        }
//        Serial.println("Digits up to the point: "+s);
        int Latitude = s.toInt();
        int Lat1 = Latitude/100;
        Serial.print("Latitude: ");
//        Serial.println(Latitude);
//        Serial.println("Lat1: "+String(Lat1));
        int Lat1b = (Latitude-(Lat1*100));
//        Serial.println("Lat1b: "+String(Lat1b));
        s="";
        yx+=1;
        ix=skipToNext(buffer, ',', yx);
        for(i=yx;i<ix;i++) {
          s=s+String(buffer[i]);
        }
        i+=1;
        c=buffer[i];
        i+=2;
//        Serial.println("s: "+s);
        double Lat2=s.toInt();
//        Serial.print("Lat2/1: ");
//        Serial.println(Lat2);
        Lat2=Lat1b+(Lat2/100000);
//        Serial.print("Lat2/2: ");
//        Serial.print(Lat2, 5);
//        Serial.println(" "+String(c));
        EsploraTFT.setCursor(12, 50);
        EsploraTFT.print(String(Lat1)+"d "+String(Lat1b)+"."+s+"mn "+c+"   ");
        Serial.println(String(Lat1)+"d "+String(Lat1b)+"."+s+"mn "+c+"   ");
        
        // Longitude
        Serial.println("Longitude");
        yx=skipToNext(buffer, '.', i);
        s="";
        for(ix=i;ix<yx;ix++) {
          s=s+String(buffer[ix]);
        }
//        Serial.println("Digits up to the point: "+s);
        int Lontitude = s.toInt();
        int Lont1 = Lontitude/100;
//        Serial.print("Lontitude: ");
//        Serial.println(Lontitude);
//        Serial.println("Lont1: "+String(Lont1));
        int Lont1b = (Lontitude-(Lont1*100));
//        Serial.println("Lont1b: "+String(Lont1b));
        s="";
        yx=ix+1;
        ix=skipToNext(buffer, ',', yx);
        for(i=yx;i<ix;i++) {
          s=s+String(buffer[i]);
        }
        i+=1;
        c=buffer[i];
        i+=2;
//        Serial.println("s: "+s);
        double Lont2=s.toInt();
//        Serial.print("Lont2/1: ");
//        Serial.println(Lont2);
        Lont2=Lont1b+(Lont2/100000);
//        Serial.print("Lont2/2: ");
//        Serial.print(Lont2, 5);
//        Serial.println(" "+String(c));
        EsploraTFT.setCursor(12, 65);
        EsploraTFT.print(String(Lont1)+"d "+String(Lont1b)+"."+s+"mn "+c+"   ");
        Serial.print(String(Lont1)+"d "+String(Lont1b)+"."+s+"mn "+c+"   ");
      } else {
        EsploraTFT.print("  [x]");
        Serial.println("Warning!");
      }
    }
    clearBufferArray(); // call clearBufferArray function to clear the storaged data from the array
    count = 0; // set counter of while loop to zero
  }
  if (Serial.available())
    // if data is available on hardwareserial port ==> data is comming from PC or notebook
    SoftSerial.write(Serial.read()); // write it to the SoftSerial shield
}

void clearBufferArray() {
  // function to clear buffer array
  for (int i=0; i<count;i++) {
    buffer[i]=NULL;
  } // clear all index of array with command NULL
}
