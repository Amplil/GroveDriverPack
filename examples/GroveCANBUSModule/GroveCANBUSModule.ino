// BOARD Seeed Wio BG770A
// GROVE UART <-> Grove - CAN BUS Module based on GD32E103 (SKU#101020782)

#include <GroveDriverPack.h>

#define INTERVAL    (1000)

GroveBoard Board;
GroveCANBUSModule CanBus(&Board.UART);

void OnMessageReceived(const CANMessage& message)
{
  Serial.print("Received CAN Message - ID: 0x");
  Serial.print(message.id, HEX);
  Serial.print(", Extended: ");
  Serial.print(message.isExtended ? "Yes" : "No");
  Serial.print(", Remote: ");
  Serial.print(message.isRemote ? "Yes" : "No");
  Serial.print(", Data: ");
  
  for (int i = 0; i < 8; i++)  // Always print 8 bytes
  {
    if (message.data[i] < 0x10) Serial.print("0");
    Serial.print(message.data[i], HEX);
    Serial.print(" ");
  }
  Serial.println();
}

void setup() {
  delay(200);
  Serial.begin(115200);

  // Initialize UART at 9600 baud (default for Grove CAN BUS Module)
  Board.UART.Enable(9600, 8, HalUART::PARITY_NONE, 1);
  
  if (!CanBus.Init())
  {
    Serial.println("CAN BUS Module not found or initialization failed.");
    while(1);
  }

  Serial.println("Grove CAN BUS Module initialized successfully.");
  
  // Set CAN speed to 500KBPS (matching Longan Labs default)
  if (CanBus.SetSpeed(GroveCANBUSModule::SPEED_500KBPS))
  {
    Serial.println("CAN speed set to 500KBPS.");
  }
  else
  {
    Serial.println("Failed to set CAN speed.");
  }
  
  // Attach message received callback
  CanBus.AttachMessageReceived(OnMessageReceived);
  
  Serial.println("Setup completed. Starting CAN communication...");
}

void loop() {
  // Check for incoming messages
  CanBus.DoWork();
  
  // Send a test message every INTERVAL milliseconds
  static unsigned long lastSendTime = 0;
  static uint8_t counter = 0;
  
  if (millis() - lastSendTime > INTERVAL)
  {
    CANMessage testMessage;
    testMessage.id = 0x123;          // CAN ID
    testMessage.isExtended = false;   // Standard ID
    testMessage.isRemote = false;     // Data frame
    testMessage.length = 8;           // 8 bytes of data (standard)
    testMessage.data[0] = 0x01;       // Test data
    testMessage.data[1] = 0x02;
    testMessage.data[2] = 0x03;
    testMessage.data[3] = counter++;  // Incrementing counter
    testMessage.data[4] = 0x05;
    testMessage.data[5] = 0x06;
    testMessage.data[6] = 0x07;
    testMessage.data[7] = 0x08;
    
    if (CanBus.SendMessage(testMessage))
    {
      Serial.print("Sent test message #");
      Serial.println(counter - 1);
    }
    else
    {
      Serial.println("Failed to send test message.");
    }
    
    lastSendTime = millis();
  }
  
  delay(10);
}
