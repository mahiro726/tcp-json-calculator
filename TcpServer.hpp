#pragma once

#include "Logger.hpp"
#include "InetAddr.hpp"
#include "Socket.hpp"
#include <memory>
#include <functional>

#include <signal.h> // signal()、SIGCHLD、SIG_IGN
#include <unistd.h> // fork()、sleep()、pid_t

static uint16_t gport = 8080;
using handler_t = std::function<std::string(std::string&)>;

class TcpServer
{
public:
    TcpServer(handler_t handler, uint16_t port = gport)
        : _port(port),
          _listensock(std::make_unique<TcpSocket>()),
          _handler(handler)
    {
        // 使用创建listen socket的模板方法
        _listensock->BuildTcpSocketMethod(_port);
        LOG(LogLevel::DEBUG) << "创建 listen socket 成功，listensockfd: " << _listensock->GetSockfd();
    }

    void Loop()
    {
        signal(SIGCHLD, SIG_IGN); // 父进程不需要wait，直接让内核回收子进程，避免变成僵尸进程
        while (true)
        {
            // 获取链接，获取一个一个的套接字
            InetAddr clientaddr; // 输出型参数
            auto sockfd = _listensock->Accept(clientaddr);
            if (!sockfd)
            {
                continue;
            }
            LOG(LogLevel::DEBUG) << "接收到新客人，socket address: " << clientaddr.ToString() << "sockfd: " << _listensock->GetSockfd();

            // 创建子进程，并让子进程来调用service
            if (fork() == 0)
            {
                service(sockfd, clientaddr);
                // 做完任务后，要关闭accept_sockfd
                sockfd->Close();
                // 子进程做完事情一定要退出，不能让子进程出来
                exit(0);
            }
        }
    }

    ~TcpServer()
    {
    }

private:
    void service(std::shared_ptr<Socket> sockfd, InetAddr &clientaddr)
    {
        std::string inbuffer, outbuffer;
        while (true)
        {
            outbuffer.clear();

            int n = sockfd->Recv(&inbuffer); // 这里的 sockfd 是 accept 返回的 sockfd ，所以调用的是TcpSocket中的_sockfd，是accept返回的
            if (n <= 0)
            {
                LOG(LogLevel::WARNING) << "recv: client quit, " << clientaddr.ToString();
                break;
            }
            
            LOG(LogLevel::DEBUG) << "inbuffer:\n" << inbuffer;

            if (_handler)
                outbuffer += _handler(inbuffer);
            
            if (outbuffer.empty())
            {
                continue;
            }

            LOG(LogLevel::DEBUG) << "outbuffer:\n" << outbuffer;

            n = sockfd->Send(outbuffer);
            if (n < 0)
            {
                LOG(LogLevel::WARNING) << "send: client quit, " << clientaddr.ToString();
                break;
            }
        }
    }

private:
    uint16_t _port;
    std::unique_ptr<TcpSocket> _listensock;
    handler_t _handler;
};