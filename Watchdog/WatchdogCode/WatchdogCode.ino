#define HEARTBEAT_PIN_0 1
#define HEARTBEAT_PIN_0_INTERRUPT PCINT1
#define HEARTBEAT_PIN_1 2
#define HEARTBEAT_PIN_1_INTERRUPT PCINT2
#define RESET_PIN 0

volatile bool beat0 = false;
volatile bool beat1 = false;

ISR(PCINT0_vect) {
  if (digitalRead(HEARTBEAT_PIN_0) == HIGH) {
    beat0 = true;
  }
  if (digitalRead(HEARTBEAT_PIN_1) == HIGH) {
    beat1 = true;
  }
}

// 1 minute before looking for heartbeat
#define MS_WAIT_AFTER_BOOT 60000 * 5
#define MS_WAIT_IN_OP 60000

void setup() {
  // setup
  pinMode(HEARTBEAT_PIN_0, INPUT);
  pinMode(HEARTBEAT_PIN_1, INPUT);
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);

  // any logical change interrupt
  MCUCR &= ~((1 << ISC01) | (1 << ISC00));
  MCUCR |= (1 << ISC00);
  // enable Pin Change interrupt
  GIMSK |= (1 << PCIE);
  // enable SREG, I think the arduino sdk will do that

  // enable pins
  PCMSK |= (1 << HEARTBEAT_PIN_0_INTERRUPT) | (1 << HEARTBEAT_PIN_1_INTERRUPT);

  delay(MS_WAIT_AFTER_BOOT);  // wait 10s to start checking
}

void loop() {
  delay(MS_WAIT_IN_OP);             // check every period
  if ((beat0 && beat1) == false) {  // if either beat hasn't changed
    // one of the tasks if frozen, reset
    digitalWrite(RESET_PIN, HIGH);
    delay(500);
    digitalWrite(RESET_PIN, LOW);
    delay(MS_WAIT_AFTER_BOOT);  // give the pico time to setup again
    // clear beats
  }
  beat0 = false;
  beat1 = false;
}
