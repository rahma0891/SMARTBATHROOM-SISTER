#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

const char* ssid = "MASQOMAR_21 0365";
const char* password = "budakcindo";

// inisialisasi Bot Token
#define BOTtoken "6236249395:AAFYKmTJUcZmcOVPUIQdsSQf5O2mtv5yqVg"  // Bot Token dari BotFather

// chat id dari @myidbot
#define CHAT_ID "1365262211"

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

LiquidCrystal_I2C lcd(0x27, 16, 2);

Servo myservo;

const int btn1 = 15;
const int btn2 = 23;

const int servo1Pin = 2;

const int relay1 = 18;
const int relay2 = 19;

const int TrigPin = 13;
const int echoPin = 12;

const int ldrPin = 35;

const int pirPin = 34;

// dalam bak mandi in cm 
int bakMandi = 17;

int lampValue = 0;
int pompavalue = 0;

int BtnlampValue = 0;
int Btnpompavalue = 0;

int pirStatus = 0;

float jarak;

// pesan status
String pesanAda = "sedang ada orang";
String pesanTidakAda = "tidak ada orang";

void setup(){
    myservo.attach(servo1Pin);

    lcd.init();
    lcd.backlight();                
    lcd.setCursor(0,1); 
    lcd.print("----------------");

    pinMode(btn1, INPUT);
    pinMode(btn2, INPUT);

    pinMode(relay1, OUTPUT);
    pinMode(relay2, OUTPUT);

    pinMode(TrigPin, OUTPUT);
    pinMode(echoPin, INPUT);

    pinMode(ldrPin, INPUT);

    pinMode(pirPin, INPUT);

    Serial.begin(9600);

    // Koneksi Ke Wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    #ifdef ESP32
        client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    #endif
    while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
    }
    // Print ESP32 Local IP Address
    Serial.println(WiFi.localIP());
}
void onLamp(){
    digitalWrite(relay2, HIGH);
    lampValue = 1;
}
void offLamp(){
    digitalWrite(relay2, LOW);
    lampValue = 0;
}

void onPompa(){
    digitalWrite(relay1, HIGH);
    pompavalue = 1;
}

void offPompa(){
    digitalWrite(relay1, LOW);
    pompavalue = 0;
}

void lampu( int sensor){
    // Serial.println(sensor);
    if(sensor > 3000){
        if(lampValue == 0){
            onLamp();
        }
    } else {
            offLamp();
    }
    
}

int btnValue(int input, String text){
    if (text == "lampu"){
        if(input){
            BtnlampValue++;
            if(BtnlampValue > 1) {
                BtnlampValue = 0;
            }
            delay(250);
        }
        return BtnlampValue;
    }else{
        if(input){
            Btnpompavalue++;
            if(Btnpompavalue > 1) {
                Btnpompavalue = 0;
            }
            delay(250);
        }
        return Btnpompavalue;
    }   
}

float range(){
    //jarak in cm
    digitalWrite(TrigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(TrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(TrigPin, LOW);
  
    long duration = pulseIn(echoPin, HIGH);
    float rangepoin = duration * 0.034 / 2; 

    // Serial.print("jarak fungsi : ");
    // Serial.println(rangepoin);

    return rangepoin;
}

void pompa(float range) {
    if((jarak/(bakMandi-2))* 100 > 50.00) {
        onPompa();
    } if ((jarak/(bakMandi-2))* 100 <= 15.00) {
        Btnpompavalue = 0;
        offPompa();
    }
}

 void handleNewMessages(int numNewMessages, float rangevalue) {
    Serial.println("handleNewMessages");
    Serial.println(String(numNewMessages));

    for (int i=0; i<numNewMessages; i++) {
        String chat_id = String(bot.messages[i].chat_id);
        if (chat_id != CHAT_ID){
            bot.sendMessage(chat_id, "Unauthorized user", "");
            continue;
        }

        String text = bot.messages[i].text;
        Serial.println(text);
        String from_name = bot.messages[i].from_name;

        if (text == "/start") {
            String control = "Selamat Datang, " + from_name + ".\n";
            control += "Gunakan Commands Di Bawah Untuk Control Lednya.\n\n";
            control += "/cek_status Untuk mengecek ada orang atau tidak\n";
            control += "/cek_air Untuk memberhentikan proses pengisian \n";
            control += "/isi_air untuk mengisi air ke bak mandi\n";
            control += "/lampu_Nyala Untuk Menyalakan lampu \n";
            control += "/lampu_Mati Untuk Matikan lampu \n";
            bot.sendMessage(chat_id, control, "");
        }

        if (text == "/cek_status") {
            bot.sendMessage(chat_id, "status kamar mandi\n", "");
            if (pirStatus == 1) {
                bot.sendMessage(chat_id, pesanAda, "");
            } else {
                bot.sendMessage(chat_id, pesanTidakAda, "");
            }
            
        }
        
        if (text == "/cek_air") {
            bot.sendMessage(chat_id, "kapasitas air", "");
            bot.sendMessage(chat_id, String((((rangevalue/bakMandi)100)-100)-1) + " % ", "");
        }
        if (text == "/isi_air") {
            if(pompavalue == 0) {
                onPompa();
                bot.sendMessage(chat_id, "isi air", "");
                Serial.println("isi air");
            } else {
                bot.sendMessage(chat_id, "air sedang di isi", "");
            }
            
        }
        if (text == "/lampu_Mati") {
            if (lampValue == 1 ){
                offLamp();
                bot.sendMessage(chat_id, "lampu di matikan", "");
                // Serial.println("lampu mati");
            } else {
                bot.sendMessage(chat_id, "lampu sudah matikan", "");
            }
            
        }
        if (text == "/lampu_Nyala") {

            if (lampValue == 0 ){
                onLamp();
                bot.sendMessage(chat_id, "lampu di dihidupkan", "");
            //     Serial.println("lampu hidup");
            }
            else {
                bot.sendMessage(chat_id, "lampu sudah hidup", "");
            }
        }
    }
}

void loop(){
    jarak = range();

    // Serial.println((jarak/bakMandi)* 100);
    pompa(jarak);

    BtnlampValue = btnValue(digitalRead(btn1), "lampu");
    if(BtnlampValue == 1){
        if(lampValue == 0) {
            onLamp();
        }
    }

    Btnpompavalue = btnValue(digitalRead(btn2), "pompa");

    // Serial.println(Btnpompavalue);
    if(Btnpompavalue == 1 ){
        if(pompavalue == 0){
            onPompa();
        }
    }
    
    if (digitalRead(pirPin)) {
        pirStatus = 1;
        lcd.setCursor(0,0);
        lcd.print(pesanAda);
        myservo.write(90);

        lampu(analogRead(ldrPin));
        Serial.println(analogRead(ldrPin));
    } else {
        // delay(2000);
        pirStatus = 0;
        lcd.setCursor(0,0);
        lcd.print(pesanTidakAda);
        myservo.write(0);
        if(lampValue == 1){
            BtnlampValue = 0;
            offLamp();
        }
    }

    
    if (millis() > lastTimeBotRan + botRequestDelay)  {
        int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        Serial.println(numNewMessages);

        while(numNewMessages) {
            Serial.println("got response");
            handleNewMessages(numNewMessages, jarak);
            numNewMessages = bot.getUpdates(bot.last_message_received + 1);
        }
        lastTimeBotRan = millis();
    }
}
