void error(const char * str) {
  Serial.print("error: ");
  Serial.println(str);
  digitalWrite(redLEDpin, HIGH); // red LED indicates error
  while(1);
}

