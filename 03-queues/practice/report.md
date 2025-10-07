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

### 📊 การสังเกตและบันทึกผล
| ทดลอง | Sender Rate | Receiver Rate | Queue Status | สังเกต |
|-------|-------------|---------------|--------------|---------|
| 1 | 2s | 1.5s | Normal | เห็น Messages 1–3 ตลอด, LED กะพริบสม่ำเสมอ|
| 2 | 0.5s | 1.5s | Full | Messages ค้างที่ 5, ขึ้น Warning “Queue full!”|
| 3 | 2s | 0.1s | Empty | Messages = 0, Free spaces = 5, มี log “No message received”|

### คำถามสำหรับการทดลอง
1. เมื่อ Queue เต็ม การเรียก `xQueueSend` จะเกิดอะไรขึ้น?
- เมื่อเพิ่มผู้ผลิต (Producer) คนที่ 4 เข้ามา ระบบจะมีจำนวน Producer มากกว่า Consumer ซึ่งหมายความว่า “อัตราการผลิต” จะสูงกว่า “อัตราการบริโภค”
  - Queue จะเริ่ม สะสมสินค้า มากขึ้น เพราะ Consumer ไม่สามารถรับได้ทัน
  - เมื่อ Queue เต็ม (xQueueSend() คืนค่า pdFAIL) จะเกิดข้อความ `Queue full! Dropped ...` ใน log
  - ค่าในตัวแปร `global_stats.dropped` จะเพิ่มขึ้นเรื่อย ๆ
  - Load Balancer Task จะตรวจพบว่า `uxQueueMessagesWaiting()` มีค่าเกิน MAX_QUEUE_SIZE (8) แล้วจะแสดงข้อความเตือน
```
⚠️ HIGH LOAD DETECTED! Queue size: 9
💡 Suggestion: Add more consumers or optimize processing
```
  - ประสิทธิภาพระบบ (`Efficiency = Consumed / Produced × 100%`) จะ ลดลง เพราะมีสินค้าที่ถูก drop มากขึ้น
2. เมื่อ Queue ว่าง การเรียก `xQueueReceive` จะเกิดอะไรขึ้น?
- เมื่อเหลือ Consumer เพียงคนเดียว ระบบจะมี “คอขวด (bottleneck)” ที่ฝั่ง Consumer
  - Consumer ที่เหลือต้องรับงานทั้งหมดจาก Producers 3 คน
  - การประมวลผล (processing) จะใช้เวลานานขึ้น ทำให้ Queue มี backlog
  - LED ของ Consumer ดวงเดียวจะติดนานมาก (เพราะใช้เวลาประมวลผลของหลายสินค้า)
  - ผลที่เห็นใน Serial Monitor:
  ```
  ⚠️ HIGH LOAD DETECTED! Queue size: 10
  ⏰ Consumer 1: No products to process (timeout)
  ✗ Producer 2: Queue full! Dropped Product-P2-#56
  ```
  - สินค้าบางส่วนถูก drop เพราะ Queue เต็มเป็นประจำ
  - ค่า Efficiency จะต่ำกว่าทดลองที่ 1 อย่างเห็นได้ชัด
3. ทำไม LED จึงกะพริบตามการส่งและรับข้อความ?
```
const int MAX_QUEUE_SIZE = 8;
if (queue_items > MAX_QUEUE_SIZE) {
    safe_printf("⚠️  HIGH LOAD DETECTED! Queue size: %d\n", queue_items);
}
```
- ดังนั้น Load Balancer จะเริ่มแสดงการแจ้งเตือนเมื่อจำนวนสินค้าที่รอใน Queue (uxQueueMessagesWaiting(xProductQueue)) เกิน 8 รายการ
  - เมื่อถึงจุดนี้ หมายถึงระบบมีภาระมากกว่าที่จะประมวลผลได้ทัน
  - จะมีการแสดงข้อความเตือนและ “กระพริบ LED ทุกดวง” เพื่อให้สังเกตได้ชัดเจนทางกายภาพ
  - การแจ้งเตือนจะเกิดขึ้นบ่อยในสถานการณ์ Producer มากกว่า Consumer
    
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

### 📊 การสังเกตและบันทึกผล
| ทดลอง | Producers | Consumers | Produced | Consumed | Dropped | Efficiency |
|-------|-----------|-----------|----------|----------|---------|------------|
| 1 | 3 | 2 | 3| 2| 0| 66.7%|
| 2 | 4 | 2 | 4| 2| 0| 50.0%|
| 3 | 3 | 1 | 3| 1| 0| 33.3%|

### คำถามสำหรับการทดลอง
1. ในทดลองที่ 2 เกิดอะไรขึ้นกับ Queue?
-เมื่อเพิ่มผู้ผลิต (Producer) คนที่ 4 เข้ามา จำนวนงานที่ถูกส่งเข้าสู่ Queue ต่อวินาทีมากขึ้น ในขณะที่จำนวนผู้บริโภค (Consumer) ยังคงเท่าเดิม (2 คน)
- ผลที่เกิดขึ้นคือ:
  - Queue เต็มบ่อย (Queue full! Dropped ...)
  - มีสินค้าที่ถูก drop เพราะ Consumer ทำงานไม่ทัน
  - ค่า global_stats.dropped เพิ่มขึ้นต่อเนื่อง
  - Load Balancer ตรวจจับ “High Load” และแสดงคำเตือน
  - ประสิทธิภาพ (Efficiency) ลดลงอย่างชัดเจน
2. ในทดลองที่ 3 ระบบทำงานเป็นอย่างไร?
- เมื่อปิด Consumer 2 ออก เหลือเพียง Consumer เดียว → ระบบจะทำงานช้าลงและเกิดภาวะคอขวด (bottleneck) มากกว่าเดิม
- ผลที่สังเกตได้:
  - Queue เต็มตลอดเวลา
  - Producer หลายตัวส่งสินค้าไม่สำเร็จ (Queue full! Dropped ...)
  - Efficiency ลดลงต่ำกว่า 70%
  - LED ของ Consumer ติดค้างนาน (ประมวลผลต่อเนื่อง)
  - มี log เตือน High Load บ่อยครั้งจาก Load Balancer
3. Load Balancer แจ้งเตือนเมื่อไหร่?
- Load Balancer จะตรวจเช็กจำนวนสินค้าที่รอใน Queue ทุก 1 วินาที โดยมีเงื่อนไขดังนี้:
```
const int MAX_QUEUE_SIZE = 8;
if (queue_items > MAX_QUEUE_SIZE) {
    safe_printf("⚠️ HIGH LOAD DETECTED!");
}
```
- ถ้า Queue มีสินค้ารอมากกว่า 8 รายการ จะเกิดการแจ้งเตือน
- ทุกครั้งที่เกิดการแจ้งเตือน จะเห็น LED ทุกดวงกระพริบพร้อมกัน เพื่อบอกว่าระบบเริ่ม overload แล้ว
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

### คำถามสำหรับการทดลอง
1. Processor Task รู้ได้อย่างไรว่าข้อมูลมาจาก Queue ไหน?
- เพราะ xQueueSelectFromSet() จะ ส่งกลับ handle ของสมาชิกที่ “มีเหตุการณ์” (queue หรือ binary semaphore) นั้น ๆ
ในโค้ดคุณจึงเทียบ pointer ได้ตรง ๆ:
```
QueueSetMemberHandle_t m = xQueueSelectFromSet(xQueueSet, portMAX_DELAY);

if (m == xSensorQueue) { /* รับ sensor_data */ }
else if (m == xUserQueue) { /* รับ user_input */ }
else if (m == xNetworkQueue) { /* รับ network_message */ }
else if (m == xTimerSemaphore) { /* take semaphore ของ timer */ }
```
2. เมื่อหลาย Queue มีข้อมูลพร้อมกัน เลือกประมวลผลอันไหนก่อน?
- พฤติกรรมมาตรฐานของ FreeRTOS คือ ไม่รับประกันลำดับ ถ้าหลายสมาชิกพร้อมพร้อมกัน—ขึ้นกับลำดับภายใน/เวลาที่ event เกิดและ scheduler ตัดสินใจ (จึงอย่าพึ่ง “คอนเซ็ปต์ลำดับใน set” เพื่อทำ priority)
- ถ้าต้องการ “ลำดับชัดเจน/มี priority เอง” แนะนำแนวทาง:
  - วิธีง่าย: หลัง xQueueSelectFromSet() ให้ เช็คคิวตามลำดับความสำคัญเอง และ xQueueReceive(..., 0) แบบ non-blocking
  ```
  QueueSetMemberHandle_t m = xQueueSelectFromSet(xQueueSet, portMAX_DELAY);
  
  // ลำดับที่อยากให้มาก่อน: Network > User > Sensor > Timer
  if (uxQueueMessagesWaiting(xNetworkQueue) && xQueueReceive(xNetworkQueue, &net, 0) == pdPASS) { ... }
  else if (uxQueueMessagesWaiting(xUserQueue) && xQueueReceive(xUserQueue, &user, 0) == pdPASS) { ... }
  else if (uxQueueMessagesWaiting(xSensorQueue) && xQueueReceive(xSensorQueue, &sen, 0) == pdPASS) { ... }
  else if (xSemaphoreTake(xTimerSemaphore, 0) == pdPASS) { ... }
  ```
  - แยก queue set ตามระดับความสำคัญ แล้วให้ processor เช็ค set สูงก่อนไปต่ำ
  - หรือใช้ task คนละตัว ต่อ set แล้วกำหนด task priority ต่างกัน
3. Queue Sets ช่วยประหยัด CPU อย่างไร?
- ปกติถ้าไม่มี Queue Sets คุณต้อง poll หลายคิว (ตื่นขึ้นมาดูทีละคิวซ้ำ ๆ) → เสีย CPU/เกิด context switch บ่อย
- ด้วย Queue Sets, processor task block อยู่กับ object เดียว (xQueueSelectFromSet) จนกว่าจะมีเหตุการณ์จาก “คิวใดก็ได้” → ไม่มี busy-wait, ปล่อย CPU ให้งานอื่น/idle (และบน ESP-IDF ช่วยประหยัดพลังงานได้)
- ลดจำนวน task ที่ต้อง “เฝ้าคิว” หลายตัว → ลด context switch และ logging ทับกัน
