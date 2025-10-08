<img width="1919" height="1077" alt="image" src="https://github.com/user-attachments/assets/587eac66-c810-4a7f-85d6-9336f34e444f" /># บันทึกผลการทดลอง
## Lab 1: Basic Software Timers
### result
- การทดลองที่ 1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/66268e25-349c-414e-a885-9b9357e583fc" />

- การทดลองที่ 2
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
