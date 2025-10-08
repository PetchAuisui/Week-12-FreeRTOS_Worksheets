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


## Lab 3: สร้าง Task แรกด้วย FreeRTOS
### Result
- Step-1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/23c59a71-f8c2-4062-a037-2b2ce1dc085c" />

- Step-2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/663a4d1c-8b3d-4231-b903-a5c4aa1b5526" />

- Step-3
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/91617f11-7e30-4e6e-8250-bd4f7982b6a2" />

### แบบฝึกหัด
- Exercise 1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/bc1d610a-3efb-4071-af4d-fcfd3fc0152c" />

- Exercise 2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/dd836d76-fc7c-40a1-9ad1-253dd8abe718" />

### คำถามทบทวน
1. เหตุใด Task function ต้องมี infinite loop?
- เพราะใน FreeRTOS แต่ละ Task จะถูกรันโดย Scheduler ตลอดเวลาหาก Task จบการทำงาน (return ออกจากฟังก์ชัน) ระบบจะถือว่า Task นั้น สิ้นสุด (terminated) และ memory ที่ stack ของมันอาจถูก reclaim ได้
- ดังนั้น Task ส่วนใหญ่จะทำงานแบบวนลูป เช่น
```
while (1) {
   // ทำงานซ้ำ เช่น อ่าน sensor, กระพริบไฟ, รอ event
   vTaskDelay(pdMS_TO_TICKS(1000));
}
```
- ✅ ถ้าอยากให้ task ทำงานแค่ครั้งเดียว ควร vTaskDelete(NULL) หลังเสร็จงาน (ดู Exercise 1)
2. ความหมายของ stack size ใน xTaskCreate() คืออะไร?
- ค่าที่ส่งในพารามิเตอร์ xTaskCreate() เช่น
```
xTaskCreate(my_task, "MyTask", 2048, NULL, 1, NULL);
```
- ตัวเลข 2048 คือขนาดของ stack memory ของ task นั้น (หน่วยเป็น bytes)
- ใช้สำหรับเก็บ:
  - ตัวแปรท้องถิ่น (local variables)
  - call stack ของฟังก์ชัน
  - context ของ task (registers, return address ฯลฯ)
- หาก stack size เล็กเกินไป → จะเกิด Stack Overflow
- หากใหญ่เกินไป → เปลือง RAM โดยไม่จำเป็น

🔍 ตรวจสอบได้ด้วย uxTaskGetStackHighWaterMark() เพื่อดู stack ที่เหลือ
3. ความแตกต่างระหว่าง vTaskDelay() และ vTaskDelayUntil()?

| ฟังก์ชัน                                  | การทำงาน                                                           | เหมาะกับ                                                     |
| ----------------------------------------- | ------------------------------------------------------------------ | ------------------------------------------------------------ |
| `vTaskDelay(ticks)`                       | หน่วงเวลา “จากเวลาปัจจุบัน”                                        | Task ที่ต้องรอโดยไม่ต้อง precise (เช่น LED กระพริบ)          |
| `vTaskDelayUntil(&last_wake_time, ticks)` | หน่วง “คงที่จากจุดเริ่มต้นเดิม” เพื่อให้รอบการทำงานเท่ากันทุกครั้ง | Task แบบ periodic (รอบคงที่ เช่น sampling sensor ทุก 100 ms) |


- ตัวอย่าง:
```
TickType_t last = xTaskGetTickCount();
while (1) {
    do_something();
    vTaskDelayUntil(&last, pdMS_TO_TICKS(100)); // รอบละ 100ms คงที่
}
```
- ⚙️ ใช้ vTaskDelayUntil() เมื่อคุณต้องการ precise timing รอบละเท่าเดิมทุกครั้ง
4. การใช้ vTaskDelete(NULL) vs vTaskDelete(handle) ต่างกันอย่างไร?

| ฟังก์ชัน              | การลบใคร                         | ใช้เมื่อ                                             |
| --------------------- | -------------------------------- | ---------------------------------------------------- |
| `vTaskDelete(NULL)`   | ลบ “ตัวเอง”                      | Task ต้องการจบการทำงานเอง เช่น Temporary Task        |
| `vTaskDelete(handle)` | ลบ “Task อื่น” ที่มี handle อยู่ | ใช้ใน Task Manager หรือ Controller เพื่อลบ task อื่น |

- ตัวอย่าง:
```
// ใน temporary_task.c
vTaskDelete(NULL); // ลบตัวเองหลังครบเวลา

// ใน task_manager.c
vTaskDelete(led1_handle); // ลบ LED1 task จากภายนอก
```
5. Priority 0 กับ Priority 24 อันไหนสูงกว่า?
- Priority ยิ่งมาก → ยิ่งสูง
- Task Priority 24 สูงกว่า Priority 0
- FreeRTOS จะให้ CPU กับ Task ที่มี Priority สูงสุดก่อนเสมอ
- ถ้า Priority เท่ากัน → ใช้ Round-Robin Scheduling (ผลัดกันรัน)
- ตัวอย่างในแลป:
```
xTaskCreate(led1_task, "LED1", 2048, NULL, 2, NULL);
xTaskCreate(system_info_task, "SysInfo", 2048, NULL, 1, NULL);
```
- หมายความว่า led1_task จะ ได้รันก่อน system_info_task หากทั้งคู่พร้อมรัน
