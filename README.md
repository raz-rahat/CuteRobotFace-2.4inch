# 🤖 Cute Robot Face V2 (ESP32-S3 + ST7789 + XPT2046 + MPU6500)

<p align="center">
  <img src="https://img.shields.io/badge/Board-ESP32--S3%20N16R8-red?style=for-the-badge&logo=expressif" alt="ESP32-S3">
  <img src="https://img.shields.io/badge/Display-ST7789%20240x320-blue?style=for-the-badge" alt="ST7789">
  <img src="https://img.shields.io/badge/Touch-XPT2046-green?style=for-the-badge" alt="XPT2046">
  <img src="https://img.shields.io/badge/Sensor-MPU6500%2F6050-orange?style=for-the-badge" alt="MPU6500">
  <img src="https://img.shields.io/badge/Language-C%2B%2B%20%2F%20Arduino-00599C?style=for-the-badge&logo=cplusplus" alt="C++">
</p>

---

## 📌 প্রজেক্ট পরিচিতি (Project Overview)
**Cute Robot Face V2** হলো ESP32-S3 মাইক্রোকন্ট্রোলারের ক্ষমতা ব্যবহার করে বানানো একটি অত্যন্ত কিউট, ইন্টারঅ্যাক্টিভ ও রেসপন্সিভ ডিজিটাল রোবট ফেস প্রজেক্ট। 

এই প্রজেক্টে একটি ২৪০x৩২০ পিক্সেল TFT স্ক্রিনের ওপর রোবটের চোখ ও মুখ আঁকা হয়। জাইরোস্কোপ সেন্সরের সাহায্যে আপনি যখনই বোর্ডটি নাড়াবেন বা কাত করবেন, রোবটের চোখ আপনার নড়াচড়া ট্র্যাক করবে (Gaze Tracking)! এছাড়া এতে আছে টাচ স্ক্রিন কন্ট্রোল, বাটনের মাধ্যমে ইমোশন ট্রিগার এবং সিক্রেট টিউনিং মেনু।

> ⚡ **খাস বৈশিষ্ট্য:** কোডে **Off-screen Frame Buffer (`GFXcanvas16`)** ব্যবহার করায় ডিসপ্লেতে কোনো ধরনের ফ্লিকারিং (Screen Flicker) হয় না। অ্যানিমেশনগুলো অত্যন্ত স্মুথ (Smooth 14+ FPS)!

---

## ✨ আকর্ষনীয় ফিচারসমূহ (Key Features)

- 👁️ **স্মুথ ফেস অ্যানিমেশন (No Flicker):** ফ্রেম বাফারিং প্রযুক্তি ব্যবহারের ফলে স্ক্রিন একদম ক্লিয়ার ও স্মুথ থাকে।
- 🧭 **জাইরোস্কোপ গেজ ট্র্যাকিং (Gaze Motion):** বোর্ডের হেলে পড়া বা নড়াচড়া অনুধাবন করে চোখ এদিক-ওদিক তাকায়।
- 🎭 **৭টি ভিন্ন ইমোশন (Expressions):**
  1. `IDLE` (স্বাভাবিক মিষ্টি হাসি)
  2. `HAPPY` (খুশি মনে চোখ বন্ধ করে হাসি)
  3. `LOVE` (চোখে লাভ সাইন ও ব্লাশ)
  4. `CURIOUS` (কৌতূহলী বড় চোখ)
  5. `SLEEPY` (ঘুমে ঢুলুঢুলু ভাব ও Zzz)
  6. `ANGRY` (রাগান্বিত মুখ)
  7. `SURPRISE` (অবাক হা করা মুখ)
- 👆 **ইন্টারঅ্যাক্টিভ টাচ স্ক্রিন:**
  - স্ক্রিনের **বামে ট্যাপ** করলে আগের এক্সপ্রেশনে যাবে।
  - স্ক্রিনের **ডানে ট্যাপ** করলে পরের এক্সপ্রেশনে যাবে।
  - **মাঝখানে ট্যাপ** করলে র‍্যান্ডম এক্সপ্রেশন দেখাবে।
  - **পরপর ৫ বার দ্রুত ট্যাপ** করলে রোবটটি রেগে (`ANGRY`) যাবে!
- 😱 **অনবোর্ড বুট বাটন রিয়েকশন (Fear System):** ESP32-S3 এর `BOOT` বাটন চাপলে রোবটটি ভয় পেয়ে আঁতকে উঠবে!
- ⚙️ **সিক্রেট টিউনিং মেনু (Hidden Menu):** বোর্ডটি ৩ সেকেন্ডের জন্য **উল্টো করে ধরে রাখলে** স্ক্রিনে একটি কাস্টম অন-স্ক্রিন কনফিগারেশন মেনু আসবে, যেখান থেকে সেন্সরের সেন্সিটিভিটি টিউন করা যায়।
- 👁️‍🗨️ **অটো ব্লিঙ্কিং ও আইডল লুক:** মানুষের মতোই প্রাকৃতিক সময়ের ব্যবধানে চোখের পাতা ফেলে এবং নিশ্বাস নেওয়ার সুন্দর অ্যানিমেশন রয়েছে।

---

## 🧰 প্রয়োজনীয় উপাদান (Hardware Components)

নতুনদের সুবিধার্থে প্রতিটি উপাদানের তালিকা নিচে দেওয়া হলো:

1. **ESP32-S3 N16R8 开发板 (Microcontroller):**  
   *(অবশ্যই **16MB Flash / 8MB PSRAM** ভার্সন ব্যবহার করবেন, কারণ ফ্রেম বাফারের জন্য PSRAM প্রয়োজন।)*
2. **ST7789 240x320 2.4"/2.8" SPI TFT Display**
3. **XPT2046 Touch Controller Module** *(সাধারণত ST7789 ডিসপ্লের সাথেই ইন্টিগ্রেটেড থাকে)*
4. **MPU6500** অথবা **MPU6050** (6-axis Gyroscope & Accelerometer Module)
5. **USB-C ডাটা কেবল** (ESP32-S3 পিসিতে কানেক্ট ও কোড আপলোডের জন্য)
6. **জাম্পার ওয়ার (Jumper Wires - Female to Female)** অথবা ব্রেডবোর্ড / কাস্টম পিসিবি

---

## 📚 প্রয়োজনীয় সফটওয়্যার ও লাইব্রেরি (Libraries Required)

এই প্রজেক্টটি **Arduino IDE** দিয়ে কোডিং করা হয়েছে। কোড কম্পাইল করার জন্য নিচের লাইব্রেরিগুলো আপনার Arduino IDE-তে ইন্সটল থাকতে হবে:

| লাইব্রেরির নাম (Library Name) | কাজ / বিবরণ | ইন্সটলেশন গাইড (Library Manager) |
| :--- | :--- | :--- |
| **Adafruit GFX Library** | ডিসপ্লেতে গ্রাফিক্স (Circle, Line, Canvas) আঁকার জন্য | Arduino IDE -> Library Manager -> Search `Adafruit GFX` -> Install |
| **Adafruit ST7735 and ST7789 Library** | ST7789 ডিসপ্লে ড্রাইভার | Library Manager -> Search `Adafruit ST7789` -> Install |
| **XPT2046_Touchscreen** (by Paul Stoffregen) | টাচ স্ক্রিনের ইনপুট প্রসেস করার জন্য | Library Manager -> Search `XPT2046_Touchscreen` -> Install |
| **Wire.h & SPI.h** | I2C ও SPI কমিউনিকেশনের জন্য | Arduino ESP32 core এর সাথে ডিফল্টভাবে থাকে |

---

## 🔌 পিন টু পিন কানেকশন (Detailed Pinout Diagram)

সবগুলো মডিউল ESP32-S3 বোর্ডের সাথে যেভাবে কানেক্ট করবেন তার পূর্ণাঙ্গ পিনআউট নিচে দেওয়া হলো:

### 🖥️ ১. TFT Display (ST7789) -> ESP32-S3 Connection
| TFT Display Pin | ESP32-S3 GPIO Pin | কাজের বিবরণ |
| :--- | :--- | :--- |
| **VCC** | **3.3V** | পাওয়ার সাপ্লাই |
| **GND** | **GND** | গ্রাউন্ড |
| **CS** | **GPIO 10** | TFT Chip Select |
| **RST** | **GPIO 8** | TFT Reset |
| **DC** | **GPIO 9** | Data/Command Pin |
| **MOSI / SDA** | **GPIO 11** | SPI Data (MOSI) |
| **SCK / SCL** | **GPIO 13** | SPI Clock (SCK) |
| **MISO** | **GPIO 12** | SPI MISO |
| **LED / BLK** | **GPIO 5** (বা 3.3V) | ব্যাকলাইট কন্ট্রোল |

---

### 👆 ২. Touch Controller (XPT2046) -> ESP32-S3 Connection
*(ডিসপ্লের পেছনে থাকা Touch পিনগুলো)*

| Touch Pin | ESP32-S3 GPIO Pin | কাজের বিবরণ |
| :--- | :--- | :--- |
| **T_CLK** | **GPIO 13** | Display এর SCK এর সাথে শেয়ার করবে |
| **T_CS** | **GPIO 7** | Touch Chip Select |
| **T_DIN** | **GPIO 11** | Display এর MOSI এর সাথে শেয়ার করবে |
| **T_DO** | **GPIO 12** | Display এর MISO এর সাথে শেয়ার করবে |
| **T_IRQ** | **GPIO 6** | Touch Interrupt (ঐচ্ছিক) |

---

### 🧭 ৩. Gyro Sensor (MPU6500 / MPU6050) -> ESP32-S3 Connection

| MPU Pin | ESP32-S3 GPIO Pin | কাজের বিবরণ |
| :--- | :--- | :--- |
| **VCC** | **3.3V** | পাওয়ার |
| **GND** | **GND** | গ্রাউন্ড |
| **SDA** | **GPIO 16** | I2C Data Pin |
| **SCL** | **GPIO 15** | I2C Clock Pin |

---

### 🔘 ৪. অনবোর্ড কন্ট্রোল (Onboard Controls)
| উপাদান | ESP32-S3 Pin | কাজের বিবরণ |
| :--- | :--- | :--- |
| **BOOT Button** | **GPIO 0** | চাপলে ভয় পাওয়ার (`FEAR`) অ্যানিমেশন চালু হবে |

---

## ⚙️ আর্ডুইনো আইডিই সেটিংস ও আপলোড গাইড (Beginner Setup Guide)

নতুনরা নিচের ধাপগুলো অনুসরণ করে খুব সহজেই কোডটি আপলোড করতে পারবেন:

1. **Arduino IDE ডাউনলোড ও ওপেন করুন:** (ভার্সন 2.x বা তার উপরে সাজেস্টেড)।
2. **ESP32 Board Package সংযোজন:**
   - `Tools` -> `Board` -> `Boards Manager...` এ যান।
   - সার্চ করুন `esp32` (by Espressif Systems) এবং ইন্সটল করুন।
3. **লাইব্রেরি ইন্সটল করুন:** 
   - `Tools` -> `Manage Libraries...` এ গিয়ে পূর্বোল্লিখিত লাইব্রেরিগুলো (Adafruit GFX, Adafruit ST7789, XPT2046_Touchscreen) ইন্সটল করে নিন।
4. **বোর্ড সেটিংস নির্বাচন (খুবই গুরুত্বপূর্ণ):**
   - **Board:** `ESP32S3 Dev Module`
   - **Port:** আপনার ESP32-S3 যে পোর্টে কানেক্ট হয়েছে তা সিলেক্ট করুন।
   - **PSRAM:** `OPI PSRAM` বা `Enabled` *(এটি সিলেক্ট না করলে বাফার মেমরির অভাবে স্ক্রিন সাদা হয়ে থাকতে পারে)*
   - **Flash Size:** `16MB (128Mb)`
5. **কম্পাইল ও আপলোড:**
   - পিসির সাথে USB-C কেবল দিয়ে ESP32-S3 যুক্ত করুন।
   - `Upload` বাটনে ক্লিক করুন।
   - আপলোড শেষে বোর্ডটি সমতল টেবিলে স্থির রাখুন যাতে MPU জাইরোস্কোপ সেন্সর সঠিকভাবে ক্যালিব্রেট হতে পারে।

---

## 🎮 যেভাবে চালনা করবেন (How to Control)

| অ্যাকশন (Action) | ফলাফল (Result) |
| :--- | :--- |
| **বোর্ড ডানে/বামে কাত করা** | রোবটের চোখ সেই দিকে তাকাবে (Gyro Gaze)। |
| **স্ক্রিনের বাম পাশে টাচ** | আগের এক্সপ্রেশন অন হবে। |
| **স্ক্রিনের ডান পাশে টাচ** | পরের এক্সপ্রেশন অন হবে। |
| **স্ক্রিনের মাঝখানে টাচ** | যেকোনো একটি র‍্যান্ডম এক্সপ্রেশন আসবে। |
| **পরপর ৫ বার স্ক্রিনে ট্যাপ** | রোবটটি রেগে যাবে (`ANGRY` Expression)! |
| **BOOT বাটন প্রেস** | রোবট ভয় পেয়ে চমকে উঠবে (`FEAR` Reaction)। |
| **৩ সেকেন্ড বোর্ড উল্টো ধরে রাখা** | সিক্রেট **Tuning Menu** ওপেন হবে। টাচ দিয়ে সেটিংস পরিবর্তন করে সেন্টারে চাপ দিয়ে বের হতে পারবেন। |

---

## 🔍 ট্রাবলশুটিং / সমস্যা ও সমাধান (Troubleshooting)

- **প্রশ্ন: কোড আপলোড হওয়ার পর স্ক্রিন সাদা (Blank/White Screen) হয়ে আছে?**
  - **সমাধান:** Arduino IDE এর Tools থেকে **PSRAM** অপশনটি `Enabled` বা `OPI PSRAM` করা হয়েছে কি না নিশ্চিত করুন। পিন কানেকশন (বিশেষ করে VCC, GND, CS, RST, DC) পুনরায় চেক করুন।
- **প্রশ্ন: টাচ ঠিকমতো কাজ করছে না?**
  - **সমাধান:** T_CS, T_CLK, T_DIN, T_DO পিনগুলো সঠিকভাবে যুক্ত আছে কিনা দেখুন। কোডের `RAW_X_MIN`, `RAW_X_MAX` আপনার টাচ প্যানেল অনুযায়ী টিউন করে নিতে পারেন।
- **প্রশ্ন: MPU NOT DETECTED মেসেজ আসছে?**
  - **সমাধান:** I2C পিন (SDA=GPIO 16, SCL=GPIO 15) কানেকশন চেক করুন। MPU6500 এর VCC ৩.৩ ভোল্টে যুক্ত আছে কি না নিশ্চিত হন।

---

## 📺 ভিডিও টিউটোরিয়াল ও চ্যানেল সাবস্ক্রাইব (YouTube & Support)

আপনি যদি এই প্রজেক্টটির সম্পূর্ণ মেকিং ভিডিও ও টিউটোরিয়াল দেখতে চান এবং এরকম আরও দারুণ দারুণ রোবোটিক্স ও ইলেকট্রনিক্স প্রজেক্ট শিখতে চান, তবে আমার ইউটিউব চ্যানেলটি ভিজিট ও সাবস্ক্রাইব করতে ভুলবেন না!

<p align="center">
  <a href="https://www.youtube.com/@razfriday" target="_blank">
    <img src="https://img.shields.io/badge/YouTube-Subscribe%20Now-red?style=for-the-badge&logo=youtube" alt="YouTube Channel">
  </a>
</p>

👉 **ইউটিউব চ্যানেল লিংক:** [https://www.youtube.com/@razfriday](https://www.youtube.com/@razfriday)

প্রোজেক্টটি ভালো লাগলে গিটহাবে একটি **Star (⭐)** দিয়ে সাপোর্ট করবেন। ধন্যবাদ এবং Happy Making! 🚀
