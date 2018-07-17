unsigned long getValue(byte packet[]) {
  int high = packet[3];               //high byte for value is 4th byte in packet in the packet
  int low = packet[4];                //low byte for value is 5th byte in the packet
  unsigned long val = high*256 + low; //Combine high byte and low byte with this formula to get value
  return val* valMultiplier;
}
