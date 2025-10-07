# บันทึกผลการทดลอง
## Lab 1: Basic Queue Operations
### result
- ทดลองที่ 1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/d7e1ea18-aa13-46f3-b718-0ccd64f79fec" />

- ทดสอบที่ 2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/81dec57a-6b1d-4a2f-98b7-441be46c74cf" />

- ทดสอบที่ 3
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/2609718f-6b6c-49ea-a4f4-7ebf502b25fd" />

-🔧 การปรับแต่งเพิ่มเติม
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/b5fe846e-6dc7-4766-97e4-8e583fbea5f8" />

## Lab 2: Producer-Consumer System
### result
- ทดลองที่ 1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/5a7194f8-9d06-4d53-b8d2-f5076d82e064" />

- ทดลองที่ 2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/85a76718-2851-4046-a9a1-a637740539bd" />

- ทดลองที่ 3
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/ce65cfc1-0d9e-4df1-8aff-635e738d6554" />

- 🔧 การปรับแต่งเพิ่มเติม
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/4c40536c-4264-496a-b39b-cbb8173a2e72" />

## Lab 3: Queue Sets Implementation
### result
- ทดลองที่ 1
<img width="1919" height="1074" alt="image" src="https://github.com/user-attachments/assets/97b2072a-46e1-47f6-b2bc-cb4274be0af2" />

- ทดลองที่ 2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/4b3aa8a3-0bad-4199-a0ef-82fe46780961" />

- ทดลองที่ 3
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/f23fd576-276e-48f9-9adf-d64b6e585a60" />

### 📊 การสังเกตและบันทึกผล
| ทดลอง                |       Sensor      |        User       |      Network      |    Timer    |            Total            | สังเกต                                                                                                                |
| :------------------- | :---------------: | :---------------: | :---------------: | :---------: | :-------------------------: | :-------------------------------------------------------------------------------------------------------------------- |
| **1 (ปกติ)**         | ทำงานทุก 2–5 วิ | ทำงานทุก 3–8 วิ | ทำงานทุก 1–4 วิ | ทุก 10 วิ |  ประมาณ 60–80 ข้อความ/นาที  | ทุก Queue ทำงานปกติ LED แต่ละดวงกะพริบเป็นช่วง ๆ ไม่มีการค้าง                                                         |
| **2 (ไม่มี Sensor)** |   ปิดการทำงาน   |         ✅         |         ✅         |      ✅      |  ประมาณ 40–60 ข้อความ/นาที  | Processor ยังคงทำงานปกติ รอจาก Queue อื่น ไม่มี Error LED Sensor ไม่ติดเลย                                            |
| **3 (Network เร็ว)** |         ✅         |         ✅         |  ถี่ทุก 0.5 วิ  |      ✅      | ประมาณ 150–200 ข้อความ/นาที | Network ข้อมูลเยอะมาก Processor ทำงานตลอด มีบางช่วง Queue เต็ม / เริ่มเห็น delay หรือ drop เล็กน้อย แต่ระบบยังไม่ค้าง |

