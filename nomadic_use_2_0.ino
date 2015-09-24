
// This version uses the GPS. It is intended for nomadic use. 
// Data is logged only if the GPS has a fix.

#include <chibi.h>
#include <ascii32.h>
#include <SPI.h>
#include <SD.h>
File myFile;

#define LINE_SZ 100

int led = 8;
int buzzer = 7;
static char line[LINE_SZ];
uint8_t sdCsPin = 15;
uint8_t radioCsPin = 30;
uint8_t radioSdnPin = 23; 

gps_t *gps;
scan_t scanBuf[1000]; // buffer to hold the scan points

// this is for printf
static FILE uartout = {0};  

char filename[8];

// The spectrum analyzer works from 240 MHz to 960 MHz
int start_frequency = 800;
int end_frequency = 900;


/**************************************************************************/
/*!
  Setup
*/
/**************************************************************************/
void setup()
{
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);
  
  pinMode(buzzer, OUTPUT);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  delay(100);
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  // fill in the UART file descriptor with pointer to writer.
  fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);
  
  // The uart is the standard output device STDOUT.
  stdout = &uartout ;
  
  Serial.begin(57600);
  //chibiCmdInit(57600);
  Serial1.begin(9600);
  ascii32.begin(radioCsPin, radioSdnPin, &Serial1, line);
  
  chibiCmdAdd("rd", cmdRadioRead);
  chibiCmdAdd("wr", cmdRadioWrite);
  chibiCmdAdd("freq", cmdChangeFreq);
  chibiCmdAdd("scan", cmdScan);
  
  // REMOVE 
  //gps = gps_getData();
  
  // Initialize the SD card part
  
  
  Serial.print("Initializing SD card...");
  // On ASCII-32 version 2, the pin it number 23. In version 1 it is 10.
  pinMode(sdCsPin, OUTPUT);
   
  if (!SD.begin(sdCsPin)) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");
  

 // open the file. note that only one file can be open at a time,
 // so you have to close this one before opening another.
 
  sprintf(filename,"nomad%03u.txt",1);
  // Serial.println(filename);
  
   if (! SD.exists(filename)){
      myFile = SD.open(filename, FILE_WRITE);
     
  }
  else
  {
    for (uint8_t i = 2; i < 999; i++) {
     sprintf(filename,"nomad%03u.txt",i);
    // Serial.println("Filename is now ");
    // Serial.println(filename); 
      if (! SD.exists(filename)){
      //  Serial.println("Filename ");
      //  Serial.println(filename); 
      //  Serial.println("does not exisit, so I will use it for the file! ");

        myFile = SD.open(filename, FILE_WRITE);
        break;
      }
    }
  }
     

 
  if (! myFile) {
    Serial.println("couldnt create file");
  }
  Serial.print("Logging to: ");
  Serial.println(filename);

}

/**************************************************************************/
/*!
  Loop
*/
/**************************************************************************/
void loop()
{
  int i, len;
  char text[100];
  char gpsstatus=-1;
  
  digitalWrite(led, LOW);
  
    
//  chibiCmdPoll();
  
// REMOVE  
    gps_update();
    gps = gps_getData();
// REMOVE  if (gps_available())
// REMOVE  {
    gps_clear_flag();    // clear the available flag since we read it already
   
    len = ascii32.radioScan(start_frequency, end_frequency, scanBuf);
    
    for (i=0; i<len; i++)
    {
      printf("%d, %d\n", scanBuf[i].freq, scanBuf[i].db);
    }
    printf("gps, %s, %s, %s, %s\n", gps->date, gps->utc, gps->lat, gps->lon);
    
   // Check if GPS has a fixed position
  
  //Serial.println((String)gps->status);
  
// REMOVE  
  gpsstatus=*(gps->status);
 
// REMOVE if (gpsstatus == 'V'){
  // if (gps->status == "V") {
// REMOVE    Serial.println("GPS is not ready yet!");
// REMOVE  } 
 // REMOVE else 
 if (gpsstatus == 'A')  {
   digitalWrite(led, HIGH);
  Serial.println("GPS has a position!");
  //unsigned long latitude;
  //unsigned long longitude;
  //latitude = atol(gps->lat);
  //longitude = atol(gps->lon);
  //latitude = (unsigned long)gps->lat;
  //longitude = (unsigned long)gps->lon;
  //Serial.println(latitude);
  //Serial.println(longitude);
  String latitude;
  String longitude;
  latitude = ((String)gps->lat);
  longitude = ((String)gps->lon);
  //latitude = (unsigned long)gps->lat;
  //longitude = (unsigned long)gps->lon;
  Serial.println(latitude);
  Serial.println(longitude);
  
  //Serial.println((String)gps->lat);
  //Serial.println((String)gps->lon);

  
  // File needs to be opened at every loop
   myFile = SD.open(filename, FILE_WRITE);
   Serial.print("Logging to: ");
   Serial.println(filename);
 
  
 // if the file opened okay, write to it:
  if (myFile) {
   // Serial.print("Writing to MicroSD Card.");
     for (i=0; i<len; i++)
     {
     // myFile.println("%d, %d\n", scanBuf[i].freq, scanBuf[i].db);
     sprintf(text,"%d, %d\n", scanBuf[i].freq, scanBuf[i].db);
     myFile.print(text);
    }
   // myFile.println("gps, %s, %s, %s, %s\n", gps->date, gps->utc, gps->lat, gps->lon);
     sprintf(text,"gps, %s, %s, %s, %s\n", gps->date, gps->utc, gps->lat, gps->lon);
     myFile.print(text);
    
    
   // close the file:
    myFile.close();
    Serial.println("done.");
    
    
    
  } 
  
  else {
    // if the file didn't open, print an error:
    Serial.println("error opening file to write data.");
    digitalWrite(buzzer, HIGH);
    delay(50);
    digitalWrite(buzzer, LOW);

  }  


// REMOVE} 
}else{
  Serial.println("GPS is not ready!");
  digitalWrite(buzzer, HIGH);
  delay(100);
  digitalWrite(buzzer, LOW);
  
}

}



/*********************************************************************/
//
//
/*********************************************************************/
void cmdRadioRead(int arg_cnt, char **args)
{
  uint8_t val, addr;
  
  addr = chibiCmdStr2Num(args[1], 16);
  val = ascii32.radioReadReg(addr);
  printf("Addr %02X = %02X.\n", addr, val);
}

/*********************************************************************/
//
//
/*********************************************************************/
void cmdRadioWrite(int arg_cnt, char **args)
{
  uint8_t val, addr;
  
  addr = chibiCmdStr2Num(args[1], 16);
  val = chibiCmdStr2Num(args[2], 16);
  ascii32.radioWriteReg(addr, val);
  printf("Addr %02X = %02X.\n", addr, val);
}

/*********************************************************************/
//
//
/*********************************************************************/
void cmdChangeFreq(int arg_cnt, char **args)
{
  int freq = chibiCmdStr2Num(args[1], 10);
  ascii32.radioChangeFreq(freq);
}

/*********************************************************************/
//
//
/*********************************************************************/
void cmdScan(int arg_cnt, char **args)
{
  int i, len;
  len = ascii32.radioScan(800, 900, scanBuf);
  
  for (i=0; i<len; i++)
  {
    printf("%d, %d\n", scanBuf[i].freq, scanBuf[i].db);
  }
}

/**************************************************************************/
// This is to implement the printf function from within arduino
/**************************************************************************/
static int uart_putchar (char c, FILE *stream)
{
    Serial.write(c);
    return 0;
}
