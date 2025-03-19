#include "SocketCallback.h"

#include <arpa/inet.h>
#include <netinet/in.h>

#include <thread>

#include "WGPUJsiUtils.h"

using namespace facebook::jsi;

#define PORT 8888
#define HOST "127.0.0.1"

Value wgpu::socketCallback(Runtime &runtime) {
  return WGPU_FUNC_FROM_HOST_FUNC(socketCallback, 1, []) {
    auto message = arguments[0].asString(runtime).utf8(runtime);
    auto thread = std::thread([message] {
      int sockfd;
      struct sockaddr_in server_addr;

      sockfd = socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd < 0) {
        WGPU_LOG_ERROR("Failed to open socket");
        return;
      }

      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons(PORT);

      if (inet_pton(AF_INET, HOST, &server_addr.sin_addr) <= 0) {
        WGPU_LOG_ERROR("Invalid address");
        close(sockfd);
        return;
      }

      if (connect(sockfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
        WGPU_LOG_ERROR("Failed to connect");
        close(sockfd);
        return;
      }

      if (send(sockfd, message.c_str(), message.length(), 0) < 0) {
        WGPU_LOG_ERROR("Failed to send data");
        close(sockfd);
        return;
      }

      usleep(100000);
      close(sockfd);
    });
    thread.detach();
    return Value::undefined();
  });
}
