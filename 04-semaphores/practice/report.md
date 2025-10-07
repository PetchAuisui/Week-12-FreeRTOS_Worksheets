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

## Lab 2: Mutex and Critical Sections
### result
- การทดลองที่ 1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/18ba0c79-f01b-42f9-963c-9fae042e5a2d" />

- การทดลองที่ 2
<img width="1919" height="1079" alt="Screenshot 2025-10-07 224053" src="https://github.com/user-attachments/assets/6c463de4-2db1-4358-ae07-fc88c378ad8c" />

- การทดลองที่ 3
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/fc8926b4-60c8-4fd0-9f01-d4a559881d4c" />


### 📊 การสังเกตและบันทึกผล
| ทดลอง                | Successful | Failed | Corrupted | Success Rate            | สังเกต                                                                                         |
| -------------------- | ---------- | ------ | --------- | ----------------------- | ---------------------------------------------------------------------------------------------- |
| 1 (With Mutex)       | 16         | 0      | 0         | 100%                    | ระบบปกติ ไม่มี data corruption; Mutex ป้องกันได้สมบูรณ์                                        |
| 2 (No Mutex)         | 28         | 0      | 0         | 100%                    | ยังไม่เกิด race condition ในช่วงสั้น แต่ไม่มีการป้องกันเลย                                     |
| 3 (Changed Priority) | 42         | 0      | 3         | 100% (แต่มี corruption) | เกิด **data corruption** จาก race condition เพราะ low-priority task แย่ง access ก่อน task อื่น |


### คำถามสำหรับการทดลอง
1. เมื่อไม่ใช้ Mutex จะเกิด data corruption หรือไม่?
- เกิดได้ เพราะหลาย task เข้าถึง shared resource พร้อมกัน โดยไม่มีการป้องกัน
ทำให้เกิด Race Condition — ค่าในตัวแปรหรือ buffer อาจถูกเขียนทับกันระหว่างที่อีก task กำลังใช้งานอยู่
ผลคือค่าที่ได้ไม่ถูกต้อง หรือ checksum mismatch (ตรวจพบใน log ว่า `⚠️ DATA CORRUPTION DETECTED!`)
2. Priority Inheritance ทำงานอย่างไร?
- เมื่อ low-priority task ถือ Mutex อยู่ แล้ว high-priority task ต้องการ Mutex เดียวกัน
ระบบ FreeRTOS จะ “ยก priority” ของ low-priority task ชั่วคราวให้เท่ากับ high-priority
เพื่อให้มันปล่อย Mutex เร็วขึ้น ป้องกันไม่ให้เกิด Priority Inversion
เมื่อปล่อย Mutex แล้ว priority จะกลับไปค่าปกติทันที
3. Task priority มีผลต่อการเข้าถึง shared resource อย่างไร?
- Task ที่มี priority สูงกว่า จะได้รับ CPU ก่อน และมีโอกาส เข้าถึง critical section ได้บ่อยกว่า
แต่หากใช้ Mutex อย่างถูกต้อง ระบบจะให้สิทธิ์เข้าตามลำดับการขอ (FIFO) เพื่อความยุติธรรม
ถ้า priority ถูกตั้งไม่เหมาะสม อาจทำให้ low-priority task ไม่ค่อยได้เข้าถึง resource หรือเกิด starvation ได้
