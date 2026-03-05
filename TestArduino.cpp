// ENA = Motor A speed control (PWM) : 전기를 빠르게 켰다 껏다 반복하여 평균 전압 조절 
// IN1 = Motor A front/back spin
// IN2 = Motor A front/back spin 
// ENB = Motor B speed control (PWM)
// IN3 = Motor B front/back spin
// IN4 = Motor B front/back spin
// pwmFreq = swiching speed; pwmResolution = speed of motor; 
// ledcAttachPin = connect PWN to Pin; pinMode() = Pin Mode; 
// digitalWrite() = 방향제어; ledcWrite = control speed;
// 채널을 만들고 각각 채널의 스피드와 Hz를 설정한 후, 모터랑 연결해준다. 그 후에 핀에서 PWM출력해주고 이걸 아웃풋으로 바꿔준 후에 방향과 속도 결정
// pwmChannel : ESP32, PWN(channel unit)
// const int pwmChannel1 = 0;  ch0 = motor A
// const int pwmChannel2 = 1; channel 1 = motor B
// const int pwmFreq = 5000; 5000Hz : 1초에 PWM이 얼마나 되는가, 몇 번을 스위칭 하는가 , 속도와는 상관 없음 
// const int pwmReslution = 8; 8bit resolution(0-255); 0 = stop, 128 = half speed, 255 = MAX: 속도와 상관있음
// void setup(), when it turns on, only one time executes : PWM 채널 생성 
// ledcSetup(pwmChannel1, pwmFreq, pwmResolution); motor A setup with channel 0, 5000Hz, and 8 bits
// ledcAttachPin(ENA, pwmChannel1); connect 25 pin to PWN channel 0 -> ENA pin에서 PWM 출력 : 이 핀에서 PWM 신호를 보내라, 특정 핀을 채널에 연결
// pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);  : 입력상태였던 핀을 출력모드로 바꿈
// void loop() : loop forever
// difitalWrite(IN1, in1_value); digitalWrite(IN2, in2_value); 방향 결정 : motor A control -> (IN1, IN2) : (HIGH, HIGH) = Break, (LOW, LOW) = stop , (HIGH, LOW) = forward, (LOW, HIGH) = BACKWard
// ledcWrite(pwmChannel1, ena_value); 속도 결정 :  control speed (0-255) : ledcSetup() 에서 PWM 채널 생성 후 ledcAttachPin() 에서 채널을 특정핀에 연결한 것의 값 출력

// one more thing, if (ctl->buttons() & 0x0001) is better than using ==, & means include the button. 


 #include <Bluepad32.h>
 
 //Workshop Day 1
 #define ENA 25 
 #define IN1 26
 #define IN2 27

 #define ENB 13
 #define IN3 12
 #define IN4 14

 const int pwmChannel1 = 0;
 const int pwmChannel2 = 1;
 const int pwmFreq = 5000;
 const int pwmResolution = 8; // (0, 255)

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id,
                           properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }

    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}

//============ SEE CONTROLLER VALUES IN SERIAL MONITOR ===========//
void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, "
        "misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
        ctl->miscButtons(),  // bitmask of pressed "misc" buttons
        ctl->gyroX(),        // Gyro X 조이스틱 위치 
        ctl->gyroY(),        // Gyro Y
        ctl->gyroZ(),        // Gyro Z
        ctl->accelX(),       // Accelerometer X
        ctl->accelY(),       // Accelerometer Y
        ctl->accelZ()        // Accelerometer Z
    );
}

void dumpMouse(ControllerPtr ctl) {
    Serial.printf("idx=%d, buttons: 0x%04x, scrollWheel=0x%04x, delta X: %4d, delta Y: %4d\n",
                   ctl->index(),        // Controller Index
                   ctl->buttons(),      // bitmask of pressed buttons
                   ctl->scrollWheel(),  // Scroll Wheel
                   ctl->deltaX(),       // (-511 - 512) left X Axis
                   ctl->deltaY()        // (-511 - 512) left Y axis
    );
}

void dumpKeyboard(ControllerPtr ctl) {
    static const char* key_names[] = {
        // clang-format off
        // To avoid having too much noise in this file, only a few keys are mapped to strings.
        // Starts with "A", which is offset 4.
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
        "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0",
        // Special keys
        "Enter", "Escape", "Backspace", "Tab", "Spacebar", "Underscore", "Equal", "OpenBracket", "CloseBracket",
        "Backslash", "Tilde", "SemiColon", "Quote", "GraveAccent", "Comma", "Dot", "Slash", "CapsLock",
        // Function keys
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
        // Cursors and others
        "PrintScreen", "ScrollLock", "Pause", "Insert", "Home", "PageUp", "Delete", "End", "PageDown",
        "RightArrow", "LeftArrow", "DownArrow", "UpArrow",
        // clang-format on
    };
    static const char* modifier_names[] = {
        // clang-format off
        // From 0xe0 to 0xe7
        "Left Control", "Left Shift", "Left Alt", "Left Meta",
        "Right Control", "Right Shift", "Right Alt", "Right Meta",
        // clang-format on
    };
    Serial.printf("idx=%d, Pressed keys: ", ctl->index());
    for (int key = Keyboard_A; key <= Keyboard_UpArrow; key++) {
        if (ctl->isKeyPressed(static_cast<KeyboardKey>(key))) {
            const char* keyName = key_names[key-4];
            Serial.printf("%s,", keyName);
       }
    }
    for (int key = Keyboard_LeftControl; key <= Keyboard_RightMeta; key++) {
        if (ctl->isKeyPressed(static_cast<KeyboardKey>(key))) {
            const char* keyName = modifier_names[key-0xe0];
            Serial.printf("%s,", keyName);
        }
    }
    Console.printf("\n");
}

void dumpBalanceBoard(ControllerPtr ctl) {
    Serial.printf("idx=%d,  TL=%u, TR=%u, BL=%u, BR=%u, temperature=%d\n",
                   ctl->index(),        // Controller Index
                   ctl->topLeft(),      // top-left scale
                   ctl->topRight(),     // top-right scale
                   ctl->bottomLeft(),   // bottom-left scale
                   ctl->bottomRight(),  // bottom-right scale
                   ctl->temperature()   // temperature: used to adjust the scale value's precision
    );
}

//============ GAME CONTROLLER ACTION SECTION ===========//
void processGamepad(ControllerPtr ctl) {
    // There are different ways to query whether a button is pressed.
    // By query each button individually:
    //  a(), b(), x(), y(), l1(), etc...

    //ctl은 어떤 클래스(예: 게임패드, 컨트롤러)의 객체를 가르키는 포인터. 
    //ctl->button() 은 ctl객체의 해당 함수 실행을 의미 :객체인 컨트롤러 PS4의 해당 버튼을 누르면 statements에 따른 결과가 나옴
    // button()을 찾아보면, 현재 눌린 버튼 상채를 숫자로 반환한는 함수로서 bit로 표현됨
    // 0x0001(Hex) = 0000000000000001(Binaray) 임으로 버튼 1번을 의미
    // digitalWrite(pin 번호, HIGH or LOW) 은 핀의 전압을 출력함
    // buildInLed 는 보드에 달려있는 LED 핀 번호를 뜻함 
    
    // PS4 Dpad UP button = 0x01
    // PS4 Dpad DOWN button = 0x02
    // PS4 Dpad RIGHT button = 0x04
    // PS4 Dpad LEFT button = 0x08
    // PS4 X button = 0x0001 
    // if (ctl->buttons() == 0x0001) 
    // {
    //     digitalWrite(buildInLed, HIGH);
    // }
    // if (ctl->button() != 0x0001)
    // {
    //     digitalWrite(builtInLed, LOW);
    // }

    // // PS4 Circle button = 0x0002

    // // PS4 Squre button = 0x0004
    // if (ctl->buttons() == 0x0004)
    // {
    //     //code for when squre button is pushed
    // }
    // if(ctl->buttons() != 0x0004)
    // {
    //     //code for when square button is released
    // }

    // // PS4 Triangle button = 0x0008
    // if (ctl->button() == 0x0008)
    // {
    //     //code for when triangle button is pushed
    // }
    // if(ctl->button() != 0x0008)
    // {
    //     //code for when triangle button is released
    // }
    // // PS4 L1 Trigger button = 0x0010
    // // PS4 R1 Trigger button = 0x0020
    // // PS4 L2 Trigger button = 0x0040
    // // PS4 R2 Trigger button = 0x0080

    // // ======= LEFT JOYSTICK CONTROL ======= //
    // // LEFT JOYSTICK UP
    // if (ctl->axisY() <= -25)
    // {
    //     //code for when left joystick is pushed up
    // }
    // // LEFT JOUSTICK DOWN
    // if(ctl->axisY() >= 25)
    // {
    //     //code for when left joystick is pused down
    // }
    // // LEFT JOUSTICK LEFT
    // if(ctl->axisX() <= -100)
    // {
    //     //code for when left joystick is pushed left
    // }
    // // LEFT JOUSTICK RIGHT
    // if(ctl-<axisX >= 100)
    // {
    //     //code for when left joustick is pushed right
    // }
    // // LEFT JOYSTICK DEADZONE
    // if(ctl->axisY() > -25 && ctl->axisY() < 25 && ctl->axisX > - 100 && ctl->axisX < 100)
    // {
    //     // code for when left joustick is at idle
    // }

    // // ======= RIGHT JOYSTICK CONTROL ======= //
    // if (ctl-<axisRX())
    // {
    //     //code for when right joysutick moves along x-axis
    // }


    if (ctl->a()) {
        static int colorIdx = 0;
        // Some gamepads like DS4 and DualSense support changing the color LED.
        // It is possible to change it by calling:
        switch (colorIdx % 3) {
            case 0:
                // Red
                ctl->setColorLED(255, 0, 0);
                break;
            case 1:
                // Green
                ctl->setColorLED(0, 255, 0);
                break;
            case 2:
                // Blue
                ctl->setColorLED(0, 0, 255);
                break;
        }
        colorIdx++;
    }

    if (ctl->b()) {
        // Turn on the 4 LED. Each bit represents one LED.
        static int led = 0;
        led++;
        // Some gamepads like the DS3, DualSense, Nintendo Wii, Nintendo Switch
        // support changing the "Player LEDs": those 4 LEDs that usually indicate
        // the "gamepad seat".
        // It is possible to change them by calling:
        ctl->setPlayerLEDs(led & 0x0f);
    }

    if (ctl->x()) {
        // Some gamepads like DS3, DS4, DualSense, Switch, Xbox One S, Stadia support rumble.
        // It is possible to set it by calling:
        // Some controllers have two motors: "strong motor", "weak motor".
        // It is possible to control them independently.
        ctl->playDualRumble(0 /* delayedStartMs */, 250 /* durationMs */, 0x80 /* weakMagnitude */,
                            0x40 /* strongMagnitude */);
    }

    // Another way to query controller data is by getting the buttons() function.
    // See how the different "dump*" functions dump the Controller info.
    dumpGamepad(ctl);
}

void processMouse(ControllerPtr ctl) {
    // This is just an example.
    if (ctl->scrollWheel() > 0) {
        // Do Something
    } else if (ctl->scrollWheel() < 0) {
        // Do something else
    }

    // See "dumpMouse" for possible things to query.
    dumpMouse(ctl);
}

void processKeyboard(ControllerPtr ctl) {
    if (!ctl->isAnyKeyPressed())
        return;

    // This is just an example.
    if (ctl->isKeyPressed(Keyboard_A)) {
        // Do Something
        Serial.println("Key 'A' pressed");
    }

    // Don't do "else" here.
    // Multiple keys can be pressed at the same time.
    if (ctl->isKeyPressed(Keyboard_LeftShift)) {
        // Do something else
        Serial.println("Key 'LEFT SHIFT' pressed");
    }

    // Don't do "else" here.
    // Multiple keys can be pressed at the same time.
    if (ctl->isKeyPressed(Keyboard_LeftArrow)) {
        // Do something else
        Serial.println("Key 'Left Arrow' pressed");
    }

    // See "dumpKeyboard" for possible things to query.
    dumpKeyboard(ctl);
}

// DATA TRANSACTION Function (NO NEED For NOW)
void processBalanceBoard(ControllerPtr ctl) {
    // This is just an example.
    if (ctl->topLeft() > 10000) {
        // Do Something
    }

    // See "dumpBalanceBoard" for possible things to query.
    dumpBalanceBoard(ctl);
}

void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            } else if (myController->isMouse()) {
                processMouse(myController);
            } else if (myController->isKeyboard()) {
                processKeyboard(myController);
            } else if (myController->isBalanceBoard()) {
                processBalanceBoard(myController);
            } else {
                Serial.println("Unsupported controller");
            }
        }
    }
}

// Arduino setup function. Runs in CPU 1
void setup() {
    Serial.begin(115200);
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Workshop Day 1
    ledcSetup(pwmChannel1, pwmFreq, pwmResolution);
    ledcAttachPin(ENA, pwmChannel1);
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);

    ledcSetup(pwmChannel1, pwmFreq, pwmResolution);
    ledcAttachPin(ENB, pwmChannel2)
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // "forgetBluetoothKeys()" should be called when the user performs
    // a "device factory reset", or similar.
    // Calling "forgetBluetoothKeys" in setup() just as an example.
    // Forgetting Bluetooth keys prevents "paired" gamepads to reconnect.
    // But it might also fix some connection / re-connection issues.
    BP32.forgetBluetoothKeys();

    // Enables mouse / touchpad support for gamepads that support them.
    // When enabled, controllers like DualSense and DualShock4 generate two connected devices:
    // - First one: the gamepad
    // - Second one, which is a "virtual device", is a mouse.
    // By default, it is disabled.
    BP32.enableVirtualDevice(false);
}

// Arduino loop function. Runs in CPU 1.
void loop() {
    // This call fetches all the controllers' data.
    // Call this function in your main loop.
    bool dataUpdated = BP32.update();
    if (dataUpdated)
        processControllers();

    // The main loop must have some kind of "yield to lower priority task" event.
    // Otherwise, the watchdog will get triggered.
    // If your main loop doesn't have one, just add a simple `vTaskDelay(1)`.
    // Detailed info here:
    // https://stackoverflow.com/questions/66278271/task-watchdog-got-triggered-the-tasks-did-not-reset-the-watchdog-in-time

    //     vTaskDelay(1);

    // Workshop Day 1
    // digitalWrite(IN1, LOW);
    // digitalWrite(IN2, HIGH);
    // digitalWrite(IN3, LOW);
    // digitalWrite(IN4, HIGH);
    delay(150);
}
