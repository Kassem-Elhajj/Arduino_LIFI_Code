#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Arduino.h>


unsigned long startTime;  // Variable to store the start time
unsigned long endTime;    // Variable to store the end time
unsigned long loopTime;

const int BlueLed = 13;

LiquidCrystal_I2C lcd(0x27,16,2);
unsigned long lastExecutedMillis = 0; 
int m = 0;
int photoPin = A0;
int ref = 0;
int Threshold = 15;
int ZReg = 0;
int delb ;
int delb1 = 500 ;
int delb2 = 500 ;
String Word = "";
String Byte = "";
String Sentence = "";
String DecryptedSentence;
char ch;
bool startBit = false;
bool firstBit = true;

const char* charMap =          "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNPOQRSTUVWXYZ0123456789";
const char* encryptedCharMap = "mnbvcxzlkjhgfdsapoiuytrewqMNBVCXZLKJHGFDASPOIUYTREWQ9876543210";

void GetBit1();
void GetBit0();
void DecapsulationAndDecrypt();
char getCharacterFromBinary(String binary);
String normalizeString(const String& str);
void LedOn();
void LedOff();
typedef void (*FunctionPointer)();
void callFunctionFromMap(const String& str);
int findCharIndex(char ch, const char* map);
String decrypt(String encryptedPassword);


void setup() {
  Serial.begin(9600);
  
  lcd.init(); // initialize the lcd
  lcd.backlight();

  pinMode(BlueLed, OUTPUT);
  digitalWrite(BlueLed, LOW);
  ref = analogRead(photoPin);
  delb = delb1;
}


void loop() {
//  startTime = micros();
  
  int reading = analogRead(photoPin);
  int intensity = reading - ref;
  
  DecapsulationAndDecrypt();
  
  if(intensity > Threshold){
    GetBit1();
  }
  
  if(intensity <= Threshold && intensity >= -Threshold && !firstBit){
    GetBit0();
  }
  unsigned long delayStartTime = millis();
  
  while (millis() - delayStartTime <= delb) {
    // Wait for the specified delay duration
  }


  
//  endTime = micros();  // Record the end time

//  loopTime = endTime - startTime;  // Calculate the loop duration

  // Print the loop duration to the Serial Monitor
//  Serial.print("Loop time: ");
//  Serial.print(loopTime);
//  Serial.println(" microseconds");
  
}










//Get bit 1
//----------------------------------------------------------------------------------
void GetBit1(){
  ZReg = 0;
  Byte += '1';
  startBit = true;
  if(firstBit){
    delb = delb2;
    firstBit = false;
    lcd.setCursor(0,0);
    lcd.clear();
  }
  Serial.print('1');
 // delay(delb);
}

//Get bit 0
//----------------------------------------------------------------------------------
void GetBit0(){
  Byte += '0';
  ZReg++;
  
  if(ZReg >= 10){
    firstBit = true;
    delb = delb1;
//    ref = analogRead(photoPin);
    Byte="";
    Sentence="";
    Serial.print("\n");
  }
  Serial.print('0');
  //delay(1.25*delb);
}

//Decapsulation and converting bits to char, then decrypting and displaying the syntence on the LCD.
//----------------------------------------------------------------------------------
void DecapsulationAndDecrypt(){
  if(Byte.length() == 10 && Byte[9] == '0'){
    startBit = false;
    Byte = Byte.substring(1, Byte.length() - 1);
    
    ch = getCharacterFromBinary(Byte);
    Word += ch;
    if(ch == ' ' || ch == '.'){
      Serial.print(Word);
      Sentence += Word;
      Word = "";
      m+=0.1; //0.1 for 500ms
      if(ch == '.'){
         DecryptedSentence = decrypt(Sentence);
         lcd.setCursor(0,0);
         lcd.clear();
         lcd.setCursor(0,0);
         lcd.print(DecryptedSentence);
          if(Sentence.length()>16){
            lcd.setCursor(0,1);
            lcd.print(DecryptedSentence.substring(16,DecryptedSentence.length()));
          }
         callFunctionFromMap(DecryptedSentence);
         Sentence="";
      }
    }
    Byte = "";
    Serial.print("\n");
}else if(Byte.length() == 10 && Byte[9] != '0'){
  startBit = false;
  Word += '~';
  Serial.println(Word);
  Byte = "";
}
}







//Get Char function
//----------------------------------------------------------------------------------

//m += 0.01;

//  unsigned long endTime = micros(); 
//  
//  unsigned long elapsedTime = endTime - startTime; 
//  Serial.println(elapsedTime);}
  
char getCharacterFromBinary(String binary) {
  byte asciiValue = 0;
  for (int i = 0; i < 8; i++) {
    if (binary.charAt(i) == '1') {
      asciiValue |= (1 << (7 - i));
    }
  }
  return char(asciiValue);
}





//Funtions related to tasks based on input string
//----------------------------------------------------------------------------------


// Utility function to normalize a string (convert to lower case and remove spaces)
String normalizeString(const String& str) {
    String normalized = str;
    normalized.toLowerCase();
    normalized.replace(" ", "");
    return normalized;
}

// Example functions to be mapped
void LedOn() {
    digitalWrite(BlueLed, HIGH);
    Serial.println("Led is ON!");
}

void LedOff() {
    digitalWrite(BlueLed, LOW);
    Serial.println("Led is OFF!");
}

// Define the type for the functions we want to map to
typedef void (*FunctionPointer)();  // Define a function pointer type

// Define a structure to hold the string-function pairs
struct FunctionMap {
    String key;
    FunctionPointer function;
};

// Initialize an array of FunctionMap structures
FunctionMap functionMap[] = {
    {normalizeString("LedOn."), LedOn},
    {normalizeString("LedOff."), LedOff}
};

// Function to call the associated function from the map
void callFunctionFromMap(const String& str) {
    String normalized = normalizeString(str);
    for (unsigned int i = 0; i < sizeof(functionMap) / sizeof(FunctionMap); i++) {
        if (functionMap[i].key == normalized) {
            functionMap[i].function();
            return;
        }
    }
    Serial.println("Function not found");
}







//Decryption
//----------------------------------------------------------------------------------

// Utility function to find the index of a character in a map
int findCharIndex(char ch, const char* map) {
    for (int i = 0; i < strlen(map); i++) {
        if (map[i] == ch) {
            return i;
        }
    }
    return -1;
}

// Function to decrypt the password
String decrypt(String encryptedPassword) {
    String decryptedPassword = "";
    for (int i = 0; i < encryptedPassword.length(); i++) {
        char ch = encryptedPassword[i];
        int index = findCharIndex(ch, encryptedCharMap);
        if (index != -1) {
            decryptedPassword += charMap[index];
        } else {
            decryptedPassword += ch;
        }
    }
    return decryptedPassword;
}
