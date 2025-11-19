#include <Arduino.h>
#include <OneButton.h>
#include "ota.h"
#include "func.h"  // Bao gồm file header func.h để sử dụng các hàm từ func.cpp


//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Khởi tạo đối tượng màn hình OLED U8G2
//U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE); // Khởi tạo đối tượng màn hình OLED U8G2

// Thông tin mạng WiFi và OTA



StaticJsonDocument<200> jsonDoc;

const char* jsonString = R"()";
bool isChanged = false;
void tinhToanCaiDat();
void loadSetup();
void veGoc();

OneButton btnMenu(0, true,false);
OneButton btnSet(2, false,false);
OneButton btnUp(12, false,false);
OneButton btnDown(15, false,false);
OneButton btnRun(23,false,false);
OneButton btnEstop(13,false,false);


void btnMenuClick() {
  //Serial.println("Button Clicked (nhấn nhả)");
  if (displayScreen == "ScreenCD") {
    if (keyStr == "CD" && isChanged) {
      isChanged = false;
      writeFile(jsonDoc,"/config.json");
    }
    showList(menuIndex);  // Hiển thị danh sách menu hiện tại
    displayScreen = "MENU";
  } else if (displayScreen == "ScreenEdit") {
    loadJsonSettings();
    displayScreen = "ScreenCD";
  } else if (displayScreen == "index" && mainStep == 0) {
    trangThaiHoatDong = 0;
    showList(menuIndex);  // Hiển thị danh sách menu hiện tại
    displayScreen = "MENU";
  } else if (displayScreen == "MENU" && mainStep == 0){
    displayScreen= "index";
    trangThaiHoatDong = 1;
    veGoc();
    //showText("HELLO", "ESP32-OPTION");
  } else if (displayScreen == "testIO"){
    loadJsonSettings();
    displayScreen = "ScreenCD";
    trangThaiHoatDong = 0;
  } else if (displayScreen == "testOutput"){
    loadJsonSettings();
    displayScreen = "ScreenCD";
    trangThaiHoatDong = 0;
  } else if (displayScreen == "screenTestMode" && testModeStep == 0){
    loadJsonSettings();
    displayScreen = "ScreenCD";
    trangThaiHoatDong = 0;
  } else if (displayScreen == "OTA"){
    loadJsonSettings();
    displayScreen = "ScreenCD";
    trangThaiHoatDong = 0;
  }
}

// Hàm callback khi bắt đầu nhấn giữ nút
void btnMenuLongPressStart() {
  if (displayScreen == "OTA") {
  }
}
// Hàm callback khi nút đang được giữ
void btnMenuDuringLongPress() {
  //Serial.println("Button is being Long Pressed (BtnMenu)");
}

void btnSetClick() {
  if (displayScreen == "MENU") {
    pIndex = 1;
    loadJsonSettings(); // Hiển thị giá trị thiết lập
    displayScreen = "ScreenCD"; // Chuyển màn hình sau khi xử lý dữ liệu thành công
  } else if (displayScreen == "ScreenCD" && editAllowed){
    if (keyStr == "CD"){
      columnIndex = maxLength-1;
      showEdit(columnIndex);
      displayScreen = "ScreenEdit";
    } else if (keyStr == "CN") {
      if (setupCodeStr == "CN1"){
        trangThaiHoatDong = 201;   //Trạng thái hoạt động 201 là trạng thái TestMode
        testModeStep = 0;
        chayTestMode = true;
        showText("TEST MODE", String("Step " + String(testModeStep)).c_str());
        displayScreen = "screenTestMode";
      } else if (setupCodeStr == "CN2"){
        trangThaiHoatDong = 202;   //Trạng thái hoạt động 202 là trạng thái TEST IO INPUT
        showText("TEST I/O", "TEST I/O INPUT");
        displayScreen = "testIO";
      } else if ((setupCodeStr == "CN3")){
        trangThaiHoatDong = 203;  //Trạng thái hoạt động 203 là trạng thái TEST IO OUTPUT
        testOutputStep = 0;
        displayScreen = "testOutput";
        hienThiTestOutput = true;
      } else if ((setupCodeStr == "CN5")){
        setupOTA();
        displayScreen = "OTA";
        trangThaiHoatDong = 204;  //Trạng thái hoạt động 204 là trạng thái OTA UPDATE+0
      } else {
        columnIndex = maxLength - 1;
        showEdit(columnIndex);
        displayScreen = "ScreenEdit";
      }
    }
  } else if (displayScreen == "ScreenEdit" && editAllowed)  {
    if (keyStr == "CD"){
      if (columnIndex - 1 < 0){
        columnIndex = maxLength-1;
      } else {
        columnIndex --;
      }
      showEdit(columnIndex);
    }
  } else if (displayScreen == "testOutput"){
    daoTinHieuOutput = true;
  }
}

// Hàm callback khi bắt đầu nhấn giữ nút
void btnSetLongPressStart() {
  if (displayScreen == "ScreenEdit"){
    if (keyStr == "CD"){
      jsonDoc["main"]["main" + String(menuIndex)]["children"][setupCodeStr]["configuredValue"] = currentValue;
      isChanged = true;
      log("Đã lưu giá trị:" + String(currentValue) + " vào thẻ " + keyStr + "/" + setupCodeStr);
      loadJsonSettings();
      loadSetup();
      tinhToanCaiDat();
      displayScreen = "ScreenCD";
    } else if (keyStr == "CN"){
      if (setupCodeStr == "CN4" && currentValue == 1){
        reSet();
        showText("RESET","Tắt máy khởi động lại!");
        trangThaiHoatDong = 200;  //Trạng thái hoạt động 200 là reset, không cho phép thao tác nào
        displayScreen = "RESET";
      }
    }
  }
}

// Hàm callback khi nút đang được giữ
void btnSetDuringLongPress() {
  //showSetup("Setup", "OFF", "Dang giu nut");
}

void btnUpClick() {
  if (displayScreen == "MENU") {
    if (menuIndex + 1 > 3) {
      menuIndex = 1;  // Khi chỉ số vượt quá giới hạn, quay lại đầu danh sách
    } else {
      menuIndex++;    // Tăng menuIndex lên 1
    }
    showList(menuIndex);  // Hiển thị danh sách với chỉ số mới
  } else if (displayScreen == "ScreenCD") {
    if (pIndex + 1 > totalChildren) {
      pIndex = 1;
    } else {
      pIndex ++;
    }
    loadJsonSettings(); // Hiển thị giá trị thiết lập
  } else if (displayScreen == "ScreenEdit") {
    if (keyStr == "CD"){
      editValue("addition");
      log("Value:" + valueStr);
    } else if (keyStr == "CN") {
      editValue("addition");
      log("Value:" + valueStr);
    }
  } else if (displayScreen == "testOutput"){
    if (testOutputStep == maxTestOutputStep){
      testOutputStep = 0;
      hienThiTestOutput = true;
    } else {
      testOutputStep ++;
      hienThiTestOutput = true;
    }
  } else if (displayScreen == "screenTestMode"){
    if (testModeStep < maxTestModeStep){
      testModeStep ++;
    } else {
      testModeStep = 0;
    }
    chayTestMode = true;
    showText("TEST MODE", String("Step " + String(testModeStep)).c_str());
  }
}

// Hàm callback khi bắt đầu nhấn giữ nút
void btnUpLongPressStart() { 
  //Serial.println("Button Long Press Started (btnUp)");
}

// Hàm callback khi nút đang được giữ
void btnUpDuringLongPress() {
  //Serial.println("Button is being Long Pressed (btnUp)");
}

void btnDownClick() {
  if (displayScreen == "MENU") {
    if (menuIndex - 1 < 1) {
      menuIndex = 3;  // Khi chỉ số nhỏ hơn giới hạn, quay lại cuối danh sách
    } else {
      menuIndex--;    // Giảm menuIndex đi 1
    }
    showList(menuIndex);  // Hiển thị danh sách với chỉ số mới
  } else if (displayScreen == "ScreenCD"){
    if (pIndex - 1 < 1) {
      pIndex = totalChildren;
    } else {
      pIndex --;
    }
    loadJsonSettings(); // Hiển thị giá trị thiết lập
  } else if (displayScreen == "ScreenEdit"){
    if (keyStr == "CD"){
      editValue("subtraction");
      log("Value:" + valueStr);
    } else if (keyStr == "CN"){
      editValue("subtraction");
      log("Value:" + valueStr);
    }
  } else if (displayScreen == "testOutput"){
    if (testOutputStep == 0){
      testOutputStep = maxTestOutputStep;
      hienThiTestOutput = true;
    } else {
      testOutputStep --;
      hienThiTestOutput = true;
    }
  } else if (displayScreen == "screenTestMode"){
    if (testModeStep > 0){
      /*testModeStep --;
      chayTestMode = true;
      showText("TEST MODE", String("Step " + String(testModeStep)).c_str());*/
    }
  }
}

// Hàm callback khi bắt đầu nhấn giữ nút
void btnDownLongPressStart() {
  //Serial.println("Button Long Press Started (btnDown)");
  veGoc();
}

// Hàm callback khi nút đang được giữ
void btnDownDuringLongPress() {
  //Serial.println("Button is being Long Pressed (btnDown)");
}

//KHAI BÁO CHÂN IO Ở ĐÂY

const int SensorIn1 = 36;
const int SensorIn2 = 39;
const int SensorIn3 = 34;
const int SensorIn4 = 35;
const int SensorIn5 = 32;
const int SensorIn6 = 33;

const int SensorIn7 = 25;
const int SensorIn8 = 26;

const int SensorIn9 = 23;
const int SensorIn10 = 13;


const int Out1 = 4;
const int Out2 = 16;
const int Out3 = 17;
const int Out4 = 5;
const int Out5 = 18;
const int Out6 = 19;

const int Out7 = 27;
const int Out8 = 14;

int pinPWM = 27; // Chọn chân xuất xung
float freq_kHz = 0;  

//KHAI BÁO THÔNG SỐ TRƯƠNG TRÌNH

/* Ví dụ: int thoiGianNhaDao = 200;
          int soDuMuiDauVao = 10;*/


//TRƯƠNG TRÌNH NGƯỜI DÙNG LẬP TRÌNH


void testMode(){
  switch (testModeStep){
  case 0:
    if(chayTestMode){
      maxTestModeStep = 2;
      chayTestMode = false;
    }
    break;

  case 1:
    if (chayTestMode){
      /* code */
    }
    break;
  case 2:
    /* code */
    break;
  default:
    /* code */
    break;
  }
}

void testInput(){
  static bool trangthaiCuoiIO1;
  if (digitalRead(SensorIn1)!= trangthaiCuoiIO1){
    trangthaiCuoiIO1 = digitalRead(SensorIn1);
    showText("IO 36" , String(trangthaiCuoiIO1).c_str());
  }
  static bool trangthaiCuoiIO2;
  if (digitalRead(SensorIn2)!= trangthaiCuoiIO2){
    trangthaiCuoiIO2 = digitalRead(SensorIn2);
    showText("IO 39" , String(trangthaiCuoiIO2).c_str());
  }
  static bool trangthaiCuoiIO3;
  if (digitalRead(SensorIn3)!= trangthaiCuoiIO3){
    trangthaiCuoiIO3 = digitalRead(SensorIn3);
    showText("IO 34" , String(trangthaiCuoiIO3).c_str());
  }
  static bool trangthaiCuoiIO4;
  if (digitalRead(SensorIn4)!= trangthaiCuoiIO4){
    trangthaiCuoiIO4 = digitalRead(SensorIn4);
    showText("IO 35" , String(trangthaiCuoiIO4).c_str());
  }
  static bool trangthaiCuoiIO5;
  if (digitalRead(SensorIn5)!= trangthaiCuoiIO5){
    trangthaiCuoiIO5 = digitalRead(SensorIn5);
    showText("IO 32" , String(trangthaiCuoiIO5).c_str());
  }
  static bool trangthaiCuoiIO6;
  if (digitalRead(SensorIn6)!= trangthaiCuoiIO6){
    trangthaiCuoiIO6 = digitalRead(SensorIn6);
    showText("IO 33" , String(trangthaiCuoiIO6).c_str());
  }
  static bool trangthaiCuoiIO7;
  if (digitalRead(SensorIn7)!= trangthaiCuoiIO7){
    trangthaiCuoiIO7 = digitalRead(SensorIn7);
    showText("IO 25" , String(trangthaiCuoiIO7).c_str());
  }
  static bool trangthaiCuoiIO8;
  if (digitalRead(SensorIn8)!= trangthaiCuoiIO8){
    trangthaiCuoiIO8 = digitalRead(SensorIn8);
    showText("IO 26" , String(trangthaiCuoiIO8).c_str());
  }
  static bool trangthaiCuoiIO9 = true;
  if (digitalRead(SensorIn9)!= trangthaiCuoiIO9){
    trangthaiCuoiIO9 = digitalRead(SensorIn9);
    showText("IO 23" , String(trangthaiCuoiIO9).c_str());
  }
  static bool trangthaiCuoiIO10 = true;
  if (digitalRead(SensorIn10)!= trangthaiCuoiIO10){
    trangthaiCuoiIO10 = digitalRead(SensorIn10);
    showText("IO 13" , String(trangthaiCuoiIO10).c_str());
  }
}

void testOutput(){
  switch (testOutputStep){
    case 0:
      if (hienThiTestOutput){
        maxTestOutputStep = 7;
        bool tinHieuHienTai = digitalRead(Out1);
        showText("IO 4", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(Out1);
        digitalWrite(Out1,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
    case 1:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(Out2);
        showText("IO 16", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(Out2);
        digitalWrite(Out2,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
    case 2:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(Out3);
        showText("IO 17", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(Out3);
        digitalWrite(Out3,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
    case 3:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(Out4);
        showText("IO 5", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(Out4);
        digitalWrite(Out4,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
    case 4:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(Out5);
        showText("IO 18", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(Out5);
        digitalWrite(Out5,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
      case 5:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(Out6);
        showText("IO 19", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(Out6);
        digitalWrite(Out6,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
      case 6:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(Out7);
        showText("IO 27", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(Out7);
        digitalWrite(Out7,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;
      case 7:
      if (hienThiTestOutput){
        bool tinHieuHienTai = digitalRead(Out8);
        showText("IO 14", String(tinHieuHienTai).c_str());
        hienThiTestOutput = false;
      } else if (daoTinHieuOutput){
        bool tinHieuHienTai = digitalRead(Out8);
        digitalWrite(Out8,!tinHieuHienTai);
        hienThiTestOutput = true;
        daoTinHieuOutput = false;
      }
      break;

    default:
      break;
  }
}


void tinhToanCaiDat(){
  /* Ví dụ:
  soXungCanChay = soXungMotor * soVongCuon;
  thoiGianDaoPWM = (1000000*30)/(tocDoQuay*soXungMotor);
  digitalWrite(pinDir,chieuQuayDongCo);
  */

}

void loadSetup(){
  /* Ví dụ:
  cheDoHoatDong = jsonDoc["main"]["main1"]["children"]["CD1"]["configuredValue"];
  thoiGianNhaDao = jsonDoc["main"]["main1"]["children"]["CD2"]["configuredValue"];
  soDuMuiDauVao = jsonDoc["main"]["main1"]["children"]["CD3"]["configuredValue"];
  soDuMuiDauRa = jsonDoc["main"]["main1"]["children"]["CD4"]["configuredValue"];
  */

}

void veGoc(){
  // trangThaiHoatDong = 198 , 199;
  showText("ORIGIN", "Đang về gốc");
  trangThaiHoatDong = 199;
}

void khoiDong(){
  delay(200);
  displayScreen = "index";
  showText("HELLO","Xin Chào");
  mainStep = 0;
  trangThaiHoatDong = 0;
  loadSetup();
  delay(200);
  tinhToanCaiDat();
  delay(100);
  veGoc();
}

void mainRun(){
  switch (mainStep){
  case 0:
    /* code */
    break;
  
  case 1:

    break;
  case 2:
    /* code */
    break;
  default:
    break;
  }
}



void setup() {

  Serial.begin(115200);     // Khởi tạo Serial và màn hình

  u8g2.begin();  // Khởi tạo màn hình OLED
  u8g2.enableUTF8Print(); // Kích hoạt hỗ trợ UTF-8

  btnMenu.attachClick(btnMenuClick);
  btnMenu.attachLongPressStart(btnMenuLongPressStart);
  btnMenu.attachDuringLongPress(btnMenuDuringLongPress);

  btnSet.attachClick(btnSetClick);
  btnSet.attachLongPressStart(btnSetLongPressStart);
  btnSet.attachDuringLongPress(btnSetDuringLongPress);

  btnUp.attachClick(btnUpClick);
  btnUp.attachLongPressStart(btnUpLongPressStart);
  btnUp.attachDuringLongPress(btnUpDuringLongPress);

  btnDown.attachClick(btnDownClick);  
  btnDown.attachLongPressStart(btnDownLongPressStart);
  btnDown.attachDuringLongPress(btnDownDuringLongPress);

  btnMenu.setDebounceMs(btnSetDebounceMill);
  btnSet.setDebounceMs(btnSetDebounceMill);
  btnUp.setDebounceMs(btnSetDebounceMill);
  btnDown.setDebounceMs(btnSetDebounceMill);

  btnMenu.setPressMs(btnSetPressMill);
  btnSet.setPressMs(btnSetPressMill);
  btnUp.setPressMs(btnSetPressMill);
  btnDown.setPressMs(btnSetPressMill);

  pinMode(SensorIn1,INPUT);
  pinMode(SensorIn2,INPUT);
  pinMode(SensorIn3,INPUT);
  pinMode(SensorIn4,INPUT);
  pinMode(SensorIn5,INPUT);
  pinMode(SensorIn6,INPUT);

  pinMode(SensorIn7,INPUT);
  pinMode(SensorIn8,INPUT);
  pinMode(SensorIn9,INPUT);
  pinMode(SensorIn10,INPUT);

  pinMode(Out1,OUTPUT);
  pinMode(Out2,OUTPUT);
  pinMode(Out3,OUTPUT);
  pinMode(Out4,OUTPUT);
  pinMode(Out5,OUTPUT);
  pinMode(Out6,OUTPUT);

  pinMode(Out7,OUTPUT);
  pinMode(Out8,OUTPUT);


  if (!LittleFS.begin()) {
    showSetup("Error", "E003", "LittleFS Mount Failed");
    Serial.println("LittleFS Mount Failed");
    return;
  }

  

  // Kiểm tra xem file config.json có tồn tại không
  if (!LittleFS.exists(configFile)) {
    DeserializationError error = deserializeJson(jsonDoc, jsonString); // Phân tích chuỗi JSON
    if (error) {
        showSetup("Error", "E005", "JsonString Error");
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.f_str());
        return;
    }
    showSetup("Error", "E007", "JsonString Load Mode");
    Serial.println("Read Data From JsonString");
    loadSetup();
    Serial.println("File config.json does not exist.");
    return;
  }

  readConfigFile();

  Serial.println("Load toàn bộ dữ liệu thành công");
  khoiDong();
}

void loop() {
  switch (trangThaiHoatDong){
  case 0:
    btnMenu.tick();
    btnSet.tick();
    btnUp.tick();
    btnDown.tick();
    break;
  case 1: {
  const uint32_t durationTest_ms = 1000;  // Thời gian test: 1000 ms = 1 giây
  const uint32_t xung_us = 1;             // Thời gian mỗi xung: 2 µs (1 µs HIGH + 1 µs LOW)

  soXungDaChay = 0;                       // Reset bộ đếm
  uint32_t start = millis();             // Ghi lại thời điểm bắt đầu

  // Vòng lặp test: xuất xung liên tục trong 1 giây
  while (millis() - start < durationTest_ms) {
    xuatXungPWM_us(xung_us, pinPWM);     // Gọi hàm xuất xung theo đơn vị µs
  }

  // Tính tần số thực tế đạt được (số xung / giây)
  float freq_Hz = (float)soXungDaChay / (durationTest_ms / 1000.0); // Hz
  freq_kHz = freq_Hz / 1000.0;                                      // kHz

  // Hiển thị thông tin lên màn hình OLED
  char buf[48];
  u8g2.clearBuffer();

  u8g2.drawStr(0, 12, "PWM xung test (us)");

  snprintf(buf, sizeof(buf), "Pin: %d", pinPWM);
  u8g2.drawStr(0, 26, buf);

  snprintf(buf, sizeof(buf), "Xung: %lu", soXungDaChay);
  u8g2.drawStr(0, 40, buf);

  snprintf(buf, sizeof(buf), "Freq: %.2f kHz", freq_kHz);
  u8g2.drawStr(0, 54, buf);

  u8g2.sendBuffer();

  delay(1000); // Chờ 1 giây trước khi test lại
}  
    break;
  case 2:
    mainRun();
    break;
  case 198:     // Về Gốc 1
    trangThaiHoatDong = 1;
    break;
  case 199:     // Về Gốc 2
    if(Wait(2000)){
      trangThaiHoatDong = 1;
      showText("READY", "Đang Hoạt Động");
    }
    break;
  case 200:        //ESTOP dừng khẩn cấp
    btnMenu.tick();
    break;
  case 201:         // Func Test Mode
    btnMenu.tick();
    btnUp.tick();
    btnDown.tick();
    testMode();
    break;
  case 202:        // Func Test Input
    btnMenu.tick();
    testInput();
    break;
  case 203:      // Func Test Output
    btnMenu.tick();
    btnSet.tick();
    btnUp.tick();
    btnDown.tick();
    testOutput();
    break;
  case 204:
    btnMenu.tick();
    handleOTA(); // Xử lý OTA khi điều kiện đúng
    break;  
  default:
    break;
  }
}
