#define BLYNK_TEMPLATE_ID "BLYNK_TEMPLATE_ID"
#define BLYNK_DEVICE_NAME "BLYNK_DEVICE_NAME"
#define BLYNK_AUTH_TOKEN "BLYNK_AUTH_TOKEN"
#define BLYNK_PRINT Serial

// Wifi, Blynk uygulaması, Firebase, Servo motor kütüphaneleri
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp8266.h>
#include <FirebaseArduino.h>
#include <Servo.h>

Servo servo;       // Servo motoru kontrol etmek için kullanılacak nesneyi oluşturma
int servoAngle;       // Servo motor açısı

char auth[] = BLYNK_AUTH_TOKEN;

char ssid[] = "ssid";
char pass[] = "pass"; 

const char *sunucu_IP = "canlisaat.com";           // bağlanılacak olan web sayfasının adresi
String yol = "/";            // bağlanılacak olan web sayfasının adresi canlisaat.com/

String okunan_veri="";
String tarih=""; 

#define firebase_access_link "firebase_access_link"   // oluşturduğunuz projenizdeki veritabanınızın erişim linki
#define firebase_pass "firebase_pass"    // veritabanı secret key

BLYNK_WRITE(V1)        // (Mobil) Blynk uygulamasında kullanılacak button - Blynk uygulamasından 1 veya 0 olarak ayarlandı 1 on 0 off
{
  int i=param.asInt();        // Blynk uygulamasından tam sayı olarak değer alır 1 veya 0 - param.asInt() Blynk kütüphane kodu
    if (i==1)         // button açılırsa
    {
      servoAngle=179;        // açı
      servo.write(servoAngle);        // servo motoru belirlenen açıya getir
    }
    else         // button kapatılırsa
    {
      servoAngle=0;
      servo.write(servoAngle);           // servo motoru belirlenen açıya getir
    }
}

BLYNK_WRITE(V2)        // (WEB) Blynk uygulamasında kullanılacak switch button - Blynk uygulamasından 1 veya 0 olarak ayarlandı 1 on 0 off
{
  int i=param.asInt();        // Blynk uygulamasından tam sayı olarak değer alır 1 veya 0 - param.asInt() Blynk kütüphane kodu
    if (i==1)       // switch button açılırsa
    {
      servoAngle=179; // açı
      servo.write(servoAngle);     // servo motoru belirlenen açıya getir
    }
    else        // switch button kapatılırsa
    {
      servoAngle=0;
      servo.write(servoAngle);    // servo motoru belirlenen açıya getir
    }
}



void setup(void) {
    Serial.begin(9600);    

    delay(1000);
  
    WiFi.mode(WIFI_STA);         //ESP8266 istasyon moduna alınıyor. 
    WiFi.begin(ssid, pass);            //SSID ve şifre girilierek wifi başlatılıyor.

  //------- Wifi ağına bağlanıncaya kadar beklenilen kısım ------------
    Serial.print("Bağlantı kuruluyor");
    while (WiFi.status() != WL_CONNECTED) 
    { // Wifi bağlantısı sağlanana kadar bekleniyor.
      delay(500);                               // Bu arada her 500 ms de bir seri ekrana yan yana noktalar yazdırılarak
      Serial.print(".");                        //görsellik sağlanıyor.
    }

    Serial.println("");                         //Bir alt satıra geçiliyor.
    Serial.println("Bağlantı sağlandı...");      //seri ekrana bağlantının kurulduğu bilgisi gönderiliyor.

    Serial.print("Alınan IP addresi: ");        // kablosuz ağdan alınan IP adresi
    Serial.println(WiFi.localIP());             // kablosuz ağdan alınan IP adresi

    Firebase.begin(firebase_access_link, firebase_pass);      // Firebase erişimini başlat
       
    Blynk.begin(auth,ssid,pass);          // Blynk uygulamasını başlatma
    servo.attach(D1);         // Servo motor sinyal kablosu Nodemcu D1 pinine bağlı
    
    servoAngle=0; 
    servo.write(servoAngle);          // Başlangıçta servo motor 0 açısında

    
}



void loop(void) {
  WiFiClient istemci;                       // istemci nesnesi oluşturuldu
  
  if (!istemci.connect(sunucu_IP, 80))           // web bağlantısı yap, bağlanamadı mı?
  {
    Serial.println("Baglanti kurulamadi!!!");        // bağlanamadı, hata ver
    return;
  }
  
  // sunucudan "***.***.***.***/" adresindeki siteyi iste, aşağıdaki kod HTTP protokolü ile sayfa isteği
  istemci.print(String("GET ") + yol + " HTTP/1.1\r\n" + "Host: " + sunucu_IP + "\r\n" + "Connection: close\r\n\r\n");

  //-------- 3 sn lik zaman döngüsü kur ve istek geldi mi diye kontrol et ------
  // çalışma zamanı milisaniye cinsinden tutan millis() fonksiyonundan süre al, başlangıc zamanı
  unsigned long timeout = millis();     
  while (istemci.available() == 0) 
  {
    if (millis() - timeout > 3000)      // o anki zamandan başlangıç zamanını çıkar, 3 sn dolmuş mu?
    {
      Serial.println(">>> Baglanti Zaman Asimi !");
      istemci.stop();
      return;
    }
  }

  while (istemci.available())     // sunucudan gelen bilgi var mı?
  {
    okunan_veri = istemci.readStringUntil('\r');  // sunucu tarafından gönderilen tüm bilgiyi satır sonuna kadar oku
    
    //Serial.print(okunan_veri);                  // seri porttan gönder, ilk önce bu satır açık olup seri porttan gelen yazıya bakabiliriz
                                                  // ardından aşağıdaki işlemleri yapmak daha kolay olur, sonra bu satır tekrar yorum yapılır
    
  

    //-------------- string işleme prosedürünün yapıldığı yer ----------------
    if(okunan_veri.indexOf("Date:")!= -1)            // okunan satırda "Date: " yazısı var mı?
    {                                                // evet varmış
      int baslangic=okunan_veri.indexOf("Date:");     // "Date: " yazısının başlangıç indexini al
      tarih="";                                       // tarih değişkenini bir temizle
      
   
      tarih=okunan_veri.substring(baslangic+10,baslangic+32);       //Date değerini çekme
      
      
      Serial.println(tarih);
     }
  }
  
  Blynk.run();        // Nodemcu ile Blynk uygulaması arasında sürekli bağlantıyı sağlar
    

  if(servoAngle>0)         //Servo motor 0 dan farklı bi açıdaysa yani sadece besleme işlemi yapılıyorsa
  {
     Firebase.setString("En son besleme",tarih);        // Firebase de En son besleme etiketinin altına besleme tarihini yazdır
  }
  
}
