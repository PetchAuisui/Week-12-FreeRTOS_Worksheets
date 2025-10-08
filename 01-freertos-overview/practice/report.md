# บันทึกผลการทดลอง
## Lab 1: ESP-IDF Setup และโปรเจกต์แรก
### Result
<img width="1918" height="1079" alt="image" src="https://github.com/user-attachments/assets/df8fc53d-45c8-42f0-83c2-6011e8031d22" />

### Exercise 
- Exercise-1
<img width="1918" height="1079" alt="image" src="https://github.com/user-attachments/assets/9ca99415-1b8d-4f79-ac22-6ae5295a0d6e" />

- Exercise-2
<img width="1919" height="1078" alt="image" src="https://github.com/user-attachments/assets/eae89076-ae02-4666-828e-bf0a04f50bb0" />

- Exercise-3
<img width="1917" height="1079" alt="image" src="https://github.com/user-attachments/assets/c94a9944-9328-41b5-994f-238b41f9689e" />

### คำถามทบทวน
1. ไฟล์ใดบ้างที่จำเป็นสำหรับโปรเจกต์ ESP-IDF ขั้นต่ำ?
- CMakeLists.txt (ระดับรากโปรเจกต์) ใช้ project(<name>)
- main/CMakeLists.txt ลงทะเบียน component และรายชื่อไฟล์ .c
- main/hello_esp32.c ที่มีฟังก์ชัน app_main()
- sdkconfig จะถูกสร้างโดยอัตโนมัติหลัง build ครั้งแรก (จำเป็นต่อการคอมไพล์ แต่ไม่ต้องสร้างเอง)
```
your_project/
├─ CMakeLists.txt # ระดับรากโปรเจกต์
├─ main/
│ ├─ CMakeLists.txt # ลงทะเบียนคอมโพเนนต์/ไฟล์ .c
│ └─ hello_esp32.c # มีฟังก์ชัน app_main()
└─ sdkconfig # สร้างอัตโนมัติหลัง build ครั้งแรก
```
2. ความแตกต่างระหว่าง hello_esp32.bin และ hello_esp32.elf คืออะไร?
- `.elf` คือไฟล์สำหรับดีบัก มีสัญลักษณ์ ตารางที่อยู่ และส่วนต่าง ๆ ครบ ใช้กับ gdb, addr2line, size
- `.bin` คือไบนารีที่สกัดออกมาจาก .elf เพื่อแฟลชลงชิป ถูกจัดรูป/สตริปข้อมูลดีบักแล้ว
- ระหว่างแฟลชมักมีหลายไฟล์ .bin เช่น bootloader.bin, partition-table.bin, และไฟล์แอป (เช่น hello_esp32.bin)
3. คำสั่ง idf.py set-target ทำอะไร?
- ตั้งค่าชิปเป้าหมาย เช่น esp32, esp32s3, esp32c3
- อัปเดต `IDF_TARGET` และคอนฟิกใน sdkconfig ให้สอดคล้องกับสถาปัตยกรรม
- บังคับให้ CMake รีคอนฟิก เลือกทูลเชน/ไดรเวอร์ที่ถูกต้อง
- ใช้เมื่อสลับบอร์ดหรือสถาปัตยกรรม
- ตัวอย่าง: ```idf.py set-target esp32```
4. โฟลเดอร์ build/ มีไฟล์อะไรบ้าง?
- เอาต์พุตหลักของแอป: hello_esp32.elf, hello_esp32.bin, hello_esp32.map
- โฟลเดอร์ย่อยสำคัญ:
  - bootloader/ (เช่น bootloader.elf/.bin)
  - partition_table/ (เช่น partition-table.bin/.csv)
  - esp-idf/ (ออบเจ็กต์ของแต่ละคอมโพเนนต์)
  - ไฟล์อำนวยความสะดวก: flasher_args.json, flash_project_args, config/, sdkconfig.h, compile_commands.json
  - ไฟล์ CMake: CMakeCache.txt, CMakeFiles/, และบางโปรเจกต์จะมีโฟลเดอร์ ld/ สำหรับลิงเกอร์สคริปต์ที่สร้างขึ้น
  - สรุป: build/ คือพื้นที่กลางของ CMake ที่เก็บไฟล์ทั้งหมดที่จำเป็นต่อการแฟลชและดีบัก
5. การใช้ vTaskDelay() แทน delay() มีความสำคัญอย่างไร?
- vTaskDelay() ทำให้ task เข้าสถานะ blocked ชั่วคราวและคืน CPU ให้ scheduler จัดสรรให้ task อื่นรัน จึงไม่ busy-wait และประหยัดพลังงาน
- ทำงานตาม tick ของ FreeRTOS (ใช้ pdMS_TO_TICKS(ms) เพื่อความถูกต้องของเวลา)
- พฤติกรรมเวลาคาดเดาได้ เหมาะกับมัลติทาสก์ใน ESP-IDF
- ถ้าต้องการคาบเวลาคงที่ให้ใช้ vTaskDelayUntil()
- ใน ESP-IDF ไม่มี delay() แบบ Arduino และการ busy-wait จะรบกวนการจัดสรรเวลาให้ task อื่น

## Lab 2: Hello World และ Serial Communication
### Result
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/0678180a-1adb-484b-85f2-78e45bfca2d6" />

- Exercise-1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/6b295dd6-d230-4fb3-ab6a-4ed19592bfe9" />

- Exercise-2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/365c2920-80d3-4b1b-a12a-c007ade6c2d1" />

- Exercise-3
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/a670796e-fea4-4c28-b932-dd839e3eb292" />

## คำถามทบทวน
1. ความแตกต่างระหว่าง `printf()` และ `ESP_LOGI()` คืออะไร?

| หัวข้อ                 | `printf()`                          | `ESP_LOGI()`                                |
| ---------------------- | ----------------------------------- | ------------------------------------------- |
| **ประเภท**             | ฟังก์ชันมาตรฐาน C สำหรับแสดงข้อความ | ฟังก์ชันของ ESP-IDF สำหรับระบบ Logging      |
| **ระดับ Log**          | ไม่มีระดับ — แสดงทุกข้อความ         | มีระดับ (Error, Warn, Info, Debug, Verbose) |
| **การควบคุม Log**      | ไม่สามารถเปิด/ปิดเฉพาะส่วนได้       | ควบคุมระดับและ Tag ได้ในแต่ละ Module        |
| **การแสดงผลเพิ่มเติม** | ไม่มี timestamp / tag               | แสดงเวลาบน log, ชื่อ tag และสีข้อความ       |
| **เหมาะสำหรับ**        | Debug ธรรมดา                        | Debug ที่มีระบบ log และการ filter ที่ดีขึ้น |

- **สรุป**:ESP_LOGI() เหมาะกับการพัฒนาใน ESP-IDF เพราะสามารถควบคุมความละเอียดของ log ได้ และดูข้อมูลเชิงระบบได้ง่ายกว่า printf() มาก
2. Log level ไหนที่จะแสดงใน default configuration?
- ค่าเริ่มต้นคือ INFO level
- นั่นหมายความว่า log ที่ระดับ Error (E), Warning (W) และ Info (I) จะถูกแสดงออกมา ส่วน Debug (D) และ Verbose (V) จะไม่แสดงจนกว่าจะตั้งค่าให้สูงขึ้น
3. การใช้ `ESP_ERROR_CHECK()` มีประโยชน์อย่างไร?
- ใช้ตรวจสอบว่าโค้ดหรือฟังก์ชันที่คืนค่า esp_err_t ทำงานสำเร็จหรือไม่
- ถ้าเกิด error จะ พิมพ์ข้อความ error อัตโนมัติ และ หยุดการทำงาน (abort) ของโปรแกรม
- ช่วยลดโค้ดเช็กเงื่อนไขซ้ำ ๆ เช่น if (ret != ESP_OK) {...}
- ตัวอย่าง
```
ESP_ERROR_CHECK(nvs_flash_init());
```
- ถ้า NVS init ล้มเหลว จะขึ้น log error พร้อมบอกบรรทัดของโค้ดทันที
4. คำสั่งใดในการออกจาก Monitor mode?
-  `Ctrl+]`
  - เมื่อกดตามนี้ โปรแกรมจะออกจาก idf.py monitor และกลับสู่ command line
5. การตั้งค่า Log level สำหรับ tag เฉพาะทำอย่างไร?
- ใช้ฟังก์ชัน:
```
esp_log_level_set("TAG_NAME", ESP_LOG_DEBUG);
```
- Ex :
```
esp_log_level_set("LOGGING_DEMO", ESP_LOG_DEBUG);
```
- → จะเปิด log ระดับ DEBUG เฉพาะ tag ที่ชื่อว่า "LOGGING_DEMO" ส่วน tag อื่น ๆ จะยังคงใช้ระดับ log เดิมตามที่ตั้งไว้
