#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <message>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int sockfd;
    struct sockaddr_in servaddr;

    // 创建套接字
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("create socket error");
        return EXIT_FAILURE;
    }

    // 初始化服务器地址结构体
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));  // 将端口号从字符串转换为整数

    // 将IP地址从字符串转换为网络地址
    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
        perror("inet_pton error");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // 发起连接请求
    if (connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect error");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Connected to the server. Sending message: %s\n", argv[3]);

    // 发送消息到服务器
    if (send(sockfd, argv[3], strlen(argv[3]), 0) < 0) {
        perror("send error");
        close(sockfd);
        return EXIT_FAILURE;
    }

    // 接收服务器的响应
    char buffer[1024];
    ssize_t recv_len = recv(sockfd, buffer, sizeof(buffer), 0);

    if (recv_len < 0) {
        perror("recv error");
        close(sockfd);
        return EXIT_FAILURE;
    } else if (recv_len == 0) {
        printf("Connection closed by the server\n");
    } else {
        buffer[recv_len] = '\0';
        printf("Received from server: %s\n", buffer);
    }

    // 关闭套接字
    close(sockfd);

    return EXIT_SUCCESS;
}
