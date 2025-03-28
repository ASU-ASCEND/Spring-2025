// pico code to test watchdog, flatlines after 50 iterations, resets around 100
// iterations
#define h1 20
#define h2 21

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(h1, OUTPUT);
  pinMode(h2, OUTPUT);
}

int i = 0;
// the loop function runs over and over again forever
void loop() {
  Serial.println(i++);
  digitalWrite(LED_BUILTIN,
               HIGH);  // turn the LED on (HIGH is the voltage level)
  delay(250);          // wait for a second
  digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
  delay(250);

  if (i < 80) {
    digitalWrite(h1, (i & 0x1));
    digitalWrite(h2, (i & 0x1));
  }
  if (i == 50) {
    Serial.println("Flatline");
  }
}
