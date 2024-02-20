#pragma once

#include "constants.h"
#include "socklib.h"
#include <sstream>

class UDPClient {
private:
  Socket* socket;
  bool include_ids;

  int send_message_without_ids(const std::string& str, std::string& result) 
  {
    constexpr int response_size = 256;
    char received[response_size] = {0};

    for (char c : str) {
      float ttl = consts::INITIAL_TIMEOUT;
      socket->SetTimeout(ttl);

      int err = -1;
      while (err == -1) {
        char data[2] = {c, '\0'};
        socket->Send(data, sizeof(char) * 2);

        try {
          err = socket->Recv(received, response_size - 1);
        } catch (std::runtime_error e) {
          std::cerr << "Failed, " << ttl << " secs ttl" << std::endl;
        }

        ttl *= 2.f;
        if (ttl >= consts::MAX_TIMEOUT) {
          return -1;
        }
      }

      result.push_back(received[0]);
    }

    return 0;
  }

  int send_message_with_ids(const std::string& str, std::string& result) 
  {
    constexpr int response_size = 256;
    char received[response_size] = {0};

    for (char c : str) {
      float ttl = consts::INITIAL_TIMEOUT;
      socket->SetTimeout(ttl);

      int id = rand();

      while (true) {
        char data[256] = {0};
        snprintf(data, 255, "%d|%c", id, c);
        socket->Send(data, sizeof(char) * 256);

        bool got_resp = false;
        try {
          socket->Recv(received, response_size - 1);
          got_resp = true;
        } catch (std::runtime_error e) {
          std::cerr << "Failed, " << ttl << " secs ttl" << std::endl;
        }

        if (got_resp) {
          std::string str_recv(received);
          std::cout << str_recv << std::endl;
          std::string recv_id = str_recv.substr(0, str_recv.find("|"));
          std::string response = str_recv.substr(str_recv.find("|") + 1, str_recv.size());

          try {
            int recv_id_int = std::stoi(recv_id);
            if (recv_id_int == id) {
              result += response;
              break;
            }
          }
          catch (std::invalid_argument e) {
            std::cout << "Couldn't parse ID from " << recv_id << std::endl;
          }
        }
        
        ttl *= 2.f;
        if (ttl >= consts::MAX_TIMEOUT) {
          return -1;
        }
      }
    }

    return 0;
  }

public:
  UDPClient(const char* host, int port, bool include_ids = false)
  {
    this->include_ids = include_ids;

    std::cout << host << std::endl;
    socket = new Socket();
    socket->Create(Socket::Family::INET, Socket::Type::DGRAM);
    int err = socket->Connect(Address(host, port));
    if (err != 0)
      std::cout << "Err: " << err << std::endl;
  }

  int send_message_by_character(const std::string& str, std::string& result)
  {
    if (include_ids) return send_message_with_ids(str, result);
    else return send_message_without_ids(str, result);
  }
};
