
//////////////////////////////////////////////////////////////////
// Scroll 8 Matrix - Web client
// WebMatrix 2.2 - pierre@frabriqueurs.com
//
// Inspired by "HTML to LCD Server" v1.1 from Everett Robinson
//
//  Client Web permettant de recuperer un message sur un serveur Web et
// de le faire defiler sur un afficheur composé de 8 matrices de 8x8 LED 
// (Module d'affichage MAX7219)/

//////////////////////////////////////////////////////////////////
// Inclusion des libraries necessaires
//////////////////////////////////////////////////////////////////
#include <string.h>
#include <pgmspace.h>
#include <ESP8266WiFi.h>
// Utilisation d'une version speciale de MaxMatrix2 pour modules chinois de
// 4 matrices tournées de 90° : https://github.com/les-fabriqueurs/MaxMatrix2
#include <MaxMatrix2.h>


//////////////////////////////////////////////////////////////////////////
// Declaration variables globales
////////////////////////////////////////////////////////////////////////
int data = 12;     // Pin Arduino connecte a Pin DIN de la premiere martice de leds
int load = 14;     // Pin Arduino connecte a Pin CS des martices de leds
int clock = 4;    // Pin Arduino connecte a Pin CLK des martices de leds
int maxInUse = 8;  // Nombre de matrices de Led utilisees 
int buzzpin = 5;    // Pin Arduino connecte au buzzer  (l'autre est connecte a la masse)

/////////////////////////////////////////////////////////////////////////
// Tableau de caracteres stoquee en Flash 
// (afin de limiter l'utilisation de la RAM)
// Ce tableau contient le codage permettant d allumer les pixels necessaire
// a l'affichage d un caractere sur une matrice de Leds (une ligne par caracteres)
//
//  1er  element: largeur du caractere en pixels
//  2eme element: hauteur du caractere en pixels
//  3-5eme element: Codage en binaire des pixels a allumer 
///////////////////////////////////////////////////////////////////////////
PROGMEM const unsigned char CH[] = {
3, 8, B00000000, B00000000, B00000000, B00000000, B00000000, // space
1, 8, B01011111, B00000000, B00000000, B00000000, B00000000, // !
3, 8, B00000011, B00000000, B00000011, B00000000, B00000000, // "
5, 8, B00010100, B00111110, B00010100, B00111110, B00010100, // #
4, 8, B00100100, B01101010, B00101011, B00010010, B00000000, // $
5, 8, B01100011, B00010011, B00001000, B01100100, B01100011, // %
5, 8, B00110110, B01001001, B01010110, B00100000, B01010000, // &
1, 8, B00000011, B00000000, B00000000, B00000000, B00000000, // '
3, 8, B00011100, B00100010, B01000001, B00000000, B00000000, // (
3, 8, B01000001, B00100010, B00011100, B00000000, B00000000, // )
5, 8, B00101000, B00011000, B00001110, B00011000, B00101000, // *
5, 8, B00001000, B00001000, B00111110, B00001000, B00001000, // +
2, 8, B10110000, B01110000, B00000000, B00000000, B00000000, // ,
4, 8, B00001000, B00001000, B00001000, B00001000, B00000000, // -
2, 8, B01100000, B01100000, B00000000, B00000000, B00000000, // .
4, 8, B01100000, B00011000, B00000110, B00000001, B00000000, // /
4, 8, B00111110, B01000001, B01000001, B00111110, B00000000, // 0
3, 8, B01000010, B01111111, B01000000, B00000000, B00000000, // 1
4, 8, B01100010, B01010001, B01001001, B01000110, B00000000, // 2
4, 8, B00100010, B01000001, B01001001, B00110110, B00000000, // 3
4, 8, B00011000, B00010100, B00010010, B01111111, B00000000, // 4
4, 8, B00100111, B01000101, B01000101, B00111001, B00000000, // 5
4, 8, B00111110, B01001001, B01001001, B00110000, B00000000, // 6
4, 8, B01100001, B00010001, B00001001, B00000111, B00000000, // 7
4, 8, B00110110, B01001001, B01001001, B00110110, B00000000, // 8
4, 8, B00000110, B01001001, B01001001, B00111110, B00000000, // 9
2, 8, B01010000, B00000000, B00000000, B00000000, B00000000, // :
2, 8, B10000000, B01010000, B00000000, B00000000, B00000000, // ;
3, 8, B00010000, B00101000, B01000100, B00000000, B00000000, // <
3, 8, B00010100, B00010100, B00010100, B00000000, B00000000, // =
3, 8, B01000100, B00101000, B00010000, B00000000, B00000000, // >
4, 8, B00000010, B01011001, B00001001, B00000110, B00000000, // ?
5, 8, B00111110, B01001001, B01010101, B01011101, B00001110, // @
4, 8, B01111110, B00010001, B00010001, B01111110, B00000000, // A
4, 8, B01111111, B01001001, B01001001, B00110110, B00000000, // B
4, 8, B00111110, B01000001, B01000001, B00100010, B00000000, // C
4, 8, B01111111, B01000001, B01000001, B00111110, B00000000, // D
4, 8, B01111111, B01001001, B01001001, B01000001, B00000000, // E
4, 8, B01111111, B00001001, B00001001, B00000001, B00000000, // F
4, 8, B00111110, B01000001, B01001001, B01111010, B00000000, // G
4, 8, B01111111, B00001000, B00001000, B01111111, B00000000, // H
3, 8, B01000001, B01111111, B01000001, B00000000, B00000000, // I
4, 8, B00110000, B01000000, B01000001, B00111111, B00000000, // J
4, 8, B01111111, B00001000, B00010100, B01100011, B00000000, // K
4, 8, B01111111, B01000000, B01000000, B01000000, B00000000, // L
5, 8, B01111111, B00000010, B00001100, B00000010, B01111111, // M
5, 8, B01111111, B00000100, B00001000, B00010000, B01111111, // N
4, 8, B00111110, B01000001, B01000001, B00111110, B00000000, // O
4, 8, B01111111, B00001001, B00001001, B00000110, B00000000, // P
4, 8, B00111110, B01000001, B01000001, B10111110, B00000000, // Q
4, 8, B01111111, B00001001, B00001001, B01110110, B00000000, // R
4, 8, B01000110, B01001001, B01001001, B00110010, B00000000, // S
5, 8, B00000001, B00000001, B01111111, B00000001, B00000001, // T
4, 8, B00111111, B01000000, B01000000, B00111111, B00000000, // U
5, 8, B00001111, B00110000, B01000000, B00110000, B00001111, // V
5, 8, B00111111, B01000000, B00111000, B01000000, B00111111, // W
5, 8, B01100011, B00010100, B00001000, B00010100, B01100011, // X
5, 8, B00000111, B00001000, B01110000, B00001000, B00000111, // Y
4, 8, B01100001, B01010001, B01001001, B01000111, B00000000, // Z
2, 8, B01111111, B01000001, B00000000, B00000000, B00000000, // [
4, 8, B00000001, B00000110, B00011000, B01100000, B00000000, // \ backslash
2, 8, B01000001, B01111111, B00000000, B00000000, B00000000, // ]
3, 8, B00000010, B00000001, B00000010, B00000000, B00000000, // hat
4, 8, B01000000, B01000000, B01000000, B01000000, B00000000, // _
2, 8, B00000001, B00000010, B00000000, B00000000, B00000000, // `
4, 8, B00100000, B01010100, B01010100, B01111000, B00000000, // a
4, 8, B01111111, B01000100, B01000100, B00111000, B00000000, // b
4, 8, B00111000, B01000100, B01000100, B00101000, B00000000, // c
4, 8, B00111000, B01000100, B01000100, B01111111, B00000000, // d
4, 8, B00111000, B01010100, B01010100, B00011000, B00000000, // e
3, 8, B00000100, B01111110, B00000101, B00000000, B00000000, // f
4, 8, B10011000, B10100100, B10100100, B01111000, B00000000, // g
4, 8, B01111111, B00000100, B00000100, B01111000, B00000000, // h
3, 8, B01000100, B01111101, B01000000, B00000000, B00000000, // i
4, 8, B01000000, B10000000, B10000100, B01111101, B00000000, // j
4, 8, B01111111, B00010000, B00101000, B01000100, B00000000, // k
3, 8, B01000001, B01111111, B01000000, B00000000, B00000000, // l
5, 8, B01111100, B00000100, B01111100, B00000100, B01111000, // m
4, 8, B01111100, B00000100, B00000100, B01111000, B00000000, // n
4, 8, B00111000, B01000100, B01000100, B00111000, B00000000, // o
4, 8, B11111100, B00100100, B00100100, B00011000, B00000000, // p
4, 8, B00011000, B00100100, B00100100, B11111100, B00000000, // q
4, 8, B01111100, B00001000, B00000100, B00000100, B00000000, // r
4, 8, B01001000, B01010100, B01010100, B00100100, B00000000, // s
3, 8, B00000100, B00111111, B01000100, B00000000, B00000000, // t
4, 8, B00111100, B01000000, B01000000, B01111100, B00000000, // u
5, 8, B00011100, B00100000, B01000000, B00100000, B00011100, // v
5, 8, B00111100, B01000000, B00111100, B01000000, B00111100, // w
5, 8, B01000100, B00101000, B00010000, B00101000, B01000100, // x
4, 8, B10011100, B10100000, B10100000, B01111100, B00000000, // y
3, 8, B01100100, B01010100, B01001100, B00000000, B00000000, // z
3, 8, B00001000, B00110110, B01000001, B00000000, B00000000, // {
1, 8, B01111111, B00000000, B00000000, B00000000, B00000000, // |
3, 8, B01000001, B00110110, B00001000, B00000000, B00000000, // }
4, 8, B00001000, B00000100, B00001000, B00000100, B00000000, // ~
1, 8, B00000000, B00000000, B00000000, B00000000, B00000000, // DEL (unused)
4, 8, B00111110, B01000001, B11000001, B00100010, B00000000, // Ç  96
4, 8, B00111100, B01000001, B01000000, B01111100, B00000000, // ü  97
4, 8, B00111000, B01010110, B01010101, B00011000, B00000000, // é  98
4, 8, B00100000, B01010110, B01010101, B01111010, B00000000, // â  99
4, 8, B00100000, B01010101, B01010100, B01111001, B00000000, // ä  100
4, 8, B00100000, B01010101, B01010110, B01111000, B00000000, // à  101
4, 8, B00100000, B01010100, B01010100, B01111000, B00000000, // å  102
4, 8, B00111010, B01010101, B01010101, B00011010, B00000000, // ê  103
4, 8, B00111000, B01010101, B01010100, B00011001, B00000000, // ë  104
4, 8, B00111000, B01010101, B01010110, B00011000, B00000000, // è  105
3, 8, B01000101, B01111100, B01000001, B00000000, B00000000, // ï  106
3, 8, B01000110, B01111101, B01000010, B00000000, B00000000  // î  107
};


byte buffer[10];

// Constantes pour connexion Wifi
char ssid[]     = "xxx";
char password[] = "xxx";

// site sur lequel faire la requete GET 
const char* host = "www.xxxxxx.xx";

// Instanciation d'un module MaxMatrix (utilise pour piloter les dites matrices)
MaxMatrix m(data, load, clock, maxInUse);

////////////////////////////////////////////////////////////////
// Fonctions d affichage d'un texte deroulant sur les Matrices
//
////////////////////////////////////////////////////////////////
void printCharWithShift(char c, int shift_speed){
  int char_indice = 0;
//  Serial.print("c1 = ");
//  Serial.println(c);
//  Serial.print("c1 hexa = "); 
//  Serial.println(c,HEX);
  
// Switch pour gestion des caracteres accentués
  switch (c) {
    case 0XFFFFFFC3:
      break;
    case 0XFFFFFFA0 :
       char_indice= 101; // à
      break;
   case 0XFFFFFFA9:
        char_indice=98; // é
      break;
   case 0XFFFFFFA8 :
        char_indice=105; // è
      break; 
   case 0XFFFFFFAA:
        char_indice=103; // ê
      break;
   case 0XFFFFFFAB :
        char_indice=104; // ë
      break;
   case 0XFFFFFFA4 :
        char_indice=100; // ä
      break; 
   case 0XFFFFFFAE:
        char_indice=107; // î
      break;
   case 0XFFFFFFA2:
        char_indice=99; // â
      break;
   case 0XFFFFFFAF:
        char_indice=106; // ï     
  }
    
  if ((c < 32)&&(char_indice == 0)) return;
  if ((c < 127)&&(char_indice == 0)) char_indice = c - 32;
 
  memcpy_P(buffer, CH + 7*char_indice, 7);
  m.writeSprite(maxInUse*8, 0, buffer);
  //m.setCollumn(maxInUse*8 + buffer[0], 0);
  
  for (int i=0; i<buffer[0]+1; i++) 
  {
    delay(shift_speed);
    m.shiftLeft(false,false);
  }
}

void printStringWithShift(char* s, int shift_speed){
  while (*s != 0){
    printCharWithShift(*s, shift_speed);
    s++;
  }
}


//////////////////////////////////////////////////////////////////
////////////          Fonction bip        /////////////////////
//////////////////////////////////////////////////////////////////
void bip(int targetPin, long length) {
  digitalWrite(targetPin,HIGH);
  delay(length);
  digitalWrite(targetPin,LOW);
}


void setup() {
  // declare pin 9 to be an output for Buzzer:
  pinMode(buzzpin, OUTPUT);
  
  Serial.begin(115200);
  delay(10);

 // Matrix init
  m.init(); // module initialize
  m.setIntensity(3); // dot matix intensity 0-15

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  char msg1[]="Connecting to WiFi                  ";
  char msg2[]="Connected to ";
  char msg3[]="Connecting to WiFi failed...        ";
  
//  printStringWithShift(msg1, 80);
  WiFi.begin(ssid, password);
  
  for (int i=0; i<30; i++) {
    if (WiFi.status() == WL_CONNECTED) {
//      printStringWithShift(msg2, 80);
//      printStringWithShift(ssid, 80);
        Serial.print(WiFi.status());
      break;
    }  
    else
    {
      delay(500);
      Serial.print(".");
      Serial.print(WiFi.status());    
    }
    if (i==19)
      printStringWithShift(msg3, 80);
  }
 
  Serial.println("");
  Serial.println("WiFi connected"); 
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}


void loop() {
//  delay(5000);

  Serial.print("connecting to ");
  Serial.println(host);
  
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/afficheur-led/";
  url += "?lastmsg";
  
  Serial.print("Requesting URL: ");
  Serial.println(url);
  
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  String line;
  int i=0;
  while(client.available()){   
    line = client.readStringUntil('\n');
    i=line.indexOf("Message en cours:");
    if (i >=0)
      break;
  }
 
  Serial.println(line);

  char msg2[] = ". . . . .                      ";
  char msg[50];
  Serial.println(i);  
  line.replace("Message en cours: ","");
  Serial.println(line);
  line.toCharArray(msg, 50); 
  printStringWithShift(msg, 60);
  printStringWithShift(msg2, 60); 
  
  Serial.println();
  Serial.println("closing connection");
}

