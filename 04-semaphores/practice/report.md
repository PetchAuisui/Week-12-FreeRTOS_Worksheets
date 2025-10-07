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

## Lab 3: Counting Semaphores 
### result
- การทดลองที่ 1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/e14076b5-7a67-4189-a0bc-dcbb6a0e3ed7" />

- การทดลองที่ 2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/a783a59b-eb2d-4cc2-bd4e-f051cf224a8a" />

- การทดลองที่ 3
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/d6c79938-87f7-4bc6-8b4c-7e05efb83846" />

### 📊 การสังเกตและบันทึกผล
| ทดลอง          | Resources | Producers |            Success Rate (%) | Avg Wait (ms) | Resource Utilization (%) | หมายเหตุ                                       |
| :------------- | --------: | --------: | --------------------------: | ------------: | -----------------------: | :--------------------------------------------- |
| **1 (3R, 5P)** |         3 |         5 |                   **100 %** |        ≈ 1690 |                   ≈ 95 % | ทรัพยากรถูกใช้เต็มเกือบตลอด ไม่มี timeout      |
| **2 (5R, 5P)** |         5 |         5 |                   **100 %** |         ≈ 400 |                ≈ 60–70 % | มีทรัพยากรว่างเกือบตลอด ใช้ไม่เต็ม pool        |
| **3 (3R, 8P)** |         3 |         8 | **≈ 100 % (บางช่วง delay)** |        ≈ 2700 |                  ≈ 100 % | มีการรอคิวนานขึ้น / ใช้ทรัพยากรเต็มแทบตลอดเวลา |


### คำถามสำหรับการทดลอง
1. เมื่อ Producers มากกว่า Resources จะเกิดอะไรขึ้น?
- เมื่อจำนวน Producers (ผู้ร้องขอทรัพยากร) มากกว่าจำนวน Resources (ทรัพยากร) ที่มีอยู่ในระบบ:
  - จะเกิด Resource Contention หรือ “การแย่งชิงทรัพยากร”
  - Producers ที่ขอทรัพยากรในขณะที่ไม่มี resource ว่างจะ ถูกบล็อก (blocked) รอจนกว่าจะมีการคืน resource กลับเข้ามาใน pool
  - ใน log จะเห็นข้อความว่า
    ```
    ⏰ ProducerX: Timeout waiting for resource
    ```
    ถ้าเวลารอนานเกินไป (timeout)
   - LED ของ Resource จะติดเฉพาะช่วงที่มีการใช้งานอยู่ และ Producers ที่ไม่ได้ resource จะไม่มี LED ใดเปลี่ยนสถานะ
- **สรุป**: เมื่อ Producers มากกว่า Resources → เกิดการรอ (blocking) และ Success Rate ลดลง เพราะมีบาง task ที่ไม่ได้ resource ภายในเวลาที่กำหนด
2. Load Generator มีผลต่อ Success Rate อย่างไร?
- Load Generator จะสร้าง “ภาระโหลดชั่วขณะ (burst load)” โดยทำให้มีการขอทรัพยากรจำนวนมากในระยะเวลาสั้น ๆ
- ผลที่ตามมา:
  - Success Rate จะลดลงชั่วคราว เพราะ resource ถูกใช้จนเต็ม
  - บาง Producers จะเกิด timeout เนื่องจากไม่มี resource เหลือใน pool
  - ใน log จะเห็นข้อความเช่น:
  ```
  🚀 LOAD GENERATOR: Creating burst of requests...
  ⚠️ LoadGen: Resource pool exhausted
  ```
  - LED_SYSTEM จะติดขึ้นเพื่อแสดงช่วงเวลาที่ระบบมีโหลดสูง
- **สรุป**:Load Generator ทำให้เกิด ภาวะ overload ชั่วขณะ ซึ่งช่วยทดสอบความทนทานของระบบภายใต้การใช้งานหนัก และทำให้ Success Rate ลดลงในช่วงนั้น
3. Counting Semaphore จัดการ Resource Pool อย่างไร?
- Counting Semaphore ทำหน้าที่เป็น ตัวนับจำนวนทรัพยากรที่เหลืออยู่ (Available Resources Counter)
- โดยมีหลักการดังนี้:

| **ฟังก์ชัน** | **คำอธิบายการทำงาน** |
|---------------|--------------------------|
| `xSemaphoreCreateCounting(max, max)` | สร้าง Counting Semaphore ที่มีค่าเริ่มต้นเท่ากับจำนวนทรัพยากรทั้งหมด เพื่อใช้ควบคุมการเข้าถึงทรัพยากรที่จำกัด |
| `xSemaphoreTake()` | ลดค่าของ Semaphore ลง 1 เมื่อมีการขอใช้ทรัพยากร ถ้าค่าเหลือ 0 จะทำให้ Task ที่ร้องขอถูกบล็อก (รอ) จนกว่าจะมีการคืนทรัพยากร |
| `xSemaphoreGive()` | เพิ่มค่าของ Semaphore ขึ้น 1 เมื่อมีการคืนทรัพยากรกลับเข้าสู่ระบบ เพื่อให้ Task อื่นสามารถเข้ามาใช้ได้ต่อ |
| `uxSemaphoreGetCount()` | ใช้ตรวจสอบจำนวนทรัพยากรที่ยังเหลืออยู่ในปัจจุบันภายในระบบ |

- ด้วยกลไกนี้ Counting Semaphore จะทำหน้าที่เหมือน “ระบบจองทรัพยากร”
  - ป้องกันไม่ให้มีการใช้ resource เกินจำนวนที่มีจริง
  - ควบคุมลำดับการเข้าถึงอย่างปลอดภัยและเป็นระบบ
- **สรุป**:Counting Semaphore ทำให้ Resource Pool มีความเป็นระเบียบ (controlled sharing) โดยไม่เกิดการใช้ทรัพยากรเกินขนาด และช่วยให้หลาย task ใช้งานร่วมกันได้อย่างปลอดภัย
