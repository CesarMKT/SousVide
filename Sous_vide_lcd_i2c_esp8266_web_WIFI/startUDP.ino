
void startUDP()
{
  Serial.println("Iniciando UDP");
  //Inicializa UDP na porta 23
  UDP.begin(123);
  Serial.print("Porta local:t");
  Serial.println(UDP.localPort());
  Serial.println();
}
uint32_t getTime()
{
  if (UDP.parsePacket() == 0)
  {
    return 0;
  }
  UDP.read(NTPBuffer, NTP_PACKET_SIZE); // read the packet into the buffer
  //Combina os 4 bytes do timestamp em um numero de 32 bits
  uint32_t NTPTime = (NTPBuffer[40] << 24) | (NTPBuffer[41] << 16) | (NTPBuffer[42] << 8) | NTPBuffer[43];
  //Converte o horario NTP para UNIX timestamp
  //Unix time comeca em 1 de Jan de 1970. Sao 2208988800 segundos no horario NTP:
  const uint32_t seventyYears = 2208988800UL;
  //Subtrai setenta anos do tempo
  uint32_t UNIXTime = NTPTime - seventyYears;
  return UNIXTime;
}
void sendNTPpacket(IPAddress& address)
{
  //Seta todos os bytes do buffer como 0
  memset(NTPBuffer, 0, NTP_PACKET_SIZE);
  //Inicializa os valores necessarios para formar a requisicao NTP
  NTPBuffer[0] = 0b11100011;   // LI, Version, Mode
  //Envia um pacote requisitando o timestamp
  UDP.beginPacket(address, 123);
  UDP.write(NTPBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}
inline int getSeconds(uint32_t UNIXTime) 
{
  return UNIXTime % 60;
}
inline int getMinutes(uint32_t UNIXTime) 
{
  return UNIXTime / 60 % 60;
}
inline int getHours(uint32_t UNIXTime) 
{
  return UNIXTime / 3600 % 24;
}
