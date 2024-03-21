#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <MFRC522.h>
#include <Ticker.h>
#include <ESP8266WiFi.h>
#include <HTTPSRedirect.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C

const char *GScriptId = "*Your Google Script ID*";
String gate_number = "Gate1";

const char* ssid     = "*Your SSID*";
const char* password = "*AP Password*";

String payload_base =  "{\"command\": \"insert_row\", \"sheet_name\": \"Sheet1\", \"values\": ";
String payload = "";

const char* host        = "script.google.com";
const int   httpsPort   = 443;
const char* fingerprint = "";
String url = String("/macros/s/") + GScriptId + "/exec";
HTTPSRedirect* client = nullptr;

constexpr uint8_t RST_PIN = D8;
constexpr uint8_t SS_PIN = D4;

const byte int_pin = 0;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

MFRC522 rfid(SS_PIN, RST_PIN);
MFRC522::MIFARE_Key key;
MFRC522::StatusCode status; 

struct KeyValue
{
    String key;
    String value;
};

const uint8_t maxItems=8;
KeyValue KeyValueArray[maxItems];
String tag;

int counter = 3;
volatile bool alarm_flag=false;
bool draw_flag=false;
bool admin_card_flag=false;
bool stop_interrupt_flag=false;
bool service_flag=false;
volatile bool admin_button_flag=false;
unsigned long act_time=0;
unsigned long start_time=0;
unsigned long interval=3000;
uint8_t led_stan=0;

uint8_t var=0;

Ticker timer_a, timer_b;

void addKeyValuePair(String key, String value);
String getValue( String key);
String getKey(String tag);
uint8_t check_card(String tag);
String GSpayload(String value);
void drawStartPage();
void drawMainPage();
void drawWelcomePage(String tag);
void drawWrongPage(uint8_t counter);
void drawAlarmPage(void);
void drawServicePage(void);
void timerCallback();
void admin_button();
void isr();

void setup() {

    pinMode(D9, OUTPUT);
    pinMode(D2, INPUT_PULLUP);
    pinMode(D1, INPUT_PULLUP);
    attachInterrupt(D2, admin_button, FALLING);
    attachInterrupt(D1, isr, FALLING);
    timer_a.attach_ms(200, timerCallback);
   
    Wire.begin(D3, D10);
    SPI.begin();
    WiFi.begin(ssid, password);

    rfid.PCD_Init();
    addKeyValuePair("Admin", "19215108245");
    addKeyValuePair("Kuba", "9915516515");
    addKeyValuePair("Konrad", "11964204115");
    addKeyValuePair("Ania", "2786224115");

    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;);
    }

  client = new HTTPSRedirect(httpsPort); // tworzy nowy obiekt klasy do obsługi połączeń po HTTPS
  client->setInsecure(); // klient nie weryfikuje certyfikatów serwera
  client->setPrintResponseBody(true); // ustawia opcję "printowania" odpowiedzi
  client->setContentTypeHeader("application/json"); // ustawia nagłówek Content-Type dla żądania HTTP na "application/json", klient wysyła żądania z oznaczeniem zawartości jako JSON

  delay(5000);

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10,10);
  display.println("Connecting to ");
  display.setCursor(10,20);
  display.println(host);
  display.display();

  // Próba połączenia 5 razy
  bool flag = false;
  for(int i=0; i<5; i++){
    int retval = client->connect(host, httpsPort); // nawiązanie połączenia z serwerem o określonym adresie i porcie HTTPS

    if (retval == 1){
      flag = true;
 
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(10,10);
      display.println("Connected. OK");
      display.display();
      delay(2000);
      break;
    }

    else{
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(10,0);
      display.println("Connection failed. Retrying...");
      display.display();

  }
  }
  if (!flag){
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,3);
    display.println("Could not connect to server: ");
    display.println(host);
    display.display();
    delay(5000);
    return;
  }

  delete client;    // usuń obiekt HTTPSRedirect
  client = nullptr; // wskaźnik null

    drawStartPage();
}


void loop() {
  static bool flag = false;
  if (!flag){
    client = new HTTPSRedirect(httpsPort);
    client->setInsecure();
    flag = true;
    client->setPrintResponseBody(true);
    client->setContentTypeHeader("application/json");
  }
  if (client != nullptr){

    if (!client->connected()){
      int retval = client->connect(host, httpsPort);
      if (retval != 1){
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(10,10);
        display.println("Disconnected. Retrying...");
        display.display();

        return; //reset pętli
      }
    }
  }
    act_time = millis();
    var=0;
 
    if(!rfid.PICC_IsNewCardPresent());
        // return;
    if(rfid.PICC_ReadCardSerial()){
        for(uint8_t i=0; i<4; i++){
            tag += rfid.uid.uidByte[i];
        }
        rfid.PICC_HaltA();
        rfid.PCD_StopCrypto1();
    }


    var = check_card(tag);

        if(var==1 && (!alarm_flag) && (!service_flag)){
            // alarm_flag = false;


            // stop_interrupt_flag = false;
            counter =3;


            digitalWrite(D9, LOW);
            drawWelcomePage(tag);
            admin_card_flag=false;
            draw_flag=true;
            start_time=act_time;
            }


        if(var==2){
            alarm_flag = false;


            stop_interrupt_flag = false;
            service_flag = false;
            admin_button_flag = false;
            counter =3;
            digitalWrite(D9, LOW);
            drawWelcomePage(tag);
           
            admin_card_flag=true;
            draw_flag=true;
            start_time=act_time;
            }
           
   
        if((var==0) && (!alarm_flag)&& (!service_flag)){
            counter--;
            drawWrongPage(counter);
            draw_flag=true;
            admin_card_flag=false;
            start_time=act_time;
            if(counter <= 0){
                alarm_flag=true;
                }


        }
        if(admin_button_flag==true && admin_card_flag==true){

            stop_interrupt_flag = true;
            draw_flag=false;
            drawServicePage();
            service_flag=true;
           
        }

        if(draw_flag){
            if(act_time - start_time>=interval){
                drawMainPage();
                draw_flag=false;
                admin_button_flag=false;
                admin_card_flag=false;
            }
        }
 

        if(alarm_flag==true && stop_interrupt_flag==false){
            drawAlarmPage();
        }
      if (tag != "") {
        payload = GSpayload(tag);
        client->POST(url, host, payload);
        tag = "";  // resetuje tag
    }
}

void addKeyValuePair(String key, String value){
    for(uint8_t i=0; i<maxItems; i++){
        if(KeyValueArray[i].key==""){
            KeyValueArray[i].key=key;
            KeyValueArray[i].value=value;
            return;
        }
    }
}

String getValue(String key){
    String tmp="";
    for(uint8_t i=0; i<maxItems; i++){
        if(KeyValueArray[i].key==key){
            tmp = KeyValueArray[i].value;
        }
    }
    return tmp;
}

String getKey(String value){
    String tmp="";
    for(uint8_t i=0; i<maxItems; i++){
        if(KeyValueArray[i].value==value){
            tmp=KeyValueArray[i].key;
        }
    }
    return tmp;
}


void drawStartPage(void) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(20,15);
    display.println("WELCOME!");
    display.display();
    delay(1000);
    display.clearDisplay();
    display.drawRect(15, 10, 94, 19, WHITE);
    display.setTextSize(1);
    display.setCursor(20,15);
    display.println("Scan Your Card");
    display.display();
}


void drawMainPage(void){
    display.clearDisplay();
    display.drawRect(15, 10, 94, 19, WHITE);
    display.setTextSize(1);
    display.setCursor(20,15);
    display.println("Scan Your Card");
    display.display();
}


void drawWelcomePage(String tag){
   
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(30,0);
    display.println("WELCOME!");
    display.setCursor(30,20);
    display.setTextSize(1);
    display.println(getKey(tag));
    display.display();
}


void drawWrongPage(uint8_t counter){


    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(15,0);
    display.println("Permission denied.");
    display.setTextSize(1);
    display.setCursor(15,20);
    display.print(counter, DEC);
    display.print(" attemps left.");
    display.display();
}


void drawAlarmPage(void){
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(20,15);
    display.println("ALARM!!!");
    display.display();
}


void drawServicePage(void){
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(20,15);
    display.println("SERVICE MODE");
    display.display();
}


void timerCallback(){
    if(alarm_flag==true && stop_interrupt_flag==false){
        digitalWrite(D9, !digitalRead(D9)); 
    }
}

uint8_t check_card(String tag){
    uint8_t check=0;
    for(uint8_t i=0; i<maxItems; i++){
        if(KeyValueArray[i].value==tag){
            check++;
            if(KeyValueArray[i].key=="Admin") check++;
    }
    }
    return check;
}


String values = "";

String GSpayload(String value) {
    String who = "Unknown";  // Ustawiamy wartość domyślną przed pętlą


    for (uint8_t i = 0; i < maxItems; i++) {
        if (KeyValueArray[i].value == value) {
            who = KeyValueArray[i].key;  // Jeśli znaleziono wartość, ustawiamy who
            break;  // Przerywamy pętlę, ponieważ już znaleźliśmy wartość
        }
    }


    values = "\"" + who + "," + value + "\"}";
    payload = payload_base + values;


    return payload;
}


void IRAM_ATTR admin_button(){
    admin_button_flag=true;
}

void IRAM_ATTR isr(){
    alarm_flag = true;
}





