//Khai báo thư viện sử dụng
#include <Wire.h> 
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#define SCREEN_WIDTH 128  // Chiều rộng OLED
#define SCREEN_HEIGHT 64 // Chiều cao OLED
//Khởi tạo một đối tượng hiển thị với chiều rộng, chiều cao được xác định trước đó bằng giao thức truyền thông I2C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
//Tham số -1 nghĩa là màn hình OLED không có chân Reset

char auth[] = "vGbHjZV8kE0dR44WwxS7C-eXdoqPfwjT";       // Auth Token
char ssid[] = "SSS_D";        //Tên Wifi
char pass[] = "123@123Hh";       //Mật khẩu Wifi
int UpperThreshold = 518; //Giá trị ngưỡng trên, Xác định tín hiệu nào được tính là 1 nhịp, tín hiệu nào bỏ qua.
int LowerThreshold = 490; //Giá trị ngưỡng dưới
int reading = 0; //Đọc giá trị ADC từ cảm biến
float BPM = 0.0; //Nhịp tim
bool IgnoreReading = false;
bool FirstPulseDetected = false;
unsigned long FirstPulseTime = 0;
unsigned long SecondPulseTime = 0;
unsigned long PulseInterval = 0;
const unsigned long delayTime = 10;
const unsigned long delayTime2 = 1000;
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;

void setup() {
  Serial.begin(9600); // Tốc độ baud
  Blynk.begin(auth, ssid, pass); // Khởi tại Blynk
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) //Khởi tạo màn hình
  {
    Serial.println(F("SSD1306 allocation failed"));
    //Thông báo trên Serial Monitor, trong trường hợp không thể kết nối với màn hình
    for (;;); //Tránh lặp liên tục
  }
  delay(2000);
  // Trễ hai giây để OLED có đủ thời gian khởi tạo trước khi viết văn bản
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
  display.println("Loading . . .");
  display.display();
  delay(3000);
}

void loop()
{
  Blynk.run();
  BPM_True();
  Send_Blynk();
}

void BPM_True()
{
  //Nhận thời gian hiện tại
  unsigned long currentMillis = millis();

  //Thiết lập thời gian đo cho 1 xung
  if (myTimer1(delayTime, currentMillis) == 1)
  {

    reading = analogRead(0);

    //Giá trị đọc được vượt ngưỡng UpperThreshold
    if (reading > UpperThreshold && IgnoreReading == false)
    {
      if (FirstPulseDetected == false)
      {
        FirstPulseTime = millis();
        FirstPulseDetected = true;
      }
      else {
        SecondPulseTime = millis();
        PulseInterval = SecondPulseTime - FirstPulseTime;
        FirstPulseTime = SecondPulseTime;
      }
      IgnoreReading = true;      
    }

    //Giá trị đọc được dưới ngưỡng LowerThreshold
    if (reading < LowerThreshold && IgnoreReading == true)
    {
      IgnoreReading = false;    
    }

    // Tính nhịp đập mỗi phút
    BPM = int((1.0 / PulseInterval) * 60.0 * 1000); // Ép kiểu int để nhận giá trị nguyên
    //Khoảng thời gian được tính bằng ms, *1000 để chuyển sang s,*60 để chuyển sang phút
  }

  //Hiển thị lên Serial
  if (myTimer2(delayTime2, currentMillis) == 1)
  {
    Serial.print(reading);
    Serial.print("\t");
    Serial.print(PulseInterval);
    Serial.print("\t");
    Serial.print(BPM); 
    Serial.println(" BPM "); // Gửi giá trị tín hiệu đến Serial Plotter.
    Serial.flush();
  }                   
  display.clearDisplay(); //Xóa bộ đệm hiển thị bằng clearDisplay()
  display.setTextSize(1); //Đặt kích thước phông chữ bằng cách sử dụng setTextSize()
  display.setTextColor(WHITE); //Đặt màu phông chữ với setTextColor()
  display.setCursor(0, 10); //Xác định vị trí mà văn bản bắt đầu bằng cách sử dụng setCursor(x, y)
  display.println("Measure heartbeat"); // Gửi văn bản đến màn hình bằng cách sử dụng println()
  display.setCursor(0, 30);
  display.println("BPM");
  display.setCursor(40, 30);
  display.println(String(BPM));
  display.display();
}

void Send_Blynk()
{
  Blynk.virtualWrite(V7, BPM); //Truyền dữ liệu lên bộ Gauge kênh V7
}

//Bộ đếm thời gian cập nhật phép tính BPM
int myTimer1(long delayTime, long currentMillis) 
{
  if (currentMillis - previousMillis >= delayTime) 
  {
    previousMillis = currentMillis;
    return 1;
  }
  else {
    return 0;
  }
}

//Bộ đếm thời gian cập nhật hiển thị lên Serial
int myTimer2(long delayTime2, long currentMillis) 
{
  if (currentMillis - previousMillis2 >= delayTime2)
  {
    previousMillis2 = currentMillis;
    return 1;
  }
  else 
  {
    return 0;
  }
}
