#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>
#include <netinet/in.h>
#include <unistd.h>

const int MAXBUFF = 1024;

std::mutex g_mx; // 用于保护共享资源的互斥锁
std::queue<std::pair<int, std::string>> g_dataQue; // 存储客户端套接字和消息的队列

// 在单独的线程中处理从客户端接收到的消息
void writeThread() {
    while (true) {
        g_mx.lock(); // 上锁以确保安全访问共享资源
        if (!g_dataQue.empty()) {
            int clientFd = g_dataQue.front().first;
            std::string data = g_dataQue.front().second;
            std::cout << "从客户端接收到的消息：" << data << std::endl;

            // 假设有一个处理消息并返回响应的函数
            std::string response = "Server response: 你好，客户端！";
            send(clientFd, response.c_str(), response.size(), 0);

            g_dataQue.pop(); // 从队列中移除已处理的消息
        }
        g_mx.unlock(); // 解锁
    }
}

int main() {
    int listenFd, clientFd;
    struct sockaddr_in servaddr;

    if ((listenFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "创建套接字错误" << std::endl;
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(1121);

    if (bind(listenFd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        std::cerr << "绑定套接字地址和端口错误" << std::endl;
        return -1;
    }

    if (listen(listenFd, 10) < 0) {
        std::cerr << "开启监听错误" << std::endl;
        return -1;
    }

    std::thread write_thread(writeThread); // 创建一个线程来处理从客户端接收到的消息
    size_t readLen = 0;

    while (true) {
        struct sockaddr_in client_addr;
        socklen_t size = sizeof(client_addr);

        if ((clientFd = accept(listenFd, (struct sockaddr *)&client_addr, &size)) < 0) {
            std::cerr << "建立连接错误" << std::endl;
            return -1;
        }

        // 假设有一个读取函数来从 clientFd 中读取数据
        char buff[MAXBUFF] = {0};
        readLen = read(clientFd, buff, MAXBUFF);

        if (readLen <= 0) {
            close(clientFd); // 在读取到数据后关闭客户端连接
            break;
        }

        std::string data(buff, readLen);
        g_mx.lock(); // 上锁以确保安全访问共享资源
        g_dataQue.push(std::make_pair(clientFd, data)); // 将接收到的消息和客户端套接字放入队列
        g_mx.unlock(); // 解锁
    }

    write_thread.join(); // 等待写线程结束
    close(listenFd);

    return 0;
}
