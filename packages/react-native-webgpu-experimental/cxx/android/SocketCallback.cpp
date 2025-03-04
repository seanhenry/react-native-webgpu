#include "SocketCallback.h"

#include <arpa/inet.h>
#include <netinet/in.h>

#include "WGPUJsiUtils.h"

using namespace facebook::jsi;

#define PORT 8888
#define HOST "10.0.2.2"

Value wgpu::socketCallback(Runtime &runtime) {
  return WGPU_FUNC_FROM_HOST_FUNC(socketCallback, 1, []) {
    auto exampleName = arguments[0].asString(runtime).utf8(runtime);
    int sockfd;
    struct sockaddr_in server_addr {};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
      WGPU_LOG_ERROR("ERROR opening socket");
      return Value::undefined();
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, HOST, &server_addr.sin_addr) <= 0) {
      WGPU_LOG_ERROR("ERROR on inet_pton");
      close(sockfd);
      return Value::undefined();
    }

    if (connect(sockfd, reinterpret_cast<struct sockaddr *>(&server_addr), sizeof(server_addr)) < 0) {
      WGPU_LOG_ERROR("ERROR connecting");
      close(sockfd);
      return Value::undefined();
    }

    if (send(sockfd, exampleName.c_str(), exampleName.length(), 0) < 0) {
      WGPU_LOG_ERROR("ERROR sending data");
      close(sockfd);
      return Value::undefined();
    }

    close(sockfd);
    return Value::undefined();
  });
}
