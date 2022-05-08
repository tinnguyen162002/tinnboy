#include <DS3231.h>
#include <SparkFunSi4703.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>

MCUFRIEND_kbv tft;       // hard-wired for UNO shields anyway.

#define LCD_CS A3 // Chip Select goes to Analog 3
#define LCD_CD A2 // Command/Data goes to Analog 2
#define LCD_WR A1 // LCD Write goes to Analog 1
#define LCD_RD A0 // LCD Read goes to Analog 0

#define rtcAddress 0x68
#define SI4703Address 0x57

int resetRadio = 2;
int SDIO = A4;
int SCLK = A5;

uint8_t YP = A1;  // must be an analog pin, use "An" notation!
uint8_t XM = A2;  // must be an analog pin, use "An" notation!
uint8_t YM = 7;   // can be a digital pin
uint8_t XP = 6;   // can be a digital pin

uint16_t TS_LEFT = 950;
uint16_t TS_RT  = 170;
uint16_t TS_TOP = 950;
uint16_t TS_BOT = 170;

#define SENSITIVITY 100
#define MINPRESSURE 10
#define MAXPRESSURE 1000

uint16_t identifier;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, SENSITIVITY);
TSPoint tp;

// Assign human-readable names to some common 16-bit color values:
#define BLACK   0x0000
#define BLUE    0x0009
#define LBLUE   0x057F
#define L2BLUE  0x06FF
#define RED     0x7000
#define ORANGE  0xFC00
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0x4008
#define YELLOW  0xFFE0
#define WHITE   0xFFFF
#define GRAY    0x0861
#define LGRAY   0x8410
#define GREEN2  0x0180

int page = 0;
bool blueON = false;

DS3231 Clock;
bool Century = false;
bool h12;
bool PM;
float temperature;

int Seconds = 57;
int Minutes = 15;
int Hours = 23;

int Mday = 19;
int Month = 10;
int Year = 18;

bool tone1 = false;
int alarm = 0;
bool alarmon = false;
bool alarmset = false;
int alarmHours = 8;
int alarmMinutes = 10;
bool radio_al = true;
bool tone_al = false;
int setHours = 0;
int setMinutes = 0;
int setSeconds = 0;
int setMonthDay = 1;
int setMonth = 1;
int setYear = 18;
bool disekto = false;

bool dateselected = false;
bool timeselected = true;
bool clock_pressed = false;

Si4703_Breakout radio(resetRadio, SDIO, SCLK);   // Start radio
bool fmon = false;
int fmTimer = 0;
int volume = 6;
int channel = 1071 ;
int channel1;
int channel2;
int mstation = 0;

unsigned long duration;

Adafruit_GFX_Button buttons[15];

void setup(void)
{
  Wire.begin();
  tft.reset();
  identifier = tft.readID();
  tft.begin(identifier);
  tft.setRotation(3);
  /*Clock.setSecond(50);//Set the second 
  Clock.setMinute(05);//Set the minute 
  Clock.setHour(12);  //Set the hour 
  Clock.setDoW(1);    //Set the day of the week
  Clock.setDate(20);  //Set the date of the month
  Clock.setMonth(10);  //Set the month of the year
  Clock.setYear(18);  //Set the year (Last two digits of the year)*/
  pinMode(1, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(2, OUTPUT);
  digitalWrite(1, LOW);
  delay(50);
  digitalWrite(1, HIGH);
  tft.fillScreen(BLACK);
  tft.setTextSize(3);
  tft.setTextColor(LBLUE);
  tft.setCursor(50, 80);
  tft.print("version 29.10.18");
  tft.setCursor(70, 150);
  tft.print("Yiannis Dou");
  delay(3000);
  
  tft.fillScreen(BLACK);
  set_rtc();
  Homepage();
  
}

void loop(void)
{
  
//update time and date
  if (Seconds != Clock.getSecond())
  {
    set_rtc();
    
    if (page == 0 || page == 1 || page ==4)
    {
      Clockpage();
    }
    else if (page == 5)
    {
      Nightpage();
    }
   }

  if (alarmHours == Hours && alarmMinutes == Minutes && Seconds == 0 && alarm == 1)
  {
    if (radio_al == true )
    {
      if (blueON == true)
      {
      blueON = false;
      digitalWrite(1, LOW);
      delay(50);
      digitalWrite(1, HIGH);
      }
      tft.fillScreen(BLACK);
      Nightpage();
      fmon = true;
      volume = 15;
      RadioPowerOn();
      alarm = 2;
      
    }
    else if (tone_al == true )
    {
      tone1 = true;
      alarm = 2;
      tft.fillScreen(BLACK);
      Nightpage();
    }
  }

  if (tone1 == true)
  {
    playtone();
  }

   // Retrieve a point
  tp = ts.getPoint();
  pinMode(YP, OUTPUT);      //restore shared pins
  pinMode(XM, OUTPUT);

  // See if there's any  touch data for us
  if (tp.z > MINPRESSURE && tp.z < MAXPRESSURE)
  {
    tp.x = map(tp.x, TS_LEFT, TS_RT, tft.height(), 0);
    tp.y = map(tp.y, TS_BOT, TS_TOP, tft.width(), 0);
    int y = tp.x;
    int x = tp.y;



    if ((x > 345) && (x < 400))
    {
      if ((y > 0) && (y < 50))
      {
        delay(100);
        if (page == 0 && fmon == false)         //In home page Blue pressed,fm must be off
        {
          if (blueON == true)
          {
           Bluemode(); 
          }
          else
          {          
          blueON = true;         
          digitalWrite(1, LOW);
          delay(50);
          digitalWrite(1, HIGH);
          Bluemode();
          }
        }
        else if (page == 1)         //In blue page Blue pressed
        {
          blueON = false;
          digitalWrite(1, LOW);
          delay(50);
          digitalWrite(1, HIGH);
          Homepage();               
        }
        else if (page == 4 && mstation < 10)         //In fm page m+ pressed
        {
          mstation++;
          memoryStation();
        }
      }
    }


    if ((x > 345) && (x < 400))
    {
      if ((y > 60) && (y < 110))
      {
        delay(100);
        if (page == 0)
        {
          alarmset = true;
          Alarm();             //In home page Alarm pressed
        }
        else if (page == 2)
        {
          alarmset = false;
          Homepage();         //In alarm page Alarm pressed
        }
        else if (page == 4)     //In fm page m- pressed
        {
          if (mstation > 0)
          {
           mstation--;
          }
          else if (mstation = 0)
          { 
            mstation = 0;
          }
          memoryStation();
        }
        
      }
    }


    if ((x > 15) && (x < 70))
    {
      if ((y > 70) && (y < 125))
      {
        delay(100);
        if (page == 0 || page == 1 || page == 2 || page == 4 || page == 5)      //In various pages Alarm on/off pressed
        {
          if (alarmon == false)
          {
            alarmon = true;
            alarm = 1;
            Alarm_bt2();             
          }
          else if (alarmon == true)
          {
            alarmon = false;
            alarm = 0;
            Alarm_bt2();
          }
        }
      }
    }


    if ((x > 110) && (x < 190))
    {
      if ((y > 120) && (y < 160))
      {
        delay(100);
        if (page == 2)              //In Alarm page type of alarm  selection, radio pressed
        {
          if (radio_al == true)
          {
           radio_al = false;
           tone_al = true;
           radio_tone();
          }
          else if (radio_al == false)
          {
            radio_al = true;
            tone_al = false;
            radio_tone();
          }
         }
       }
    }

    if ((x > 220) && (x < 300))
    {
      if ((y > 120) && (y < 160))
      {
        delay(100);
        if (page == 2)              //In Alarm page type of alarm  selection, tone pressed
        {
          if (tone_al == true)
          {
           tone_al = false;
           radio_al = true;
           radio_tone();
          }
          else if (tone_al == false)
          {
            tone_al = true;
            radio_al = false;
            radio_tone();
          }
         }
       }
    }


    if ((x > 345) && (x < 400))
    {
      if ((y > 120) && (y < 170))
      {
        delay(100);
        if (page == 0)              //In home page Clock bt pressed
        {
          clock_pressed = true;
          Clockset();            
        }
        else if (page == 3)         //In clockset page Clock bt pressed
        {
          clock_pressed = false;
          Homepage();            
        }
        else if (page == 4 && fmon == true)   //in fm page clock pressed
        {
         Homepage();
        }
        else if (page == 1 && blueON == true)   //in blue page clock pressed
        {
         Homepage();
        }
      }
    }



    if ((x > 345) && (x < 400))
    {
      if ((y > 180) && (y < 240))
      {
        delay(100);
        if (page == 0 && blueON == false)       //In home page Fm pressed, bluetooth must be off
        {
          if (fmon == true)
          {
            Fmradio();
          }
          else 
          {
          fmon = true;
          RadioPowerOn();
          Fmradio();          
          }
        }
        else if (page == 4)           //In fm page Fm pressed
        {
          fmon = false;
          digitalWrite(2, LOW);
          delay(100);
          digitalWrite(2, HIGH);
          
          Homepage();               
        }
      }
    }


    if ((x > 0) && (x < 60))              //fm seek down
    {
      if ((y > 190) && (y < 240))
      {
        delay(100);
        if (page == 4)
        {
          channel = radio.seekDown();
          printStation();
        }
      }
    }


    if ((x > 110) && (x < 170))           //fm seek up
    {
      if ((y > 190) && (y < 240))
      {
        delay(100);
        if (page == 4)
        {
          channel = radio.seekUp();
          printStation();
        }
      }
    }


    if ((x > 280) && (x < 335))               //fm volume up
    {
      if ((y > 190) && (y < 240))
      {
        delay(100);
        if (page == 4)
        {
          volume++;
          if (volume > 15)
          {
            volume = 15;
          }
          radio.setVolume(volume);
          printVolume();
        }
      }
    }




    if ((x > 180) && (x < 230))           //fm volume down
    {
      if ((y > 190) && (y < 240))
      {
        delay(100);
        if (page == 4)
        {
          volume--;
          if (volume < 0)
          {
            volume = 0;
          }
          radio.setVolume(volume);
          printVolume();
        }
      }
    }


if (page == 3)                                //select time or date to set in clockset page
{
 if ((x > 30) && (x < 250))
    {
      if ((y > 65) && (y < 115))
      {
        delay(100);
        timeselected = true; 
        dateselected = false;
        Clockset();
      }
      else if ((y > 125) && (y < 175))
      {
       delay(100);
       timeselected = false; 
       dateselected = true;
       Clockset(); 
      }
    }
}

 if ((x > 40) && (x < 90))
    {
      if ((y > 0) && (y < 55))
      {
        delay(100);
        if (page == 3 && timeselected == true)    // increase hours
        {
          setHours++;
          if (setHours == 24)
          {
            setHours = 0;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(50, 80);
  
          if (setHours < 10)
          {
            tft.print("0");
          }
          tft.print(setHours);
          Clock.setHour(setHours);
        }

        else if (page == 3 && dateselected == true)    // increase setMonthDayday
        {
          setMonthDay++;
          if (setMonth == 4 || setMonth == 6 || setMonth == 9 || setMonth == 11)
          {
            if (setMonthDay == 31)
            {
              setMonthDay = 1;
            }
          }
          else if (setMonth == 2 && disekto == true)
          {
            if (setMonthDay == 30)
            {
              setMonthDay = 1;
            }
            Clock.setDate(setMonthDay);
          }

          else if (setMonth == 2 && disekto == false)
          {
            if (setMonthDay == 29)
            {
              setMonthDay = 1;
            }
          }
          else if (setMonthDay == 32)
          {
            setMonthDay = 1;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(50, 135);
          if (setMonthDay < 10)
          {
            tft.print("0");
          }
          tft.print(setMonthDay);
          Clock.setDate(setMonthDay);
        }
      }
    }

    if ((x > 40) && (x < 90))
    {
      if ((y > 185) && (y < 240))
      {
        delay(100);
        if (page == 3 && timeselected == true)    // decrease sethours
        {
          if (setHours > 0)
          {
           setHours--;
          }
          else if (setHours == 0)
          {
           setHours = 23;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(50, 80);
  
          if (setHours < 10)
          {
            tft.print("0");
          }
          tft.print(setHours);
          Clock.setHour(setHours);
        }

        
        else if (page == 3 && dateselected == true)    // decrease setMonthDay
        {
         if (setMonthDay > 0)
          {
            setMonthDay--;
          }
          if (setMonthDay == 0)
          {
            if (setMonth == 4 || setMonth == 6 || setMonth == 9 || setMonth == 11)
            {
              setMonthDay = 30;
            }
            else if ((setMonth == 2) && (disekto == true))
            {
              setMonthDay = 29;
            }
            else if ((setMonth == 2) && (disekto == false))
            {
              setMonthDay = 28;
            }
            else if (setMonth == 1 || setMonth == 3 || setMonth == 5 || setMonth == 7 || setMonth == 8 || setMonth == 10 || setMonth == 12)
            {
              setMonthDay = 31;
            }
          }
            tft.setTextColor(LBLUE, GRAY);
            tft.setTextSize(3);
            tft.setCursor(50, 135);
            if (setMonthDay < 10)
            {
             tft.print("0");
            }
            tft.print(setMonthDay);
            Clock.setDate(setMonthDay);
        }
      }
    }


    if ((x > 130) && (x < 180))
    {
      if ((y > 0) && (y < 55))
      {
        delay(100);
        if (page == 3 && timeselected == true)  // increase setMinutes  
        {
          setMinutes++;
          if (setMinutes == 60)
          {
           setMinutes = 0;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(140, 80);
  
          if (setMinutes < 10)
          {
            tft.print("0");
          }
          tft.print(setMinutes);
          Clock.setMinute(setMinutes);
        }

        else if (page == 3 && dateselected == true)  // increase setMonth  
        {
          setMonth++;
          if (setMonth == 13)
          {
            setMonth = 1;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(140, 135);
          if (setMonth < 10)
          {
           tft.print("0");
          }
          tft.print(setMonth);
          Clock.setMonth(setMonth);
        }

        else if (page == 2)    // increase alarmhours
        {
          alarmHours++;
          if (alarmHours == 24)
          {
            alarmHours = 0;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(140, 80);
  
          if (alarmHours < 10)
          {
            tft.print("0");
          }
          tft.print(alarmHours);
        }
      }
    }

    if ((x > 130) && (x < 180))
    {
      if ((y > 185) && (y < 240))
      {
        delay(100);
        if (page ==3 && timeselected == true)    //decrease setMinutes
        {
          if (setMinutes > 0)
          {
           setMinutes--;
          }
          else if (setMinutes == 0)
          {
           setMinutes = 59;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(140, 80);
  
          if (setMinutes < 10)
          {
            tft.print("0");
          }
          tft.print(setMinutes);
          Clock.setMinute(setMinutes);
        }

        else if (page ==3 && dateselected == true)    //decrease setMonth
        {
          if (setMonth > 0)
          {
            setMonth--;
          }
          if (setMonth == 0)
          {
            setMonth = 12;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(140, 135);
          if (setMonth < 10)
          {
           tft.print("0");
          }
          tft.print(setMonth);
          Clock.setMonth(setMonth);
        }

        else if (page == 2)        // decrease alarmhours
        {
          if (alarmHours > 0)
          {
           alarmHours--;
          }
          else if (alarmHours == 0)
          {
           alarmHours = 23;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(140, 80);
  
          if (alarmHours < 10)
          {
            tft.print("0");
          }
          tft.print(alarmHours);
        }
        
      }
    }

if ((x > 220) && (x < 270))
    {
      if ((y > 0) && (y < 55))
      {
        delay(100);
        if (page ==3 && timeselected == true)    //increase setSeconds
        {
          setSeconds++;
          if (setSeconds == 60)
          {
           setSeconds = 0;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(230, 80);
  
          if (setSeconds < 10)
          {
            tft.print("0");
          }
          tft.print(setSeconds);
          Clock.setSecond(setSeconds);
        }

        else if (page == 3 && dateselected == true)  // increase setYear 
        {
          setYear++;
          if (setYear % 4 == 0)
          {
            disekto = true;
          }
          else
          {
            disekto = false;
          }
          if (setYear == 100)
          {
            setYear = 0;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(230, 135);
          tft.print("20");
          if (setYear < 10)
          {
           tft.print("0");
          }
          tft.print(setYear);
          Clock.setYear(setYear);
        }

        else if (page == 2)                // increase alarmMinutes  
        {
          alarmMinutes++;
          if (alarmMinutes == 60)
          {
           alarmMinutes = 0;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(230, 80);
  
          if (alarmMinutes < 10)
          {
            tft.print("0");
          }
          tft.print(alarmMinutes);
        }
      }
    }



   if ((x > 220) && (x < 270))
    {
      if ((y > 185) && (y < 240))
      {
        delay(100);
        if (page == 3 && timeselected == true)    //decrease setSeconds 
        {
         if (setSeconds > 0)
          {
           setSeconds--;
          }
          else if (setSeconds == 0)
          {
           setSeconds = 59;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(230, 80);
  
          if (setSeconds < 10)
          {
            tft.print("0");
          }
          tft.print(setSeconds);
          Clock.setSecond(setSeconds);
        }

      else if (page == 3 && dateselected == true)  // decrease setYear
        {
          delay(100);
          if (setYear > 0)
          {
            setYear--;
          }
          else if (setYear == 0)
          {
            setYear = 99;
          }
          if (setYear % 4 == 0)
          {
            disekto = true;
          }
          else
          {
            disekto = false;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(230, 135);
          tft.print("20");
          if (setYear < 10)
          {
           tft.print("0");
          }
          tft.print(setYear);
          Clock.setYear(setYear);
        }

        else if (page == 2)                //decrease alarmMinutes
        {
          if (alarmMinutes > 0)
          {
           alarmMinutes--;
          }
          else if (alarmMinutes == 0)
          {
           alarmMinutes = 59;
          }
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(3);
          tft.setCursor(230, 80);
  
          if (alarmMinutes < 10)
          {
            tft.print("0");
          }
          tft.print(alarmMinutes);
        }
      }
    }







if ((x > 80) && (x < 250))                  //touch on time area in different pages
    {
      if ((y > 70) && (y < 120))
      {
        if (page == 0)
        {
         tft.fillScreen(BLACK);
         Nightpage();
        }
        else if (page == 5)
        {
         tft.fillScreen(BLACK);
         Homepage();
         alarm = 1;
        }
      }
    }

    if ((x > 80) && (x < 250))
    {
      if ((y > 140) && (y < 200))
      {
        if (page == 5)
        {
        delay(100);
        alarm = 1;
        tone1 = false;
        tft.fillScreen(BLACK);
        Homepage();
        }
      }
    }
  }
}

void set_rtc()
{
  int temperature;
  Seconds = Clock.getSecond();
  Minutes = Clock.getMinute();
  Hours = Clock.getHour(h12, PM);
  Mday = Clock.getDate();
  Month = Clock.getMonth(Century);
  Year = Clock.getYear();
  temperature = Clock.getTemperature();
}

void Homepage()
{
  page = 0;
  set_rtc();
  tft.setTextColor(LBLUE, GRAY);
  tft.setTextSize(2);
  Firstframe();
  Secondframe();
  Thirdframe();
  tft.setCursor(20, 20);
  if (blueON == true)
  {
   tft.print("Bluetooth ON"); 
  }
  else if (fmon == true)
  {
   tft.setCursor(30, 20);
   tft.print("FM Radio");
   printStation();
   printVolume();
  }  
  else if (blueON ==false && fmon ==false)
  {
   tft.print("Bluetooth OFF - Radio OFF");
  }
  Clockpage();
  Blue_bt();
  Alarm_bt();
  Alarm_bt2();
  clock_bt();
  FM_bt();
}

void Clockpage()
{
  tft.setTextColor(LBLUE, GRAY);
  printTime();
  tft.setTextColor(LGRAY, GRAY);
  tft.setTextSize(2);
  tft.setCursor(220, 140);
  if (Mday < 10)
  {
    tft.print("0");
  }
  tft.print(Mday);
  tft.print(".");
  if (Month < 10)
  {
    tft.print("0");
  }
  tft.print(Month);
  tft.print(".");
  if (Year < 10)
  {
    tft.print("0");
  }
  tft.print(Year);
  Alarm_bt2();
  tft.setCursor(30, 140);
  tft.setTextColor(LGRAY, GRAY);
  tft.print("T:");

  tft.print(Clock.getTemperature(), 1);
  tft.print("  C");
  tft.drawCircle(118, 140, 2, LGRAY);

  if (page == 0 || page == 1)
  {
    if (alarmon == true)
    {
    
    tft.setCursor(30, 205);
    tft.setTextColor(LBLUE, GRAY);
    tft.print("Alarm ON ");
    if (alarmHours < 10)
    {
    tft.print("0");
    }
    tft.print(alarmHours);
    tft.print(":");
    if (alarmMinutes < 10)
    {
    tft.print("0");
    }
    tft.print(alarmMinutes);
    tft.print("           ");
    }
     else 
    {
     Show_developer();
    }
  }
}

void printTime()
{  
  tft.setTextSize(5);
  tft.setCursor(100, 85);
  if (Hours < 10)
  {
    tft.print("0");
  }
  tft.print(Hours);
  tft.print(":");
  if (Minutes < 10)
  {
    tft.print("0");
  }
  tft.print(Minutes);
  tft.drawCircle(300, 100, 20, LGRAY);
  tft.drawCircle(300, 100, 21, LGRAY);
  tft.drawCircle(300, 100, 22, LGRAY);
  tft.setTextColor(LBLUE, GRAY);
  tft.setTextSize(2);
  tft.setCursor(290, 93);
  if (Seconds < 10)
  {
    tft.print("0");
  }
  tft.print(Seconds);
}

void Show_developer()
{
  tft.setCursor(30, 205);
  tft.setTextColor(LBLUE, GRAY);
  tft.print("ver 29.10.18 yiannisdou");
}


void Bluemode()
{
  page = 1;
  Firstframe();
  tft.setTextSize(2);
  tft.setTextColor(LBLUE, GRAY);
  tft.setCursor(30, 20);
  tft.print("Bluetooth ON");
  Blue_bt();

  delay(1000);
  tft.setTextColor(LBLUE, GRAY);
  tft.setCursor(30, 20);
  tft.print("Bluetooth Ready...");
  //delay(1000);
 // tft.setTextColor(LBLUE, GRAY);
  //tft.setCursor(30, 20);
 //tft.print("Bluetooth Connected");
}

void Alarm()
{
  page = 2;
  Secondframe();
  Alarm_bt();
  Alarm_bt2();
  tft.setTextColor(LBLUE, GRAY);
  tft.setCursor(20, 15);
  tft.setTextSize(2);
  tft.print("Alarm");
  tft.setTextColor(LBLUE);
  tft.setTextSize(3);
  tft.setCursor(140, 80);
  if (alarmHours < 10)
  {
    tft.print("0");
  }
  tft.print(alarmHours);
  tft.setCursor(200, 80);
  tft.print(":");
  tft.setCursor(230, 80);
  if (alarmMinutes < 10)
  {
    tft.print("0");
  }
  tft.print(alarmMinutes);
    
  tft.fillRoundRect(0, 0, 340, 55, 27, GRAY);     //1st frame
  tft.drawRoundRect(2, 2, 336, 51, 27, LGRAY);
  tft.setTextColor(LBLUE);
  tft.setCursor(20, 15);
  
  tft.print("Alarm");
  tft.fillCircle(157, 26, 22, RED);
  tft.fillCircle(247, 26, 22, RED);
  tft.setCursor(150, 17);
  tft.print("+    +");

  tft.fillRoundRect(0, 185, 340, 55, 27, GRAY);   //3rd frame
  tft.drawRoundRect(2, 187, 336, 51, 27, LGRAY);
  tft.fillCircle(157, 211, 22, RED);
  tft.fillCircle(247, 211, 22, RED);
  tft.setCursor(60, 203);
  tft.print("     -    -");
  radio_tone();
}

void radio_tone()
{
  tft.setTextSize(2);
  if (radio_al == false)
  {
  tft.setTextColor(LGRAY, GRAY);
  tft.drawRoundRect(110, 120, 80, 40, 10, LGRAY);
  tft.setCursor(120, 132);
  tft.print("Radio");
  tft.drawRoundRect(220, 120, 80, 40, 10, LBLUE);
  tft.setTextColor(LBLUE, GRAY);
  tft.setCursor(235, 132);
  tft.print("Tone");
  }
  else if (radio_al == true)
  {
  tft.setTextColor(LBLUE, GRAY);
  tft.drawRoundRect(110, 120, 80, 40, 10, LBLUE);
  tft.setCursor(120, 132);
  tft.print("Radio");
  tft.drawRoundRect(220, 120, 80, 40, 10, LGRAY);
  tft.setTextColor(LGRAY, GRAY);
  tft.setCursor(235, 132);
  tft.print("Tone");
  }
}


void Clockset()
{
  page = 3;
  clock_bt();
  tft.setTextSize(3);
  tft.setTextColor(LGRAY);
  tft.fillRoundRect(0, 0, 340, 55, 27, GRAY);     //1st frame
  tft.drawRoundRect(2, 2, 336, 51, 27, LGRAY);
  tft.fillCircle(67, 26, 22, RED);
  tft.fillCircle(157, 26, 22, RED);
  tft.fillCircle(247, 26, 22, RED);
  tft.setCursor(60, 15);
  tft.print("+    +    +");


  tft.fillRoundRect(0, 65, 340, 110, 27, GRAY);   //2nd frame
  tft.drawRoundRect(2, 67, 336, 106, 27, LGRAY);

  setHours = Hours;
  setMinutes = Minutes;
  setSeconds = Seconds;
  setMonthDay = Mday;
  setMonth = Month;
  setYear = Year;
  tft.fillRoundRect(0, 185, 340, 55, 27, GRAY);   //3rd frame
  tft.drawRoundRect(2, 187, 336, 51, 27, LGRAY);
  tft.fillCircle(67, 211, 22, RED);
  tft.fillCircle(157, 211, 22, RED);
  tft.fillCircle(247, 211, 22, RED);
  tft.setCursor(60, 200);
  tft.print("-    -    -");

  if (timeselected == true)
  {
  tft.setTextColor(LBLUE, GRAY);
  }
  else if (timeselected == false)
  {
   tft.setTextColor(WHITE, GRAY); 
  }
  tft.setCursor(50, 80);
  if (setHours < 10)
  {
    tft.print("0");
  }
  tft.print(setHours);
  tft.setCursor(100, 80);
  tft.print(":");
  tft.setCursor(140, 80);
  if (setMinutes < 10)
  {
    tft.print("0");
  }
  tft.print(setMinutes);
  tft.setCursor(200, 80);
  tft.print(":");
  tft.setCursor(230, 80);
  if (setSeconds < 10)
  {
    tft.print("0");
  }
  tft.print(setSeconds);

  if (dateselected == true)
  {
  tft.setTextColor(LBLUE, GRAY);
  }
  else if (dateselected == false)
  {
   tft.setTextColor(WHITE, GRAY); 
  }
  tft.setTextSize(3);
  tft.setCursor(50, 135);
  if (setMonthDay < 10)
  {
    tft.print("0");
  }
  tft.print(setMonthDay);
  tft.setCursor(100, 135);
  tft.print("-");
  tft.setCursor(140, 135);
  if (setMonth < 10)
  {
    tft.print("0");
  }
  tft.print(setMonth);
  tft.setCursor(200, 135);
  tft.print("-");
  tft.setCursor(230, 135);
  tft.print("20");
  if (setYear < 10)
  {
    tft.print("0");
  }
  tft.print(setYear);
}

void Fmradio()
{
  page = 4;
  FM_bt();
  Firstframe();
  tft.setTextColor(LBLUE, GRAY);
  tft.setCursor(30, 20);
  tft.print("FM Radio");
  printStation();
  printVolume();
  tft.setTextColor(LBLUE, MAGENTA);
  tft.setTextSize(2);
  tft.fillCircle(370, 30, 25, MAGENTA);
  tft.drawCircle(370, 30, 23, LGRAY);
  tft.drawCircle(370, 30, 22, LGRAY);
  tft.setCursor(360, 25);
  tft.print("M+");
  tft.fillCircle(370, 90, 25, MAGENTA);
  tft.drawCircle(370, 90, 23, LGRAY);
  tft.drawCircle(370, 90, 22, LGRAY);
  tft.setCursor(360, 85);
  tft.print("M-");
  Secondframe();
  Clockpage();
  tft.fillRoundRect(0, 185, 340, 55, 27, GRAY);
  tft.fillRoundRect(0, 185, 165, 55, 27, GRAY);       //3rd half1
  tft.drawRoundRect(2, 187, 161, 51, 27, LGRAY);
  tft.drawCircle(26, 213, 24, LGRAY);
  tft.drawCircle(139, 213, 24, LGRAY);
  tft.fillRoundRect(175, 185, 165, 55, 27, GRAY);       //3rd half2
  tft.drawRoundRect(177, 187, 161, 51, 27, LGRAY);
  tft.drawCircle(201, 213, 24, LGRAY);
  tft.drawCircle(314, 213, 24, LGRAY);
  tft.setTextSize(3);
  tft.setTextColor(LBLUE);
  tft.setCursor(15, 202);
  tft.print("<");
  tft.setCursor(130, 202);
  tft.print(">");
  tft.setCursor(195, 202);
  tft.print("-");
  tft.setCursor(305, 202);
  tft.print("+");

  tft.setTextSize(2);
  tft.setCursor(70, 205);
  tft.print("CH");
  tft.setCursor(240, 205);
  tft.print("VOL");
}

void RadioPowerOn()
{
    radio.powerOn();
    delay(500);
    radio.setChannel(channel);
    delay(100);
    radio.setVolume(volume);
    delay(100);
    digitalWrite(2, LOW);
    delay(500);
    digitalWrite(2, HIGH);
    delay(100);
    radio.powerOn();
    delay(500);
    radio.setChannel(channel);
    delay(100);
    radio.setVolume(volume);  
}

void Firstframe()
{
  tft.fillRoundRect(0, 0, 340, 55, 27, GRAY);
  tft.drawRoundRect(2, 2, 336, 51, 27, LGRAY);
}

void Secondframe()
{
  tft.fillRoundRect(0, 65, 340, 110, 27, GRAY);
  tft.drawRoundRect(2, 67, 336, 106, 27, LGRAY);
}

void Thirdframe()
{
  tft.fillRoundRect(0, 185, 340, 55, 27, GRAY);
  tft.drawRoundRect(2, 187, 336, 51, 27, LGRAY);
}

void Blue_bt()                      //BLUETOOTH
{
  tft.fillCircle(370, 30, 25, BLUE);
  if (blueON == true)
  {
    tft.drawTriangle(370, 30, 370, 15, 380, 23, L2BLUE);
    tft.drawTriangle(370, 30, 370, 45, 380, 37, L2BLUE);
    tft.drawLine(370, 30, 360, 23, L2BLUE);
    tft.drawLine(370, 30, 360, 37, L2BLUE);
  }
  else if (blueON == false)
  {
    tft.drawTriangle(370, 30, 370, 15, 380, 23, LGRAY);
    tft.drawTriangle(370, 30, 370, 45, 380, 37, LGRAY);
    tft.drawLine(370, 30, 360, 23, LGRAY);
    tft.drawLine(370, 30, 360, 37, LGRAY);
  }
  tft.drawCircle(370, 30, 23, LGRAY);
  tft.drawCircle(370, 30, 22, LGRAY);
}


void Alarm_bt()
{
  tft.fillCircle(370, 90, 25, GREEN2);                   //MEDIA
  tft.drawCircle(370, 90, 23, LGRAY);
  tft.drawCircle(370, 90, 22, LGRAY);
  if (alarmset == true)
  {
    tft.setTextColor(L2BLUE, GREEN2);
  }
  else if (alarmset == false)
  {
    tft.setTextColor(LGRAY, GREEN2);
  }
  tft.setTextSize(2);
  tft.setCursor(360, 82);
  tft.print("AL");
}


void Alarm_bt2()
{
  if (alarmon == false)
  {
    tft.drawCircle(40, 100, 20, LGRAY);
    tft.drawCircle(40, 100, 21, LGRAY);
    tft.drawCircle(40, 100, 22, LGRAY);
    tft.setTextColor(LGRAY);
  }
  else if (alarmon == true)
  {
    tft.drawCircle(40, 100, 20, LBLUE);
    tft.drawCircle(40, 100, 21, LBLUE);
    tft.drawCircle(40, 100, 22, LBLUE);
    tft.setTextColor(LBLUE);
  }
  tft.setTextSize(2);
  tft.setCursor(30, 93);
  tft.print("AL");
}

void clock_bt()
{
  tft.fillCircle(370, 150, 25, RED);                      //CLOCK
  tft.drawCircle(370, 150, 23, LGRAY);
  tft.drawCircle(370, 150, 22, LGRAY);
  if (clock_pressed == true)
  {
    tft.drawLine(373, 150, 355, 150, L2BLUE);
    tft.drawLine(370, 153, 370, 132, L2BLUE);
  }
  else if (clock_pressed == false)
  {
    tft.drawLine(373, 150, 355, 150, LGRAY);
    tft.drawLine(370, 153, 370, 132, LGRAY);
  }
}

void FM_bt()
{
  tft.fillCircle(370, 210, 25, MAGENTA);
  tft.drawCircle(370, 210, 23, LGRAY);
  tft.drawCircle(370, 210, 22, LGRAY);
  if (fmon == true)
  {
    tft.setTextColor(L2BLUE, MAGENTA);
  }
  else if (fmon == false)
  {
    tft.setTextColor(LGRAY, MAGENTA);
  }
  tft.setTextSize(2);
  tft.setCursor(360, 202);
  tft.print("FM");
}


void Nightpage()
{
  page = 5;
  
  tft.drawRoundRect(0, 0, 400, 240, 5, LBLUE);
  
  tft.setTextColor(LGRAY, BLACK);
  tft.setTextSize(2);
  tft.setCursor(270, 20);
  if (Mday < 10)
  {
    tft.print("0");
  }
  tft.print(Mday);
  tft.print(".");
  if (Month < 10)
  {
    tft.print("0");
  }
  tft.print(Month);
  tft.print(".");
  if (Year < 10)
  {
    tft.print("0");
  }
  tft.print(Year);
  
  tft.setCursor(20, 20);
  tft.print("T:");

  tft.print(Clock.getTemperature(), 1);
  tft.print("  C");
  tft.drawCircle(108, 20, 2, LGRAY);
  tft.setTextColor(LBLUE, BLACK);
  tft.setTextSize(4);
  printTime();
  Alarm_bt2();

  tft.setCursor(120, 215);
  tft.setTextSize(2);
  if (alarmon == true)
  {
    tft.setCursor(30, 205);
    tft.setTextColor(LBLUE, BLACK);
    tft.print("Alarm ON ");
    if (alarmHours < 10)
    {
    tft.print("0");
    }
    tft.print(alarmHours);
    tft.print(":");
    if (alarmMinutes < 10)
    {
    tft.print("0");
    }
    tft.print(alarmMinutes);
    if (alarm == 2)
    {
      tft.drawRoundRect(70, 140, 200, 55, 10, YELLOW);
      tft.setCursor(100, 155);
      tft.setTextSize(3);
      tft.print("Wake up!!");
    }
  }
   else if (alarmon == false)
    {    
    tft.setCursor(30, 205);
    tft.setTextColor(LGRAY, BLACK);
    tft.print("Alarm Off      ");
    }
}

void memoryStation()
{
  switch (mstation)
  {
  case 0:
    channel = 890;
    radio.setChannel(channel);
    printStation();
    break;
  case 1:
    channel = 897;
    radio.setChannel(channel);
    printStation();
    break;
  case 2:
    channel = 968;
    radio.setChannel(channel);
    printStation();
    break;
  case 3:
    channel = 975;
    radio.setChannel(channel);
    printStation();
    break;
    case 4:
    channel = 990;
    radio.setChannel(channel);
    printStation();
    break;
  case 5:
    channel = 1003;
    radio.setChannel(channel);
    printStation();
    break;
    case 6:
    channel = 1017;
    radio.setChannel(channel);
    printStation();
    break;
  case 7:
    channel = 1047;
    radio.setChannel(channel);
    printStation();
    break;
    case 8:
    channel = 1055;
    radio.setChannel(channel);
    printStation();
    break;
  case 9:
    channel = 1058;
    radio.setChannel(channel);
    printStation();
    break;
    case 10:
    channel = 1071;
    radio.setChannel(channel);
    printStation();
    break;
  }
}

void printStation()
{
          tft.setTextSize(3);
          tft.setCursor(150, 15);
          tft.setTextColor(LBLUE, GRAY);
          channel1 = channel / 10;
          tft.print(channel1);
          tft.print(",");
          channel2 = channel % 10;
          tft.print(channel2);
          if (channel1 < 100)
          {
            tft.print("0");
          }
}

void printVolume()
{
          tft.drawCircle(311, 26, 24, LGRAY);                 //volume circle
          tft.setTextColor(LBLUE, GRAY);
          tft.setTextSize(2);
          tft.setCursor(300, 20);
          if (volume < 10)
          {
            tft.print("0");
          }
          tft.print(volume);
}

void playtone()
{
  tone(3, 3000, 500);
  delay(100);
  noTone(3);
  delay(30);
}

