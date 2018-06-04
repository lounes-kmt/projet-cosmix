#include <Wire.h>
//#include <SoftwareSerial.h>
#include <TinyGPS.h>
#include <Serial1.h>
#include <RTClib.h>
#include <SD.h>
#include <LiquidCrystal.h> 

//proto1 #include <Adafruit_MCP23017.h>
//proto1 #include <Adafruit_RGBLCDShield.h>


// ------- //
// defines //
// ------- //
#define COINC_LINE 4
#define VERSION "m3.1"
// --- subrelease ---- //
// m3.0.1              //

// BMP085 pressure sensor
#define BMP085_ADDRESS 0x77  // I2C address of BMP085

// AdaFruit LCD RGB 20x4 Display
#define REDLITE 7
#define GREENLITE 6
#define BLUELITE 5

#define RED     255,0,0
#define YELLOW  255,255,0
#define GREEN   0,255,0
#define CYAN    0,255,255
#define BLUE    0,0,255
#define VIOLET  255,0,255
#define WHITE   255,255,255
#define ORANGE  255,127,0
#define PINK  255,64,127
#define LIGHTGREEN 100,255,100
#define LIGHTGRAY 127,127,127
#define DARKGRAY 64,64,64

#define LCDDATA1  53  // GREEN Wire
#define LCDDATA2  51  // BLLUE Wire 
#define LCDDATA3  49  // PURPLE Wire
#define LCDDATA4  47  // GRAY Wire
#define LCDDATA5  45  // WHITE Wire
#define LCDDATA6  43  // BROWN Wire

#define WELCOME_BKGCOLOR WHITE

#define ACQ_STOPPED_BKGCOLOR VIOLET
#define ACQ_STARTING_BKGCOLOR ORANGE
#define ACQ_RUNNING_BKGCOLOR GREEN
#define WARNING1_BKGCOLOR RED
#define WARNING2_BKGCOLOR YELLOW

#define CFG_ACQ_BKGCOLOR CYAN 
#define CFG_GPS_BKGCOLOR PINK 
#define DISP_TEMP_BKGCOLOR DARKGRAY 
#define DISP_GPS_BKGCOLOR LIGHTGRAY



// Buttons Pad
#define BUTTONSPAD_AI     0
#define BUTTON_UP     0x1
#define BUTTON_DOWN   0x2
#define BUTTON_LEFT   0x3
#define BUTTON_RIGHT  0x4
#define BUTTON_SELECT 0x5
#define BUTTON_NOTHING  0x0
#define BUTTON_NEXT   0x6

// GPS 
#define RXPIN 19
#define TXPIN 18
#define GPSBAUD  4800

// SD

#define SD_LINE 10

/*
place #define MEGA_SOFT_SPI 1 in libraries/SD/utility/Sd2Card.h
 * SD card attached to SPI bus as follows:
 ** MOSI - pin 11
 ** MISO - pin 12
 ** CLK - pin 13
 ** CS  - pin 10
 
 
 * Interrupt pin  2 .
 
 * GPS RX: pin 19
 TX: pin 18
 
 * BMP085 SDA 20
 SCL 21
 */

// Display

#define NB_CFG_ACQ_DIGI 4
#define MAX_DURATION 9999

#define NB_MENU 5
#define MENU_ACQ     0
#define MENU_CFG_ACQ 1
#define MENU_CFG_GPS 2
#define MENU_DISP_TP  3
#define MENU_DISP_GPS  4

#define RTC_UNSET 0
#define RTC_PC    1
#define RTC_LOCAL 2
#define RTC_GPS   3

//LCD 20x4 
#define RTC_DISPLAY_POS 19
//LCD 16x2 
//#define RTC_DISPLAY_POS 15


// --------- //
// Constants //
// --------- //

// SD Card 
const int chipSelect = SD_LINE;
// BMP085
const unsigned char OSS = 0;  // Oversampling Setting


// -------- //
// Globals  //
// -------- //


// Config
bool continuousmode = true;
int duration = 0;
bool acqrunning = false;
char filename[15];
File dataFile;
char gpsfilename[15];
File gpsFile;


// Event Counter
volatile long countPulse1=0;
volatile long countPulse2=0;
volatile long countCoinc=0;
volatile boolean event1=false;
volatile boolean event2=false;
volatile boolean eventc=false;
volatile long time0;

// LCD 
byte newChar[8] = {//4,14,31,0,0,0,0}; "^"
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000
};
byte newChar1[8] = {//4,14,31,0,0,0,0}; "intersection" 
  B00000,
  B01110,
  B11011,
  B10001,
  B10001,
  B10001,
  B10001,
  B00000
};

byte newChar2[8] = { // antenne GPS
  B00000, 
  B10001, 
  B10010, 
  B01100, 
  B01100, 
  B10011, 
  B10000, 
  B10000
};

byte newChar3[8] = { //"PC"
  B11000, 
  B10100, 
  B11000, 
  B10011, 
  B00100, 
  B00100, 
  B00011, 
  B00000 
};


byte newChar4[8] = { //"LC"
  B10000, 
  B10000, 
  B10000, 
  B11011, 
  B00100, 
  B00100, 
  B00011, 
  B00000 
};

byte newChar5[8] = { //"é"
  B00010, 
  B00100, 
  B01110, 
  B10001, 
  B11111, 
  B10000, 
  B01110, 
  B00000 
};

byte newChar6[8] = { //"à"
  B01000, 
  B00100, 
  B01110, 
  B00001, 
  B01111, 
  B10001, 
  B01111, 
  B00000 
};



// you can change the overall brightness by range 0 -> 255
int brightness = 255;
char cmode[2]={
  'D',char(243)};
char RTCmodeDisplay[4]={
  char(255),'\3','\4','\2'};
int RTCmode = RTC_UNSET;
char mess[80];
char longmess[200];
int longmessstart;
char cfg_acq_selindex = 0;
boolean acqdisplay = true;
boolean gpsmode = false;
boolean gpsset = false;
int selmenu = MENU_ACQ;
int localtime = 2;
char slatitude[15], slongitude[15];
float gpsaltitude = -999;
float latitude = -999;
float longitude = -999;
int lannee;
byte lemois;
byte lejour;
byte lheure;
byte laminute;
byte laseconde;
byte lescentiemes;
unsigned long lesmillis;
char lestatus[5];
char ascdate[100];
char *menu[NB_MENU] = {  
  "","Config Acquisition","Config GPS","Temp/Pression Info","GPS Info"};


// BMP085 Calibration values
int ac1;
int ac2; 
int ac3; 
unsigned int ac4;
unsigned int ac5;
unsigned int ac6;
int b1; 
int b2;
int mb;
int mc;
int md;
// b5 is calculated in bmp085GetTemperature(...), this variable is also used in bmp085GetPressure(...)
// so ...Temperature(...) must be called before ...Pressure(...).
long b5; 
short temperature;
long pressure;

//ADKeyboard Module
int adc_key_val[5] ={ 
  50, 200, 400, 600, 800 };
int NUM_KEYS = 5;
int adc_key_in;
int key=-1;
int oldkey=-1;


// Serial comm data
char serialbuffer[20];
boolean stringComplete = false;
char datamess[200];
boolean serialcom = false;

// --------------- //
// Globals Objects //
// --------------- //

//proto1 Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();
//proto1 SoftwareSerial uart_gps(RXPIN, TXPIN);

RTC_DS1307 RTC;
//RTC_Millis RTC_m;
TinyGPS gps;
DateTime now;
LiquidCrystal lcd(LCDDATA1,LCDDATA2, LCDDATA3, LCDDATA4, LCDDATA5, LCDDATA6);

//======================================================
// Setting LCD RGB BackLight color
//======================================================
void setBacklight(uint8_t r, uint8_t g, uint8_t b) 
{
  // normalize the red LED - its brighter than the rest!
  //  r = map(r, 0, 255, 0, 170);
  //  g = map(g, 0, 255, 0, 255);
  //  b = map(b, 0, 255, 0, 255);
  r = map(r, 0, 255, 0, 255);
  g = map(g, 0, 255, 0, 255);
  b = map(b, 0, 255, 0, 255);
  r = map(r, 0, 255, 0, brightness);
  g = map(g, 0, 255, 0, brightness);
  b = map(b, 0, 255, 0, brightness);
  // common anode so invert!
  r = map(r, 0, 255, 255, 0);
  g = map(g, 0, 255, 255, 0);
  b = map(b, 0, 255, 255, 0);
  analogWrite(REDLITE, r);
  analogWrite(GREENLITE, g);
  analogWrite(BLUELITE, b);
}

//void setBacklight(uint8_t r, uint8_t g, uint8_t b) 
//{
//  boolean rb = boolean(r);
//  boolean gb = boolean(g);
//  boolean bb = boolean(b);
//  digitalWrite(REDLITE, !rb);
//  digitalWrite(GREENLITE, !gb);
//  digitalWrite(BLUELITE, !bb);
//}


//======================================================
// Stop Acquisition    (VIOLET BackGround color = Acq stopped)
// --------------------------------
// Detach interrupts
// Close datafile
// Display final counts
//======================================================
void stopAcquisition()
{
  detachInterrupt(0);
  detachInterrupt(1);
  event1 = false;
  event2 = false;
  eventc = false;
  if(dataFile)dataFile.close();
  //         dataFile = 0;
  setBacklight(ACQ_STOPPED_BKGCOLOR);
  acqrunning = false;
  if(serialcom)Serial.println("stopped");
  displaycount();
  return;
}

//======================================================
// Start Acquisition (GREEN BackGround = Acq running)
// --------------------------------
// Get initial time
// Open datafile
// attach interrupts
// Display counts
//======================================================
void startAcquisition()
{
  setBacklight(ACQ_STARTING_BKGCOLOR);
  int essai = 0;
  event1 = false;
  event2 = false;
  eventc = false;
  if (gpsmode)
  {
    while(!gpsset)
    {
      getGPSInfo();
      sprintf(mess,"Attente v\5rouillage du GPS");
      twolinesmess(0,2,mess);
      delay(2000);
      (essai%2)?setBacklight(WARNING1_BKGCOLOR):setBacklight(WARNING2_BKGCOLOR);
      essai = essai +1;
    } 
  }
  setBacklight(ACQ_RUNNING_BKGCOLOR);
  now= RTC.now();

  //         File root = SD.open("/COSMIX/");
  //         char lastfilename[100]="CMX00000.txt";
  //         bool lastfile = false;
  //         while(!lastfile)
  //           {
  //            File entry =  root.openNextFile();
  //            if(entry)
  //               sprintf(lastfilename,"%s",entry.name());
  //            else
  //               lastfile = true;
  //           }
  //         root.close();
  //         sprintf(filename,"/COSMIX/CMX%05d.txt",atoi(&lastfilename[3])+1);
  sprintf(filename,"COSMIX.TXT");
  dataFile = SD.open(filename, FILE_WRITE);

  if (dataFile)
  {
    dataFile.println("----==== Nouvelle Acquisition ====---");
    if (continuousmode)
      sprintf(mess,"Mod: infini            Heure locale : UTC%+i",localtime);
    else
      sprintf(mess,"Mod:%curee %04dmn     Heure locale : UTC%+i",cmode[continuousmode],duration,localtime);
    dataFile.println(mess);              
  }

  if (gpsFile)gpsFile.println("----==== Nouvelle Acquisition ====---");


  attachInterrupt(0, gestionINT01, RISING);
  attachInterrupt(1, gestionINT02, RISING);
  time0 = millis();
  displaycount();
  acqrunning = true;
  if(serialcom)Serial.println("started");
  return;
}


//======================================================
// Acquisition Display setting  
// BackGround GREEN when acquisition is running, VIOLET when stopped
// -------------------------------
// Set Acquisition Display header
// Display RTC status 
// Display Counts
// Manage Buttons 
//   * RIGHT : Start/Stop Acquisition
//   * LEFT  : Reset (or Stop and Reset if running)
//   * SELECT : Next menu
//======================================================
void acquisition_display(uint8_t buttons)
{
  switch(buttons)
  {
  case BUTTON_NEXT:
    lcd.clear();
    (acqrunning)?setBacklight(ACQ_RUNNING_BKGCOLOR):setBacklight(ACQ_STOPPED_BKGCOLOR);
    lcd.setCursor(0, 0);
    sprintf(mess,"%c  C1  C2 C1\1C2",cmode[continuousmode]);
    lcd.print(mess);
    displayRTC();
    displaycount();
    break;
  case BUTTON_LEFT:
    if(acqrunning)
    { 
      stopAcquisition();
    }
    InitCounters();
    displaycount();
    acquisition_display(BUTTON_NOTHING);
    break;
  case BUTTON_RIGHT:
    if(acqrunning)
    {
      stopAcquisition();
    }
    else
    {
      startAcquisition();
    } 
    acquisition_display(BUTTON_NOTHING);
    break;
  case BUTTON_SELECT:
    selmenu  = (selmenu+1)%NB_MENU;
    buttons = BUTTON_NEXT;
    selectdisplay(false,buttons);
    return;
  }
  buttons = BUTTON_NOTHING;
  return;
}

//======================================================
// Config Acquisition
// BackGround CYAN
// -------------------------------
// Set Config Acquisition Display header
// Acquisition mode 
//            - continuous (infinity symbol)
//            - finite (run duration limited to selected mn)
// Manage Buttons
//   * RIGHT/LEFT : Change cursor position
//   * UP/DOWN : change mode or change duration if finite mode selected
//   * SELECT : Next menu
//======================================================
void menu_cfg_acq(uint8_t buttons)
{
  int factor[NB_CFG_ACQ_DIGI] = {
    1000,100,10,1    };
  switch(buttons)
  {
  case BUTTON_NEXT:
    <menu_cfg_acq>
    </menu_cfg_acq>
    cfg_acq_selindex = 0;
    break;
  case BUTTON_UP:
    if (cfg_acq_selindex)
    {
      duration += factor[cfg_acq_selindex-1];
      if(duration>MAX_DURATION)duration=MAX_DURATION;
    }
    else
    {
      continuousmode = !continuousmode; 
    }
    break;
  case BUTTON_DOWN:
    if (cfg_acq_selindex)
    {
      duration -= factor[cfg_acq_selindex-1];
      if(duration<0)duration = 0;
    }
    else
    {
      continuousmode = !continuousmode;
    }
    break;
  case BUTTON_LEFT:
    if(!continuousmode){
      cfg_acq_selindex = ((cfg_acq_selindex+NB_CFG_ACQ_DIGI)%(NB_CFG_ACQ_DIGI+1));
    }
    break;
  case BUTTON_RIGHT:
    if(!continuousmode){
      cfg_acq_selindex = ((cfg_acq_selindex+1)%(NB_CFG_ACQ_DIGI+ 1));
    }
    break;
  case BUTTON_SELECT:
    if(duration==0)
    {
      continuousmode = true;
    }
    selmenu  = (selmenu+1)%NB_MENU;
    buttons = BUTTON_NEXT;
    selectdisplay(false,buttons);
    return;
  } 
  config_acq_display(); 
  buttons = BUTTON_NOTHING;
  return;
}

//======================================================
// Config GPS mode
// BackGround PINK
// -------------------------------
// Set Config GPS Display header
// Acquisition gpsmode 
//            - false (default)
//                  acquisition can start without GPS lock
//                  local time (from RTC) is used until 
//

//            - true 
// Manage Buttons
//   * RIGHT/LEFT : Change cursor position
//   * UP/DOWN : change mode or change duration if finite mode selected
//   * SELECT : Next menu
//======================================================
void menu_cfg_GPS(uint8_t buttons)
{

  switch(buttons)
  {
  case BUTTON_NEXT:
    lcd.clear();
    setBacklight(CFG_GPS_BKGCOLOR);
    lcd.setCursor(0,0);
    lcd.print(menu[selmenu]);
    longmessstart = 0;
    break;
  case BUTTON_UP:
    gpsmode = !gpsmode;
    break;
  case BUTTON_DOWN:
    gpsmode = !gpsmode;
    break;
  case BUTTON_SELECT:
    selmenu  = (selmenu+1)%NB_MENU;
    buttons = BUTTON_NEXT;
    selectdisplay(false,buttons);
    return;
  }     
  config_gps_display();
  buttons = BUTTON_NOTHING;
  return;
}

void menu_disp_TP(uint8_t buttons)
{
  switch(buttons)
  {
  case BUTTON_NEXT:
    lcd.clear();
    setBacklight(DISP_TEMP_BKGCOLOR);
    lcd.setCursor(0,0);
    lcd.print(menu[selmenu]);
    break;
  case BUTTON_UP:
    break;
  case BUTTON_DOWN:
    break;
  case BUTTON_LEFT:
    break;
  case BUTTON_RIGHT:
    break;
  case BUTTON_SELECT:
    selmenu  = (selmenu+1)%NB_MENU;
    buttons = BUTTON_NEXT;
    selectdisplay(false,buttons);
    return;
  }
  buttons = BUTTON_NOTHING;
  return;
}

void menu_disp_GPS(uint8_t buttons)
{
  switch(buttons)
  {
  case BUTTON_NEXT:
    lcd.clear();
    setBacklight(DISP_GPS_BKGCOLOR);
    lcd.setCursor(0,0);
    lcd.print(menu[selmenu]);
    break;
  case BUTTON_UP:
    break;
  case BUTTON_DOWN:
    break;
  case BUTTON_LEFT:
    break;
  case BUTTON_RIGHT:
    break;
  case BUTTON_SELECT:
    selmenu  = (selmenu+1)%NB_MENU;
    buttons = BUTTON_NEXT;
    selectdisplay(false,buttons);
    return;
  }
  buttons = BUTTON_NOTHING;
  return;
}


void selectdisplay(bool init,uint8_t buttons)
{
  if (init)
  {
    lcd.clear();
    setBacklight(CYAN);
    cfg_acq_selindex = 0;
  }
  switch(selmenu)
  {
  case MENU_ACQ:
    acquisition_display(buttons);
    break;
  case MENU_CFG_ACQ:
    menu_cfg_acq(buttons);
    break;
  case MENU_CFG_GPS:
    menu_cfg_GPS(buttons);
    break;
  case MENU_DISP_TP:
    menu_disp_TP(buttons);
    break;
  case MENU_DISP_GPS:
    menu_disp_GPS(buttons);
    break;
  }
  delay(200);
}


void config_acq_display()
{
  char selindexpos[NB_CFG_ACQ_DIGI+1] = {
    6,12,13,14,15    };
  lcd.setCursor(0, 1);
  if (continuousmode)
    sprintf(mess,"Mod : %c           ",cmode[continuousmode]);
  else
    sprintf(mess,"Mod : %cur\5e %04dmn",cmode[continuousmode],duration);
  lcd.print(mess);
  lcd.setCursor(4,2);
  lcd.print("                ");
  lcd.setCursor(selindexpos[cfg_acq_selindex],2);
  lcd.print('\0'); 
}

void config_gps_display()
{
  lcd.setCursor(0, 1);
  char cursormess[10];
  if (gpsmode)
  {
    sprintf(mess,"Mod : GPS requis   ");
    sprintf(cursormess,"----");
  }
  else
  {
    sprintf(mess,"Mod : GPS optionnel");
    sprintf(cursormess,"-------");
  }
  lcd.print(mess);
  lcd.setCursor(4,2);
  lcd.print("               ");
  lcd.setCursor(10,2);
  lcd.print('\0'); 
  lcd.print(cursormess); 
  lcd.print('\0');
}


int get_key(unsigned int input)
{
  int k;
  for (k = 0; k < NUM_KEYS; k++)
    if (input < adc_key_val[k])break;
  if (k >= NUM_KEYS)k = -1;  // No valid key pressed
  return k;
}

int readButtons()
{
  adc_key_in = analogRead(BUTTONSPAD_AI);
  switch(get_key(adc_key_in))
  {
  case 0:
    return BUTTON_SELECT; 
  case 1: 
    return BUTTON_UP; 
  case 2: 
    return BUTTON_RIGHT; 
  case 3: 
    return BUTTON_LEFT; 
  case 4:           
    return BUTTON_DOWN; 
  }       
  return BUTTON_NOTHING;
}

void ddmmssff(char *sval,float val, float invalid)
{
  char sign =' ';
  int ddval,mmval,ssval,ffval;
  float dum;
  if (val==invalid)
  {
    sprintf(sval,"**%c**'**.**%c",char(223),char(34));
    return;
  }
  if (val<0)
  {
    sign = '-';
    val = -val; 
  }
  ddval = int(val);
  dum = (val-ddval)*60;
  mmval = int(dum);
  dum = (dum-mmval)*60;
  ssval = int(dum);
  dum = (dum-ssval)*100;
  ffval = int(dum);
  sprintf(sval,"%c%d%c%02d'%02d.%02d%c",sign,ddval,char(223),mmval,ssval,ffval,char(34));
  return;
}


void getGPSInfo()
{
  boolean gpsvalid = false;
  unsigned long age;
  char sdiff[50];
  char lmess[4];
  lesmillis = millis();
  now = RTC.now();
  lannee = now.year();
  lemois = now.month();
  lejour = now.day();
  lheure = now.hour();
  laminute = now.minute();
  laseconde = now.second();
  lescentiemes = 0;
  sprintf(lestatus,"*");
  while (Serial1.available())     // While there is data on the RX pin...
  {
    int c = Serial1.read();    // load the data into a variable...
    if(gps.encode(c))      // if there is a new valid sentence...
    { 
      sprintf(sdiff,"");
      gps.f_get_position(&latitude,&longitude);
      gps.crack_datetime(&lannee,&lemois,&lejour,&lheure,&laminute,&laseconde,&lescentiemes,&age);
      gpsaltitude = gps.f_altitude();
      lheure=lheure+localtime;
      DateTime gpsdate = DateTime(lannee,lemois,lejour,lheure,laminute,laseconde);
      if((RTCmode!=RTC_GPS)&&(age<=114)&&(lejour!=0))
      {
        int diff;
        RTC.adjust(gpsdate); 
        RTCmode=RTC_GPS;
        displayRTC();
        diff = ((lheure-now.hour())*60.+(laminute-now.minute()))*60+(laseconde-now.second());
        sprintf(sdiff,"Ajustement horloge : %d secondes",diff);
        if (dataFile) 
        {
          dataFile.println(sdiff);
        }
      }
      sprintf(lestatus,"%d",gps.satellites());

      //           ddmmssff(slatitude,latitude,TinyGPS::GPS_INVALID_F_ANGLE);
      //           ddmmssff(slongitude,longitude,TinyGPS::GPS_INVALID_F_ANGLE);
      //           gpsaltitude = (TinyGPS::GPS_INVALID_F_ALTITUDE)?99999:gps.altitude();
      //           if((RTCmode==RTC_GPS)&&(age!=TinyGPS::GPS_INVALID_AGE))
      //             {
      //              gpsvalid = true;
      //              sprintf(ascdate,"%s%02d/%02d/%04d %02d:%02d:%02d.%02d (%i) %9lu",sdiff,gpsdate.day(),gpsdate.month(),gpsdate.year(),gpsdate.hour(),gpsdate.minute(),gpsdate.second(),hundredths,gps.satellites(),mills);
      //              sprintf(ascdate,"%s%02d/%02d/%04d %02d:%02d:%02d.%02d (%i) %9lu",sdiff,gpsdate.day(),gpsdate.month(),gpsdate.year(),gpsdate.hour(),gpsdate.minute(),gpsdate.second(),hundredths,gps.satellites(),mills);
      //             }
    }
  }
}

void displaycount()
{
  sprintf(mess," %4ld %4ld  %4ld     ",countPulse1,countPulse2,countCoinc);
  lcd.setCursor(0,1);
  lcd.print(mess);
}

void displayRTC()
{
 lcd.setCursor(RTC_DISPLAY_POS,0);
 lcd.print(RTCmodeDisplay[RTCmode]);
}

void setup()   
{ 
  boolean SD_OK=false;
  gpsset = false;
  serialcom = false;
  lcd.begin(20, 4);
  lcd.createChar(0, newChar);
  lcd.createChar(1, newChar1);
  lcd.createChar(2, newChar2);
  lcd.createChar(3, newChar3);
  lcd.createChar(4, newChar4);
  lcd.createChar(5, newChar5);
  lcd.createChar(6, newChar6);
  lcd.clear();

  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  bmp085Calibration();
  pinMode(REDLITE, OUTPUT);
  pinMode(GREENLITE, OUTPUT);
  pinMode(BLUELITE, OUTPUT);
  pinMode(SD_LINE, OUTPUT);
  pinMode(COINC_LINE,INPUT);
  Serial1.begin(GPSBAUD);  
  setBacklight(WELCOME_BKGCOLOR);

  lcd.setCursor(0,0);
  sprintf(mess,"COSMIX    v%6s",VERSION);
  lcd.print(mess);
  twolinesmess(0,1,"D\5tection des muons cosmiques");
  lcd.setCursor(0,3);
  lcd.print("par le CENBG");
  lcd.setCursor(19,0);
  lcd.print("\3");
  delay(1000);

  if (!RTC.isrunning()||(RTC.now().year()==2165)) 
  {
    char mess[50];
    sprintf(mess,"Horloge non active!");
    lcd.setCursor(0,4);
    lcd.print(mess);
    if(serialcom)Serial.println(mess);
    delay(2000);
    RTC.adjust(DateTime(__DATE__, __TIME__));
    now = RTC.now();
    sprintf(mess,"Ajustement de la date \6 %04d/%02d/%02d %02d:%02d:%02d",now.year(),now.month(),now.day(),now.hour(),now.minute(),now.second());
    twolinesmess(0,2,mess);
    delay(4000);
    twolinesmess(0,2,NULL);
    delay(2000);
    if(serialcom)Serial.println(now.year());
    RTCmode = RTC_PC;
  }
  else
    RTCmode = RTC_LOCAL;
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  sprintf(mess,"Initialisation carte SD...");
  twolinesmess(0,2,mess);
  delay(1000);
  if(serialcom)Serial.print(mess);
  // SD Init
  SD_OK = SD.begin(chipSelect);
  sprintf(mess,(SD_OK)?"SD initialis\5e.":"Echec SD, ou SD absente");
  (SD_OK)?setBacklight(ACQ_RUNNING_BKGCOLOR):setBacklight(ACQ_STARTING_BKGCOLOR);
  twolinesmess(0,2,mess);
  if(serialcom)Serial.println(mess);
  sprintf(gpsfilename,"GPS.TXT");
  gpsFile = SD.open(gpsfilename, FILE_WRITE);

  delay(1000);
  setBacklight(WELCOME_BKGCOLOR);
  twolinesmess(0,2,NULL);
  // see if the card is present and can be initialized:

  //proto 1  pinMode(4,INPUT); ???
  // acquisition auto start
  acqrunning = false;
  continuousmode = true;
  serialbuffer[0]=0;
  InitCounters();
  selmenu = MENU_ACQ;
  acquisition_display(BUTTON_NEXT);
  acquisition_display(BUTTON_RIGHT);
}


void float2string(float f,char *s,int precision)
{
  if ((long(f)==0)&&(f<0))
    sprintf(s,"-%ld.%lu",long(f),(f>=0)?long((f-long(f))*pow(10,precision)):long((long(f)-f)*pow(10,precision)));
  else
    sprintf(s,"%ld.%lu",long(f),(f>=0)?long((f-long(f))*pow(10,precision)):long((long(f)-f)*pow(10,precision)));
  return;
}

void InitCounters()
{
  countPulse1=0;
  countPulse2=0;
  countCoinc=0;  
  event1 = false;
  event2 = false;
  eventc = false;
  return;
}

uint8_t serialcommandanalysis()
{
  char buttonsstring[] ="start stop  reset stat  mod   gps   *idn  ";
  char command[10];
  char data[10];
  boolean request = false; 
  sscanf(serialbuffer,"%s %s",command,data);
  if(command[strlen(command)-1]=='?')
  {
    request = true;
    command[strlen(command)-1]=0;
  }
  char *s = strstr(buttonsstring,command);
  switch((s-buttonsstring)/6)
  {
  case 0:
    selmenu = MENU_ACQ;
    selectdisplay(false,BUTTON_NEXT);
    return (acqrunning)?BUTTON_NOTHING:BUTTON_RIGHT; 
  case 1: 
    selmenu = MENU_ACQ;
    selectdisplay(false,BUTTON_NEXT);
    return (acqrunning)?BUTTON_RIGHT:BUTTON_NOTHING; 
  case 2: 
    selmenu = MENU_ACQ;
    selectdisplay(false,BUTTON_NEXT);
    return BUTTON_LEFT; 
  case 3:
    Serial.println(acqrunning?"STATUS running":"STATUS stopped");     
    return BUTTON_NOTHING; 
  case 4:
    if(request)
    {
      if(continuousmode)
        Serial.println("MOD Infini");
      else
      {
        sprintf(mess,"MOD %04i",duration);
        Serial.println(mess);
      } 
    }
    else
    {
      if(!acqrunning&&atoi(data)>0)
      {
        duration = atoi(data);
        continuousmode = false;
      }
      else
      {
        continuousmode = true;        
      }
      selectdisplay(false,BUTTON_NEXT);
    }
    return BUTTON_NOTHING; 
  case 5:
    if(request)
    {
      if(gpsmode)
        Serial.println("GPS Requis");
      else
        Serial.println("GPS Optionnel");
    }
    else
    {
      if(!acqrunning&&strstr(data,"req"))
      {
        gpsmode = true;
      }
      else
      {
        gpsmode = false;
      }
      selectdisplay(false,BUTTON_NEXT);
    }
    return BUTTON_NOTHING; 
  case 6:
    serialcom = false;
    Serial.flush();
    sprintf(mess,"COSMIX %s",VERSION);
    Serial.println(mess);
    return BUTTON_NOTHING; 
  }       
  return BUTTON_NOTHING; 
}

void loop()
{
  bool init=false;
  uint8_t buttons = readButtons();
  if(stringComplete)
  {
    /*   Serial.print("Incomming String  --> ");
     Serial.println(serialbuffer);*/
    buttons = serialcommandanalysis();
    stringComplete=false;
  }
  if(acqrunning&!continuousmode)
  {
    // si l'acquisition tourne et qu'elle n'est pas en mode continu, on envoie le stop
    // en simulant une pression sur le bouton droit du menu acquisition
    if((millis()-time0)>(60000*duration))
    {
      acquisition_display(BUTTON_NEXT);
      acquisition_display(BUTTON_RIGHT);
    }
  }
  if(selmenu==MENU_CFG_GPS)
  {
    getGPSInfo();
    int len;
    sprintf(longmess,"                    %s %s %s                    ",slatitude,slongitude,ascdate);
    len = strlen(longmess)-20;
    strncpy(mess,&longmess[longmessstart],19);
    mess[20]=0;
    lcd.setCursor(0,3);
    lcd.print(mess);
    longmessstart = (longmessstart+1)%len;
    displayRTC();
    delay(250);
  }
  if(selmenu==MENU_DISP_TP)
  {
    temperature = bmp085GetTemperature(bmp085ReadUT());
    pressure = bmp085GetPressure(bmp085ReadUP());
    lcd.setCursor(0,2);
    sprintf(mess,"Temp : %d.%d %cC",(int)((float)temperature/10.),(temperature<0)?(int)((-temperature)%10):(int)(temperature%10),char(223));
    lcd.print(mess);
    sprintf(mess,"Pression : %lu Pa",(long)pressure);
    lcd.setCursor(0,3);
    lcd.print(mess);
    delay(250);
  }

  if (buttons)selectdisplay(init,buttons);

  //  if(!acqrunning&&RTCmode!=RTC_GPS)
  getGPSInfo();

  if(selmenu==MENU_DISP_GPS)
  {
    char slat[30];
    char slong[30];
    float2string(latitude,slat,6);
    float2string(longitude,slong,6);
    sprintf(mess,"Lat  : %s",slat);
    lcd.setCursor(0,1);
    lcd.print(mess);
    sprintf(mess,"Long : %s",slong);
    lcd.setCursor(0,2);
    lcd.print(mess);
    sprintf(mess,"Alt : %d m",(int)gpsaltitude);
    lcd.setCursor(0,3);
    lcd.print(mess);
    delay(250);
  }


  if (eventc or event1 or event2)
  {
    //    now= RTC.now();
    char eventmess[8];
    //    latitude = -999;
    //    longitude = -999;
    //    gpsaltitude = -999;
    getGPSInfo();
    sprintf(eventmess,"%1i,%1i,%1i",(event1)?1:0,(event2)?1:0,(eventc)?1:0);
    event1 = false;
    event2 = false;
    eventc = false;
    temperature = bmp085GetTemperature(bmp085ReadUT());
    pressure = bmp085GetPressure(bmp085ReadUP());
    if (selmenu==MENU_ACQ)displaycount();
    // write to file 

    ////      sprintf(dataSDmess,"%d,%d,%d,%d,%d,%d,%d,%d,%ld,%ld,%ld",now.year(),now.month()
    ////        ,now.day(),now.hour()
    ////        ,now.minute(),now.second()
    ////        ,(int)temperature,(int)pressure,countPulse1,countPulse2,countCoinc);
    //      sprintf(dataSDmess,"%s,%s,%s,%d,%d,%d,%s,%ld,%ld,%ld",ascdate,slatitude,slongitude,gpsaltitude,
    //        (int)temperature,(int)pressure,eventmess,countPulse1,countPulse2,countCoinc);
    char slat[30];
    char slong[30];
    float2string(latitude,slat,6);
    float2string(longitude,slong,6);
    sprintf(datamess,"DATA %4d,%02d,%02d,%02d,%02d,%02d.%02d,(%s),%lu,%s,%s,%d,%d.%d,%lu,%s,%ld,%ld,%ld"
      ,lannee,lemois,lejour,lheure,laminute,laseconde,lescentiemes,lestatus,lesmillis,
    slat,slong,
    (int)gpsaltitude,
    (int)((float)temperature/10.),(temperature<0)?(int)((-temperature)%10):(int)(temperature%10),
    (long)pressure,eventmess,countPulse1,countPulse2,countCoinc);
    if (dataFile) 
    {
      dataFile.println(datamess+5); // suppress 'DATA ' keyword in mess
      dataFile.flush();
    }
    if(serialcom)Serial.println(datamess);
  }
  if (gpsFile)
     {
      char slat[30];
      char slong[30];
      float2string(latitude,slat,6);
      float2string(longitude,slong,6);
      sprintf(datamess,"%4d,%02d,%02d,%02d,%02d,%02d.%02d,(%s),%lu,%s,%s,%d,%d.%d,%lu"
        ,lannee,lemois,lejour,lheure,laminute,laseconde,lescentiemes,lestatus,lesmillis,
      slat,slong,
      (int)gpsaltitude,
      (int)((float)temperature/10.),(temperature<0)?(int)((-temperature)%10):(int)(temperature%10),
      (long)pressure);
      gpsFile.println(datamess);       
      gpsFile.flush();
     }

  DateTime now = RTC.now();
  /*  Serial.print(now.year(), DEC);
   Serial.print('/');
   Serial.print(now.month(), DEC);
   Serial.print('/');
   Serial.print(now.day(), DEC);
   Serial.print(' ');
   Serial.print(now.hour(), DEC);
   Serial.print(':');
   Serial.print(now.minute(), DEC);
   Serial.print(':');
   Serial.print(now.second(), DEC);
   Serial.println();
   Serial.print(" since 1970 = ");
   Serial.print(now.unixtime());
   Serial.print("s = ");
   Serial.print(now.unixtime() / 86400L);
   Serial.println("d"); 
   */
}

void gestionINT01() {
  countPulse1 += 1 ;
  event1 = true;
  if(digitalRead(COINC_LINE))
  {
    countCoinc +=1;
    eventc = true;
  }
}

void gestionINT02() {
  countPulse2 +=1;
  event2 = true;
}


// Stores all of the bmp085's calibration values into global variables
// Calibration values are required to calculate temp and pressure
// This function should be called at the beginning of the program
void bmp085Calibration()
{
  ac1 = bmp085ReadInt(0xAA);
  ac2 = bmp085ReadInt(0xAC);
  ac3 = bmp085ReadInt(0xAE);
  ac4 = bmp085ReadInt(0xB0);
  ac5 = bmp085ReadInt(0xB2);
  ac6 = bmp085ReadInt(0xB4);
  b1 = bmp085ReadInt(0xB6);
  b2 = bmp085ReadInt(0xB8);
  mb = bmp085ReadInt(0xBA);
  mc = bmp085ReadInt(0xBC);
  md = bmp085ReadInt(0xBE);
}

// Calculate temperature given ut.
// Value returned will be in units of 0.1 deg C

short bmp085GetTemperature(unsigned int ut)
{
  long x1, x2;

  x1 = (((long)ut - (long)ac6)*(long)ac5) >> 15;
  x2 = ((long)mc << 11)/(x1 + md);
  b5 = x1 + x2;

  return ((b5 + 8)>>4);  
}

// Calculate pressure given up
// calibration values must be known
// b5 is also required so bmp085GetTemperature(...) must be called first.
// Value returned will be pressure in units of Pa.
long bmp085GetPressure(unsigned long up)
{
  long x1, x2, x3, b3, b6, p;
  unsigned long b4, b7;

  b6 = b5 - 4000;
  // Calculate B3
  x1 = (b2 * (b6 * b6)>>12)>>11;
  x2 = (ac2 * b6)>>11;
  x3 = x1 + x2;
  b3 = (((((long)ac1)*4 + x3)<<OSS) + 2)>>2;

  // Calculate B4
  x1 = (ac3 * b6)>>13;
  x2 = (b1 * ((b6 * b6)>>12))>>16;
  x3 = ((x1 + x2) + 2)>>2;
  b4 = (ac4 * (unsigned long)(x3 + 32768))>>15;

  b7 = ((unsigned long)(up - b3) * (50000>>OSS));
  if (b7 < 0x80000000)
    p = (b7<<1)/b4;
  else
    p = (b7/b4)<<1;

  x1 = (p>>8) * (p>>8);
  x1 = (x1 * 3038)>>16;
  x2 = (-7357 * p)>>16;
  p += (x1 + x2 + 3791)>>4;

  return p;
}

// Read 1 byte from the BMP085 at 'address'


char bmp085Read(unsigned char address)
{
  unsigned char data;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 1);
  while(!Wire.available());

  return Wire.read();
}

// Read 2 bytes from the BMP085
// First byte will be from 'address'
// Second byte will be from 'address'+1
int bmp085ReadInt(unsigned char address)
{
  unsigned char msb, lsb;

  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(BMP085_ADDRESS, 2);
  while(Wire.available()<2);
  msb = Wire.read();
  lsb = Wire.read();

  return (int) msb<<8 | lsb;
}

// Read the uncompensated temperature value
unsigned int bmp085ReadUT()
{
  unsigned int ut;

  // Write 0x2E into Register 0xF4
  // This requests a temperature reading
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x2E);
  Wire.endTransmission();

  // Wait at least 4.5ms
  delay(5);

  // Read two bytes from registers 0xF6 and 0xF7
  ut = bmp085ReadInt(0xF6);
  return ut;
}

// Read the uncompensated pressure value

unsigned long bmp085ReadUP()
{
  unsigned char msb, lsb, xlsb;
  unsigned long up = 0;

  // Write 0x34+(OSS<<6) into register 0xF4
  // Request a pressure reading w/ oversampling setting
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF4);
  Wire.write(0x34 + (OSS<<6));
  Wire.endTransmission();

  // Wait for conversion, delay time dependent on OSS
  delay(2 + (3<<OSS));

  // Read register 0xF6 (MSB), 0xF7 (LSB), and 0xF8 (XLSB)
  Wire.beginTransmission(BMP085_ADDRESS);
  Wire.write(0xF6);
  Wire.endTransmission();
  Wire.requestFrom(BMP085_ADDRESS, 3);

  // Wait for data to become available
  while(Wire.available() < 3)
    ;
  msb = Wire.read();
  lsb = Wire.read();
  xlsb = Wire.read();

  up = (((unsigned long) msb << 16) | ((unsigned long) lsb << 8) | (unsigned long) xlsb) >> (8-OSS);

  return up;
}


void serialEvent() 
{
  int i;
  i = 0;
  if(!serialcom)serialcom=true;
  while (Serial.available()) 
  {
    // get the new byte:
    delay(1);
    char inChar = (char)Serial.read();
    // add it to the inputString:
    serialbuffer[i] = tolower(inChar);
    // if the incoming character is a newline, set a flag
    // so the main loop can do something about it:

    i = i + 1;
    if (inChar == '\n') 
    {
      stringComplete = true;
      serialbuffer[i-1]=0;
      if (serialbuffer[i-2]=='\r')
        serialbuffer[i-2]=0;
    }
  }
}






