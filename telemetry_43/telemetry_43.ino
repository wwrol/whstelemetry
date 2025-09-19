void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial1.begin(9600);
}

void loop() {
  Serial1.write("A");
  

  while (Serial1.available()) {
    Serial.write(Serial1.read());
  }
  Serial.println();
  Serial.println("NEXT FRAME:");
}
