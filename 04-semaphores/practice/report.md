# บันทึกผลการทดลอง
## Lab 1: Binary Semaphores
### result
- การทดลองที่ 1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/786f2fb7-30e8-4ffb-a45d-e42e7f8d2d2b" />

- การทดลองที่ 2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/40df343d-f0d8-4b6c-a344-292db2bbec20" />

- การทดลองที่ 3
<img width="1919" height="1075" alt="image" src="https://github.com/user-attachments/assets/1f7ffc4b-b12b-4664-ae5f-2e49e33c554c" />

### 📊 การสังเกตและบันทึกผล
### ตารางบันทึกผล
| ทดลอง | Events Sent | Events Received | Timer Events | Button Presses | Efficiency |
|-------|-------------|-----------------|--------------|----------------|------------|
| 1 (Normal) | 3| 3| 1| 0| 100%|
| 2 (Multiple Give) | 5| 5| 1| 0| 100%|
| 3 (Short Timeout) | 8| 8| 2| 0| 100%|


### คำถามสำหรับการทดลง
1. เมื่อ give semaphore หลายครั้งติดต่อกัน จะเกิดอะไรขึ้น?
- Binary Semaphore มีค่าได้เพียง 0 หรือ 1 เท่านั้น
ดังนั้นการ xSemaphoreGive() ซ้ำ ๆ โดยที่ยังไม่ได้ xSemaphoreTake() จะไม่สะสมค่าเพิ่มขึ้น
ผลคือมีผลเท่ากับการ give เพียงครั้งเดียว เหตุการณ์ซ้ำจะถูกละทิ้ง (drop)
2. ISR สามารถใช้ `xSemaphoreGive` หรือต้องใช้ `xSemaphoreGiveFromISR`?
- ไม่ได้ ต้องใช้ xSemaphoreGiveFromISR() เท่านั้น
เพราะฟังก์ชันนี้ถูกออกแบบให้ปลอดภัยใน interrupt context
และสามารถปลุก task ที่รอ semaphore ได้อย่างถูกต้องโดยไม่ทำให้ระบบค้าง
3. Binary Semaphore แตกต่างจาก Queue อย่างไร?
- Binary Semaphore ใช้เพื่อ “ซิงโครไนซ์เหตุการณ์” หรือแจ้งว่า “บางสิ่งเกิดขึ้นแล้ว”
ส่วน Queue ใช้เพื่อ “ส่งข้อมูล” ระหว่าง task หรือจาก ISR ไปยัง task อื่น
Semaphore ไม่มีการเก็บข้อมูล มีเพียงสัญญาณ 0/1 ขณะที่ Queue มี buffer สำหรับเก็บข้อมูลหลายรายการ
