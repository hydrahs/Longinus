//*********************SD******************************
#include <SPI.h>
#include <SD.h>
//*********************MS5611******************************
#include <Wire.h>
#include <MS5611.h>

MS5611 ms5611;


//define the values
    const int chipSelect = 4;

    unsigned long time,time1,time2;
    double AccX, AccY, AccZ;
    double ROLL, PITCH , YAW;
    double relativeAltitude ,altitude,altitude0; //**define altitude**
    double ma = 0;
    //**define your port**
    int Input1Bi = 3;//the first molde
    int Input1Fi = 2;
    int Input2Bi = 6;//the second modle
    int Input2Fi = 5;
    int Input3Bi = 8;//the third modle
    int Input3Fi = 7;

    //**define the flags**
    int fire = 0;
    int open = 0;

    //***define timecounter***
    long long CountTime;

void setup(){
    Serial.begin(115200);
    //initializing the TimeCounter
    noInterrupts(); //禁用全局中断
    TCCR2A = TCCR2A & B11111100;     //设置TCCR2A 中WGM21和WGM20的值为0
    TCCR2B = TCCR2B & B11110000;     //设置TCCR2B 中CS22 CS21 CS20 的值为0
    TIMSK2 = TIMSK2 & B11110001;     //设置TIMSK2中OCIE2Bhe OCIE2A的值为0
  
    OCR2A = 249;
    TCCR2A = (1<<WGM21);            //定时器模式设置为CTC mode
    TCCR2B = (1<<CS22);             //预分频器64
    TIMSK2 = (1<<OCIE2A);           //启动定时器中断
    CountTime = 0;
    interrupts();
    Serial.begin(115200);

    //********************************************************************************declare your output pin
    pinMode(Input1Bi,OUTPUT); 
    pinMode(Input2Bi,OUTPUT);
    pinMode(Input3Bi,OUTPUT);
    //******************************initializing  SD card********************
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }
    Serial.print("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        while (1);
    }
    Serial.println("card initialized.");
    //******************************open the electromagent********************************
    digitalWrite(Input2Bi, HIGH);

    //*******************************open MS5611******************************************
    while(!ms5611.begin(MS5611_ULTRA_HIGH_RES))
  {
    delay(500);
  }

    //*******************************calculate the zero point altitude********************
    calculate0();

}   

ISR(TIMER2_COMPA_vect){
  CountTime++;
  if (CountTime % 200 == 0){
        SaveData();
  }
  
}

void loop() {
    GetData();
    show();
    //******************fire the second **********************
    if ( relativeAltitude > 1.2){
        if(fire == 0 ){
            delay(700);
            fire = 1;
            digitalWrite(Input1Bi, HIGH);
            time1  = time;

        }
    }
    
    //******************close the electromagent **********************
    if(fire == 1){
        if(ma - relativeAltitude > 0 ){
            if(open == 0){
                delay(1500);
            digitalWrite(Input2Bi, LOW);
            open = 1;
            time2 = time;
            }
        }
    }

    //******************burn rope **********************
    if(open == 1){
        if(time - time2 > 10000  ){
            digitalWrite(Input3Bi, HIGH);
        }
    }
    
    //delay(50);
}

void GetData(){
    calculate();
}


void calculate(){
altitude = ms5611.getAltitude(ms5611.readPressure(true));
relativeAltitude = altitude - altitude0;
if(ma <= relativeAltitude){
        ma = relativeAltitude;
    }
}

void calculate0(){
    altitude0 = ms5611.getAltitude(ms5611.readPressure(true));
}



void show(){
  showTime();
}

void showTime(){
    Serial.print("Time");
    time = millis();
    Serial.println(time);
}

void SaveData(){
    // open the file. note that only one file can be open at a time,
    File dataFile = SD.open("a.txt", FILE_WRITE);
    // if the file is available, write to it:
    if (dataFile) {
        //dataFile.print("Time");
        dataFile.print(time);
        dataFile.print(" ");
        
        dataFile.print(time1);
        dataFile.print(" ");
        dataFile.print(time2);
        dataFile.print(" ");
        dataFile.print(altitude0);
        dataFile.print(" ");

        dataFile.println(relativeAltitude);
        // so you have to close this one before opening another.
        dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
        Serial.println("error opening datalog.txt");
    }
}
