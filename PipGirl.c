//pipGirl 0.2

#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define analog_l_ir 0 // Левый датчик ИК-датчик (аналоговый вход)
#define analog_r_ir 1 // Правый датчик ИК-датчик (аналоговый вход)
#define analog_mic 2 // Микрофон (аналоговый вход)
#define analog_bat_vin 3 // Вход с делителя напряжения (аналоговый вход)

#define back_l_engine 11 // Пины левого двигателя
#define forward_l_engine 10
#define back_r_engine 9 // Пины правого двигателя
#define forward_r_engine 6

#define speaker 2 // Пьезодинамик

#define echoPin_r 4 // Правый датчик расстояния
#define trigPin_r 7 // -//-

#define echoPin_l 8 // Левый датчик расстояния
#define trigPin_l 12 // -//-

#define charge_stat 3 // Подключена ли зарядка, должен подаваться высокий сигнал, если подключена падает в ноль

#define SYMBOL_HEIGHT 8

#define OPEN_EYE_DELAY 10

#define DEBUG

LiquidCrystal_I2C lcd(0x27, 16, 2);	// 0x27 - адрес, 16 - знаков на 2 строки

bool eye_stat	= 1;
int eye_count	= 0;
float Vbat		= 0;

char emotion_now = 0;

byte eye_l[SYMBOL_HEIGHT] = {
	B11111,
	B10001,
	B10001,
	B10001,
	B10111,
	B10111,
	B10111,
	B11111,
};

byte eye_r[SYMBOL_HEIGHT] = {
	B11111,
	B10001,
	B10001,
	B10001,
	B11101,
	B11101,
	B11101,
	B11111,
};

byte eye_l_bad[SYMBOL_HEIGHT] = {
	B11111,
	B11111,
	B11111,
	B10001,
	B10111,
	B10111,
	B10111,
	B11111,
};

byte eye_r_bad[SYMBOL_HEIGHT] = {
	B11111,
	B11111,
	B11111,
	B10001,
	B11101,
	B11101,
	B11101,
	B11111,
};

byte batery[SYMBOL_HEIGHT] = {
	B01110,
	B01110,
	B11111,
	B10001,
	B10001,
	B10001,
	B11111,
	B11111,
};

byte mouth_smile[SYMBOL_HEIGHT] = {
	B00000,
	B00000,
	B00000,
	B00000,
	B00000,
	B10001,
	B01110,
	B00000,
};

byte love[SYMBOL_HEIGHT] = {
	B00000,
	B00000,
	B11011,
	B11111,
	B11111,
	B01110,
	B00100,
	B00000,
};

void setup() {
	lcd.init();
	lcd.backlight();
	lcd.createChar(0, eye_l);
	lcd.createChar(1, eye_r);
	lcd.createChar(2, eye_l_bad);
	lcd.createChar(3, eye_r_bad);
	lcd.createChar(4, mouth_smile);
	lcd.createChar(5, batery);
	lcd.createChar(6, love);
	lcd.clear();

	pinMode(trigPin_r, OUTPUT); // Правый датчик расстояния (2-а пина)
	pinMode(echoPin_r, INPUT);
	pinMode(trigPin_l, OUTPUT); // Левый датчик расстояния (2-а пина)
	pinMode(echoPin_l, INPUT);

	pinMode(speaker, OUTPUT); // Пьезодинамик
	
	pinMode(charge_stat, OUTPUT); // Зарядка
	digitalWrite(charge_stat, HIGH);

	pinMode(forward_r_engine, OUTPUT); // Двигатели (4-е пина)
	pinMode(back_r_engine, OUTPUT);
	pinMode(forward_l_engine, OUTPUT);
	pinMode(back_l_engine, OUTPUT);

	analogReference(INTERNAL);

	#ifdef DEBUG
	Serial.begin(9600); //установка порта на скорость 9600 бит/сек
	#endif
}

void distance_ahead(int& left_distance, int& right_distance) {
	int duration, cm;
	digitalWrite(trigPin_l, LOW); // Опрос левого датчика
	delayMicroseconds(2);
	digitalWrite(trigPin_l, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigPin_l, LOW);
	duration = pulseIn(echoPin_l, HIGH);
	cm = duration / 58;
	left_distance = cm;
	delayMicroseconds(500);
	digitalWrite(trigPin_r, LOW);  // Опрос правого датчика
	delayMicroseconds(2);
	digitalWrite(trigPin_r, HIGH);
	delayMicroseconds(10);
	digitalWrite(trigPin_r, LOW);
	duration = pulseIn(echoPin_r, HIGH);
	cm = duration / 58;
	right_distance = cm;
}

void emotion(byte emo) {

	/* emo
		0 - Удивление
	*/

	switch (emo) {

		case 0:		

		default:	if (Vbat => 25) {
						lcd.setCursor(6, 0);
						lcd.write(2);
						lcd.setCursor(8, 0);
						lcd.write(3);
						lcd.setCursor(10, 0);
						lcd.write(5);
						lcd.setCursor(7, 1);
						lcd.print("_");
					}

					if ((Vbat > 0) && (Vbat < 25)) {
						lcd.setCursor(6, 0);
						lcd.write(0);
						lcd.setCursor(8, 0);
						lcd.write(1);
						lcd.setCursor(10, 0);
						lcd.write(6);
						lcd.setCursor(7, 1);
						lcd.write(4);
					}

					if (Vbat <= 0) {
						lcd.setCursor(6, 0);
						lcd.write(1);
						lcd.setCursor(8, 0);
						lcd.write(0);
						lcd.setCursor(10, 0);
						lcd.print("?");
						lcd.setCursor(7, 1);
						lcd.print("_");
					}
					break;
	}
}

void bat_stat() {
	Vbat	= ((analogRead(analog_bat_vin) * 1.1) / 1024.0) / (9.9 / (50.8 + 9.9));
	Vbat	= (Vbat - 3.0) / 1.2 * 100;
	lcd.setCursor(12,1);
	for (byte s_p = 0; i < 4; i++, lcd.print(" "));
	lcd.setCursor(12,0);
	lcd.print(Vbat, 0);
	lcd.print("%");
}

boolean now_charge = false;

void loop() {
	bat_stat();	
	emotion();
	/*if (digitalRead(charge_stat) == LOW) {
		lcd.setCursor(0, 0);
		if (now_charge) {
			now_charge = false;
			lcd.write(5);
		} else {
			now_charge = true;
			lcd.print(" ");
		}
	} else {
		lcd.setCursor(0, 0);
		lcd.print(" ");
	}*/

	if(eye_count > OPEN_EYE_DELAY) {
		lcd.setCursor(6, 0);
		lcd.print("- -");
		delay(500);
		emotion();
		eye_count	= 0;
	} else {
		eye_count++;
	}
	


	delay(1000);
}