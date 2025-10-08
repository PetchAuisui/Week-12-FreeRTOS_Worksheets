# บันทึกผลการทดลอง
## Lab 1: Basic Software Timers
### result
- การทดลองที่ 1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/66268e25-349c-414e-a885-9b9357e583fc" />

- กาทดลองที่ 2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/faa994af-c52e-4e17-9b8d-855b453195aa" />

- การทดลองที่ 3
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/d92422b6-1ea8-464c-90b3-4d81d7931dd6" />

### ความท้าทายเพิ่มเติม
1. Timer Synchronization: ซิงค์หลาย timers
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/f043bbab-8cbb-42b0-9451-548c237baac5" />

2. Performance Analysis: วัด timer accuracy
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/7288eba3-cdee-40f6-8a57-4b0c958e26f6" />

3. Error Handling: จัดการ timer failures
<img width="1918" height="1079" alt="image" src="https://github.com/user-attachments/assets/ad566d87-c566-4d64-8761-b9afcdbdcd80" />

4. Complex Scheduling: สร้าง scheduling patterns
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/5cbb456c-68e6-4309-abd0-f58db0105145" />

5. Resource Management: จัดการ timer resources
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/5e92ab11-4394-4a82-9442-300b2238f161" />

## Lab 2: Timer Applications - Watchdog & LED Patterns
### result
- การทดลองที่ 1 - 2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/97eb2ea6-51b7-45cd-a824-4fb986140f22" />

- การทดลองที่ 3
<img width="1919" height="1077" alt="image" src="https://github.com/user-attachments/assets/88c566ea-5332-4a1b-b1d2-e3cefc14f966" />

- การทดลองที่ 4
<img width="1919" height="1078" alt="image" src="https://github.com/user-attachments/assets/9e0a018f-0c61-4541-b9c1-502b8195abf1" />

- การทดลองที่ 5
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/e940b77d-54ac-4e19-89e7-fa8b377c7b68" />

### 📋 Post-Lab Questions
1. **Watchdog Design**: เหตุใดต้องใช้ separate timer สำหรับ feeding watchdog?
- เพราะการ “feed” (ให้อาหาร) watchdog ต้องเกิดขึ้นตามรอบเวลาปกติของระบบเท่านั้น
  - ถ้าเราใช้ timer เดียว ทั้งสำหรับ timeout และ feeding จะไม่สามารถแยกเหตุการณ์ “ระบบปกติ” กับ “ระบบค้าง” ได้
  - การใช้ feed timer แยกต่างหาก ทำให้ watchdog timer ทำหน้าที่ “จับเวลาว่าระบบค้างหรือไม่” โดยอิสระจากงานอื่น
  - เมื่อ feed timer ถูกหยุดหรือ delay (เช่นเกิด hang จำลองในรอบที่ 15) → watchdog timer ไม่ถูก reset → เกิด timeout และแจ้งเตือน/รีเซ็ตระบบได้
- **สรุป**: แยก timer เพื่อให้ watchdog ตรวจสอบความมีชีวิตของระบบได้อย่างอิสระ ไม่ถูกหลอกด้วยงานอื่นที่ยังรันอยู่
2. **Pattern Timing**: อธิบายการเลือก Timer Period สำหรับแต่ละ pattern
- แต่ละ pattern ใช้ระยะเวลา (period) ต่างกันเพื่อสะท้อนพฤติกรรมของแสง:

| Pattern      | Period (ms) | เหตุผล                                                 |
| ------------ | ----------- | ------------------------------------------------------ |
| `OFF`        | 1000        | ไม่มีไฟ แต่ให้ refresh ช้าเพื่อประหยัดพลังงาน          |
| `SLOW_BLINK` | 1000        | กระพริบช้า เหมาะกับสถานะ idle                          |
| `FAST_BLINK` | 200         | กระพริบเร็ว ใช้เตือน/แจ้ง event สำคัญ                  |
| `HEARTBEAT`  | 100         | ใช้ step ย่อยหลายจังหวะเพื่อจำลองชีพจร                 |
| `SOS`        | 200/600     | ใช้ dot-dash ตามรหัสมอร์ส                              |
| `RAINBOW`    | 300         | หมุนสีหลาย combination ให้เห็นเป็นลำดับเร็วแต่สม่ำเสมอ |

- **สรุป**:period ถูกเลือกตามความรู้สึก “จังหวะสายตา” และจุดประสงค์ของ pattern — ยิ่งเร็ว = สื่อถึง “active” หรือ “alert” มากขึ้น
3. **Sensor Adaptation**: ประโยชน์ของ Adaptive Sampling Rate คืออะไร?
- Adaptive Sampling ทำให้ระบบ “เปลี่ยนอัตราการเก็บข้อมูล” ตามสภาพจริง เช่น:
  - เมื่ออุณหภูมิสูง → เพิ่มความถี่ในการอ่าน (sample เร็วขึ้น) เพื่อจับความเปลี่ยนแปลงได้ละเอียด
  - เมื่ออุณหภูมิต่ำหรือคงที่ → ลดความถี่การอ่าน (sample ช้าลง) เพื่อประหยัดพลังงานและลดภาระ CPU
- ข้อดีหลัก:
  - ✅ ประหยัดพลังงานและทรัพยากร
  - ✅ ลดข้อมูลซ้ำซ้อน
  - ✅ ตอบสนองเร็วเมื่อเกิดสถานการณ์สำคัญ

- **สรุป**:Adaptive rate = “ฉลาด” กว่า fixed rate เพราะปรับตัวตามภาวะจริงของ sensor
4. **System Health**: metrics ใดบ้างที่ควรติดตามในระบบจริง?
- ในระบบ embedded หรือ IoT จริง เราควร monitor หลายมิติ:

| หมวด        | Metric ที่ควรติดตาม                | เหตุผล                              |
| ----------- | ---------------------------------- | ----------------------------------- |
| 🕒 Timer    | Active/Inactive, Timeout Count     | ตรวจสอบความเสถียรของ scheduling     |
| ⚙️ Watchdog | Feed count, Timeout count          | ชี้วัดว่าระบบยังตอบสนองหรือค้าง     |
| 🔋 Resource | Free heap, CPU load, Stack usage   | บ่งชี้ความเสี่ยงเรื่อง memory leak  |
| 🌡️ Sensor  | Valid readings, sampling rate      | บ่งบอก sensor fail หรือ signal loss |
| 💡 Pattern  | Pattern changes, LED sync          | ใช้ตรวจสอบระบบแสดงผลภายนอก          |
| 🧠 System   | Uptime, Error count, Restart count | ตรวจสอบเสถียรภาพโดยรวม              |

- **สรุป**:“System health” ควรมองหลายมุม ไม่ใช่แค่ watchdog เดียว แต่รวมถึง resource, sensor, และ performance ด้วย

### 🚀 ความท้าทายเพิ่มเติม
1. **Advanced Patterns**: สร้าง pattern ที่ซับซ้อนมากขึ้น
2. **Multi-Sensor**: เพิ่ม sensor หลายตัวพร้อม priority
3. **Network Watchdog**: ส่ง heartbeat ผ่าน network
4. **Pattern Learning**: บันทึกและเรียนรู้ pattern ที่นิยม
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/53318021-3023-4a74-9867-b8edf65d9d5a" />

## Lab 3: Counting Semaphores
### result
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/02f3c64e-3c70-421a-abee-45734b501a4e" />

### 📋 Advanced Analysis Questions
1. Service Task Priority: ผลกระทบของ Priority ต่อ Timer Accuracy?
- High Priority = High Accuracy: เมื่อ Timer Service Task มี priority สูงกว่า tasks ทั่วไป callback จะถูกรันใกล้เวลาที่ตั้งไว้ delay (jitter) จะต่ำกว่า 1–2 tick เท่านั้น
- Low Priority = Accumulated Drift: หากตั้ง priority ต่ำ callback อาจถูก preempt บ่อย โดยเฉพาะเมื่อ CPU load สูง ทำให้ Timer accuracy ลดลง และ periodic task เกิด phase shift
- ✅ แนวทาง:
  - Timer Service Task ≥ priority ของ application task ที่ใช้ callback
  - ใช้ configTIMER_TASK_PRIORITY ใน FreeRTOSConfig.h ตั้งอย่างน้อย 3 หรือตามระดับ real-time ที่ต้องการ
  - สำหรับ ESP-IDF v5.x ถ้าใช้ esp_timer (ใช้ hardware microsecond timer) ไม่ต้องปรับ priority มากนัก เพราะ มัน อยู่ ใน ISR context อยู่แล้ว
2. Callback Performance: วิธีการเพิ่มประสิทธิภาพ Callback Functions?
- Avoid Blocking: ห้าม vTaskDelay() หรือ loop นาน ใน callback; ส่ง event ผ่าน queue ไปยัง worker task แทน
- Minimize Workload: แยก callback ให้เหลือเพียง “notification + timestamp capture” ส่วน งานหลัก คำนวณ ใน task ภายนอก
- Use Static Memory: หลีกเลี่ยง malloc() ใน callback โดยเตรียม static buffer ล่วงหน้า เพื่อ ลด heap fragmentation
- Profile Execution Time: ใช้ esp_timer_get_time() รอบ callback เพื่อวัด latency และ ปรับ period หรือ WCET ใน Adaptive Controller
3. Memory Management: กลยุทธ์การจัดการ Memory สำหรับ Dynamic Timers?
- Static Pool First: สร้าง timer object จาก static array (StaticTimer_t) ก่อน ใช้ dynamic allocation
- Dynamic แต่ Bounded: ถ้าต้องสร้าง/ลบ timer บ่อย ใช้ pvTimerGetTimerID() เช็กสถานะ reuse object แทนการสร้างใหม่
- Periodic vs One-shot: แยก pool สองแบบ เพื่อลด fragmentation ของ heap
- Monitor Stack/Heap: ใช้ uxTaskGetStackHighWaterMark() และ heap_caps_get_free_size() เช็ก memory pressure เพื่อ trigger adaptive timer ลดจำนวน active timers อัตโนมัติ
4. Error Recovery: วิธีการ Handle Timer System Failures?
- Watchdog Integration: หาก Timer Service Task hang ให้ มี Watchdog reset หรือ restart timer pool
- Redundant Timers: สำหรับ critical task สร้าง dual timer ที่ backup กัน (เมื่อ ตัวหลัก ไม่ callback ภายใน timeout backup ทำงานแทน)
- Auto-Recreate: เช็ก xTimerIsTimerActive() ; หาก false ทั้งที่ ควร active → xTimerStartFromISR() ใหม่
- Central Error Log: log event เช่น timer_overrun, callback_miss, queue_full เพื่อ วิเคราะห์ หลังบ้าน ใน production
5. Production Deployment: การปรับแต่งสำหรับ Production Environment?
- Fine-Tune Tick Rate: ตั้ง configTICK_RATE_HZ = 1000 ถ้าต้องการ 1 ms granularity; สำหรับ power-save ลดเหลือ 100–250 Hz
- Pin Timer Tasks to Core: ใน ESP32 ใช้ xTaskCreatePinnedToCore() ให้ Timer/Service Task รันบน Core 0 ส่วน App Task บน Core 1 ลด interrupt jitter
- Use esp_timer แทน Software Timer: ถ้าต้อง accuracy ระดับ μs หรือ มี จำนวน timer มาก (esp_timer ใช้ hardware interrupt และ min jitter < 20 µs)
- Adaptive Control Enable: เปิด controller เฉพาะใน production (เช่น “auto-tune mode”) พร้อม log CSV ออก UART หรือ MQTT เพื่อ วิเคราะห์ trend
- Fail-Safe Mode: ถ้า utilization > 95% และ miss > 10% → switch ระบบ เข้า safe-config เช่น เพิ่ม period ทุกงาน 20% อัตโนมัติ

### 🚀 ความท้าทายระดับผู้เชี่ยวชาญ
- Challenge 1: Real-time Scheduler
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/173d06dd-47cc-4e9e-9fcf-b774ced49891" />

- Challenge 2: Distributed Timer System
<img width="1907" height="1079" alt="image" src="https://github.com/user-attachments/assets/0bd97072-0097-4f3e-9b8c-5c85812fd5b2" />

- Challenge 3: Adaptive Performance
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/04434aa4-480e-4b4f-ad63-14ecfe2721ce" />
