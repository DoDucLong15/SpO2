#define RED 14
#define GREEN 27
#define BLUE 26

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Hello, ESP32!");
  pinMode(RED, OUTPUT);
  pinMode(GREEN, OUTPUT);
  pinMode(BLUE, OUTPUT);
}

void loop() {
  // color code #00C9CC (R = 0,   G = 201, B = 204)
  setColor(255, 0, 0);

  delay(1000); // keep the color 1 second

  // color code #F7788A (R = 247, G = 120, B = 138)
  setColor(0, 255, 0);

  delay(1000); // keep the color 1 second

  // color code #34A853 (R = 52,  G = 168, B = 83)
  setColor(0, 0, 255);

  delay(1000); // keep the color 1 second
}

void setColor(int R, int G, int B) {
  analogWrite(RED,   R);
  analogWrite(GREEN, G);
  analogWrite(BLUE,  B);
}