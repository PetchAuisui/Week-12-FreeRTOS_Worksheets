# บันทึกผลการทดลอง
## Lab 1: ESP-IDF Setup และโปรเจกต์แรก
## Result
<img width="1918" height="1079" alt="image" src="https://github.com/user-attachments/assets/df8fc53d-45c8-42f0-83c2-6011e8031d22" />

### Exercise 
#### Exercise-1
##### Result
<img width="1918" height="1079" alt="image" src="https://github.com/user-attachments/assets/9ca99415-1b8d-4f79-ac22-6ae5295a0d6e" />

#### Exercise-2
<img width="1919" height="1078" alt="image" src="https://github.com/user-attachments/assets/eae89076-ae02-4666-828e-bf0a04f50bb0" />

#### Exercise-3
<img width="1917" height="1079" alt="image" src="https://github.com/user-attachments/assets/c94a9944-9328-41b5-994f-238b41f9689e" />

## คำถามทบทวน
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
