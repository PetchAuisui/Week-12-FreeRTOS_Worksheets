# บันทึกผลการทดลอง
## Lab 1: Task Priority และ Scheduling
### Exercise-1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/f1a63ec5-3b9d-4011-8f67-3acfa07bf5ab" />

### Exercise-2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/576e1040-6593-4619-aceb-591f5aca6667" />

### คำถามสำหรับวิเคราะห์
1. Priority ไหนทำงานมากที่สุด? เพราะอะไร?
- Task ที่มี Priority สูงสุด (Priority 5) ทำงานมากที่สุด
- เพราะ FreeRTOS ใช้ Priority-based Preemptive Scheduling — หมายความว่า เมื่อมี task ที่มี priority สูงพร้อมทำงาน ระบบจะ “ขัดจังหวะ” (preempt) task ที่มี priority ต่ำกว่า เพื่อให้ task นั้นได้ CPU ก่อนเสมอ
2. เกิด Priority Inversion หรือไม่? จะแก้ไขได้อย่างไร?
- อาจเกิด Priority Inversion ได้ เมื่อ
  - Task priority ต่ำครอบครอง resource (เช่น mutex)
  - แล้ว task priority สูงต้องการใช้ resource เดียวกัน แต่ถูก block รอ
  - ขณะที่ task medium priority แทรกเข้ามาทำงาน ทำให้ task high ต้องรอนาน
- วิธีแก้ไข:ใช้ Priority Inheritance Mechanism — เมื่อ task priority ต่ำถือ mutex อยู่ แล้ว task priority สูงรอใช้ ระบบจะ “ยก priority” ของ task ต่ำชั่วคราวให้เท่ากับของ task สูง เพื่อให้มันคืน resource ได้เร็วขึ้น
3. Tasks ที่มี priority เดียวกันทำงานอย่างไร?
- Tasks ที่มี priority เท่ากันจะใช้ Round-Robin Scheduling คือ FreeRTOS จะสลับให้แต่ละ task ได้ CPU ตามลำดับ โดยใช้ time slice (tick) เป็นตัวแบ่งเวลา ทำให้ทุก task ได้ทำงานอย่างเท่าเทียมกัน
4. การเปลี่ยน Priority แบบ dynamic ส่งผลอย่างไร?
- การเปลี่ยน Priority ระหว่าง runtime (เช่น จาก 1 → 4 → 1) ทำให้ task ที่เดิมมี priority ต่ำสามารถแทรกเข้าทำงานได้ก่อนในช่วงที่ถูก “boost”
- ผลคือเกิดการเปลี่ยนลำดับการทำงานแบบชั่วคราว ซึ่งเหมาะสำหรับการจัดการงานเร่งด่วน (temporary critical tasks)
5. CPU utilization ของแต่ละ priority เป็นอย่างไร?
- ขึ้นกับจำนวนรอบการทำงาน แต่โดยทั่วไปจะเป็นไปตามนี้:

| Priority   | ลักษณะ                                 | CPU Utilization โดยประมาณ |
| ---------- | -------------------------------------- | ------------------------- |
| High (5)   | ทำงานบ่อยสุด ถูก preempt น้อย          | ~40–50%                   |
| Medium (3) | ถูกแทรกบ่อย แต่ยังได้เวลาทำงานบ้าง     | ~25–35%                   |
| Low (1)    | ทำงานน้อยสุด เพราะรอ task สูงเสร็จก่อน | ~15–25%                   |

## Lab 2: Task States Demonstration
### Exercise-1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/72a5c3c3-f4ea-4e5b-bd1a-98aaefcb6a4e" />

### Exercise-2
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/4ce261e2-d716-409c-b315-b7705a7d47aa" />

### คำถามสำหรับวิเคราะห์
1. Task อยู่ใน Running state เมื่อไหร่บ้าง?
- Task จะอยู่ใน Running state เมื่อมันได้รับ CPU และกำลังถูก Scheduler สั่งให้ทำงานจริง ๆ
- เช่น ตอนที่ไม่มี task ที่มี priority สูงกว่ามาแย่ง หรือเมื่อถึงคิวของ task นั้นในกรณีที่ priority เท่ากัน
- ตัวอย่าง: ตอนในโค้ดที่ task อยู่ในลูป for() หรือระหว่างที่กำลังประมวลผลคำสั่ง (LED GPIO2 จะสว่าง)
2. ความแตกต่างระหว่าง Ready และ Blocked state คืออะไร?

| ลักษณะ           | Ready                                | Blocked                                |
| ---------------- | ------------------------------------ | -------------------------------------- |
| ความหมาย         | พร้อมจะทำงานแต่ยังไม่ได้ CPU         | รอ event หรือ resource บางอย่าง        |
| ตัวอย่าง         | Task ถูก preempt หรือรอคิว scheduler | Task รอ semaphore, delay, หรือ I/O     |
| การเปลี่ยน state | Scheduler เรียกให้ทำงาน → Running    | Event เกิดขึ้นหรือ timeout หมด → Ready |

- สรุปสั้น ๆ:
  - Ready = “พร้อมจะวิ่ง”,
  - Blocked = “ยังวิ่งไม่ได้ เพราะต้องรออะไรบางอย่าง”
3. การใช้ vTaskDelay() ทำให้ task อยู่ใน state ใด?
- เมื่อเรียก vTaskDelay() task จะเข้าสู่ Blocked stateเพราะมันจะ “หลับ” เป็นระยะเวลาหนึ่ง (ตามค่า delay) และถูกปลุกโดย tick interrupt ของ FreeRTOS หลังหมดเวลา
- ระหว่างนั้น scheduler จะให้ CPU ไปทำงานกับ task อื่น
4. การ Suspend task ต่างจาก Block อย่างไร?

| ลักษณะ           | Suspended                                 | Blocked                                  |
| ---------------- | ----------------------------------------- | ---------------------------------------- |
| วิธีเข้า state   | ถูกสั่งด้วย API เช่น `vTaskSuspend()`     | รอ event, semaphore หรือ delay           |
| วิธีออกจาก state | ต้องถูกเรียก `vTaskResume()` โดย explicit | ออกจากการรอ event หรือ timeout หมด       |
| การกลับมา        | ต้องให้ task อื่น resume ให้              | Automatic เมื่อ event เกิดหรือ delay หมด |

- กล่าวคือ Suspend = ถูกหยุดโดยเจตนา (manual) ส่วน Blocked = หยุดเพราะรออะไรบางอย่าง (automatic)
5. Task ที่ถูก Delete จะกลับมาได้หรือไม่?
- ไม่สามารถกลับมาได้เมื่อ task ถูกเรียก vTaskDelete() แล้ว หน่วยความจำของ stack และ TCB (Task Control Block) จะถูกคืนให้ระบบ
ถ้าต้องการให้ task ทำงานอีก ต้อง สร้างใหม่ด้วย xTaskCreate() เท่านั้น

## Lab 3: Stack Monitoring และ Debugging
### Exercise-1
<img width="1919" height="1079" alt="image" src="https://github.com/user-attachments/assets/d8d8539d-73ba-4359-ad81-b85112da5c53" />

### Exercise-2
<img width="1918" height="1079" alt="image" src="https://github.com/user-attachments/assets/cbfdc433-948c-43ef-841c-43b28a6f0fd3" />

### คำถามสำหรับวิเคราะห์
1. Task ไหนใช้ stack มากที่สุด? เพราะอะไร?
- Task ที่ใช้ stack มากที่สุดคือ Heavy Task
- เพราะภายใน task มีการประกาศ local arrays ขนาดใหญ่ เช่น char large_buffer[1024], int large_numbers[200], และ char another_buffer[512]
- ซึ่งทั้งหมดถูกเก็บใน stack memory ของ task นั้นโดยตรง จึงทำให้เกิดการใช้ stack สูงและมีความเสี่ยงต่อ stack overflow มากที่สุด
2. การใช้ heap แทน stack มีข้อดีอย่างไร?
- การใช้ heap memory (malloc/free) แทน stack สำหรับข้อมูลขนาดใหญ่มีข้อดีคือ:
  - ลดการใช้ stack ของแต่ละ task
  - สามารถจัดสรรและคืนหน่วยความจำแบบ dynamic ได้
  - ป้องกัน stack overflow จากการใช้ตัวแปรภายใน (local variables) มากเกินไป
  - เหมาะกับข้อมูลที่มีขนาดไม่แน่นอนหรือเปลี่ยนแปลงระหว่าง runtime
- ตัวอย่าง: optimized_heavy_task ใช้ heap ทำให้ stack เหลือมากขึ้นและไม่เกิด warning
3. Stack overflow เกิดขึ้นเมื่อไหร่และทำอย่างไรป้องกัน?
- Stack overflow เกิดขึ้นเมื่อ task ใช้ stack เกินขนาดที่กำหนดตอนสร้าง (xTaskCreate) เช่น:
  - มีการใช้ local variable ขนาดใหญ่เกิน
  - มีการเรียก recursive function ลึกเกินไป
  - หรือมี interrupt/function call ซ้อนหลายชั้น
- วิธีป้องกัน:
  1. กำหนด stack size ให้เหมาะสมตามลักษณะของ task
  2. ใช้ uxTaskGetStackHighWaterMark() เพื่อตรวจสอบปริมาณ stack ที่เหลือ
  3. เปิดใช้งาน CONFIG_FREERTOS_CHECK_STACKOVERFLOW=2 เพื่อให้ระบบตรวจจับอัตโนมัติ
  4. ใช้ heap สำหรับข้อมูลขนาดใหญ่
  5. หลีกเลี่ยง recursion ที่ไม่จำเป็น
4. การตั้งค่า stack size ควรพิจารณาจากอะไร?
- การกำหนดขนาด stack ควรพิจารณาจาก:
  - จำนวนและขนาดของ local variables ที่ใช้ใน task
  - ความลึกของการเรียกฟังก์ชัน (function call depth)
  - ความซับซ้อนของโค้ด เช่น recursion, snprintf, sprintf, log ที่ใช้ buffer
  - การเผื่อ safety margin ประมาณ 30–50% ของการใช้งานสูงสุดที่คาดไว้
  - ใช้ข้อมูลจริงจาก uxTaskGetStackHighWaterMark() เพื่อปรับปรุงภายหลัง
5. Recursion ส่งผลต่อ stack usage อย่างไร?
- Recursion ทำให้ stack usage เพิ่มขึ้นตามระดับความลึกของการเรียกซ้ำ (depth) เพราะทุกครั้งที่ฟังก์ชันเรียกตัวเอง ระบบจะต้อง:
  - เก็บค่าตัวแปร local ของรอบก่อน
  - เก็บ return address
  - และสร้าง frame ใหม่ใน stack
- ดังนั้นยิ่ง recursion ลึกเท่าไหร่ stack จะถูกใช้มากขึ้นเท่านั้น
- ในโค้ด recursive_function() จะเห็นว่า stack เหลือน้อยลงในแต่ละ depth และหยุดเมื่อใกล้ overflow
