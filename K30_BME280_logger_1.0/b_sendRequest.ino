void sendRequest(byte packet[]) {
  while(!K_30_Serial.available()) {  //keep sending request until we start to get a response
    K_30_Serial.write(readCO2,7);
    //Serial.println("request response");
    delay(50);
  }
  
  int timeout=0;  //set a timeoute counter
  while(K_30_Serial.available() < 7 ) { //Wait to get a 7 byte response
    timeout++;  
    if(timeout > 10) {  // if it takes too long there was probably an error
      while(K_30_Serial.available())  // flush whatever we have
      K_30_Serial.read();
      break;                        //exit and try again
    }
    delay(50);
  }
  
  for (int i=0; i < 7; i++) {
    response[i] = K_30_Serial.read();
  }  
}
