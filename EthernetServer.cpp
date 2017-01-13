#include "Ethernet.h"
#include "w5100.h"

uint16_t EthernetServer::server_port[MAX_SOCK_NUM];


void EthernetServer::begin()
{
  for (int sock = 0; sock < MAX_SOCK_NUM; sock++) {
    EthernetClient client(sock);
    if (client.status() == SnSR::CLOSE_WAIT && !client.available()) {
        client.stop();
    }
    if (client.status() == SnSR::CLOSED) {
      socket(sock, SnMR::TCP, _port, 0);
      listen(sock);
      EthernetClass::_server_port[sock] = _port;
      break;
    }
  }  
}

void EthernetServer::accept()
{
	bool listening = false;

	if (sockindex < MAX_SOCK_NUM) {
		uint8_t status = Ethernet.socketStatus(sockindex);
		if (status == SnSR::LISTEN) {
			listening = true;
		} else if (status == SnSR::CLOSE_WAIT && Ethernet.socketRecvAvailable(sockindex) <= 0) {
			Ethernet.socketDisconnect(sockindex);
		}
	}
	if (!listening) begin();
}

EthernetClient EthernetServer::available()
{
	accept();
	for (uint8_t i=0; i < MAX_SOCK_NUM; i++) {
		if (server_port[i] == _port) {
			uint8_t stat = Ethernet.socketStatus(i);
			if (stat == SnSR::ESTABLISHED || stat == SnSR::CLOSE_WAIT) {
				if (Ethernet.socketRecvAvailable(i) > 0) {
					EthernetClient client(i);
					return client;
				}
			}
		}
	}
	return EthernetClient();
}

size_t EthernetServer::write(uint8_t b) 
{
  return write(&b, 1);
}

size_t EthernetServer::write(const uint8_t *buffer, size_t size) 
{
	accept();
	for (uint8_t i=0; i < MAX_SOCK_NUM; i++) {
		if (server_port[i] == _port) {
			if (Ethernet.socketStatus(i) == SnSR::ESTABLISHED) {
				Ethernet.socketSend(i, buffer, size);
			}
		}
	}
	return size;
}
