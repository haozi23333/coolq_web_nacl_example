// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdio.h>
#include <string.h>
#include <sstream>

#include "ppapi/cpp/host_resolver.h"
#include "ppapi/cpp/instance.h"
#include "ppapi/cpp/module.h"
#include "ppapi/cpp/tcp_socket.h"
#include "ppapi/cpp/udp_socket.h"
#include "ppapi/cpp/var.h"
#include "ppapi/utility/completion_callback_factory.h"

#define CQ_PERFIX "CQ_"

const int kBufferSize = 40960;

class CoolQServer : public pp::Instance {
public:
    explicit CoolQServer(PP_Instance instance)
            : pp::Instance(instance),
              callback_factory_(this){
        // 直接开始解析地址, 链接服务器
        get_host_resolve("192.168.50.126:11235");
    }

    // 析构函数, 释放资源, 没啥释放的
    virtual ~CoolQServer() {
    }

    // 处理由 Chrome PostMessage 过来的信息
    virtual void HandleMessage(const pp::Var& var_message) {
        if (!var_message.is_string())
            return;
        std::string message = var_message.AsString();
        if (message == "say_hello") {
            say_hello();
            return;
        }
        send_message(message);
    }

protected:
    pp::UDPSocket udp_socket_;
    pp::HostResolver resolver_;
    pp::NetAddress remote_host_;
    char receive_buffer_[kBufferSize];

private:
    pp::CompletionCallbackFactory<CoolQServer> callback_factory_;

    //
    void get_host_resolve(const std::string& host) {
        udp_socket_ = pp::UDPSocket(this);
        resolver_ = pp::HostResolver(this);
        if (resolver_.is_null()) {
            PostMessage("Error creating HostResolver.");
            return;
        }

        int port = 80;
        std::string hostname = host;
        size_t pos = host.rfind(':');
        if (pos != std::string::npos) {
            hostname = host.substr(0, pos);
            port = atoi(host.substr(pos+1).c_str());
        }

        pp::CompletionCallback callback =
                callback_factory_.NewCallback(&CoolQServer::get_host_resolve_callack);
        PP_HostResolver_Hint hint = { PP_NETADDRESS_FAMILY_UNSPECIFIED, 0 };
        resolver_.Resolve(hostname.c_str(), port, hint, callback);
        PostMessage("Resolving ...");
    }

    // 上面函数的 回调
    void get_host_resolve_callack(int32_t result) {
        if (result != PP_OK) {
            PostMessage("Resolve failed.");
            return;
        }

        pp::NetAddress addr = resolver_.GetNetAddress(0);
        PostMessage(std::string("Resolved: ") +
                    addr.DescribeAsString(true).AsString());

        // 创建本地 UDP Server
        pp::CompletionCallback callback =
                callback_factory_.NewCallback(&CoolQServer::bind_local_server_callback);

        PostMessage("Binding ...");
        remote_host_ = addr;
        PP_NetAddress_IPv4 ipv4_addr = { 0, { 0 } };
        udp_socket_.Bind(pp::NetAddress(this, ipv4_addr), callback);
    }

    //创建本地 UDP Server 的回调
    void bind_local_server_callback(int32_t result) {
        if (result != PP_OK) {
            std::ostringstream status;
            status << "Connection failed: " << result;
            PostMessage(status.str());
            return;
        }
        PostMessage("本地 udp 创建成功");
        pp::NetAddress addr = udp_socket_.GetBoundAddress();
        PostMessage("SYSTEM:SUCCESS");

        // 接受信息
        receive_message();
    }

    // 发送握手信息
    void say_hello() {
        std::string host = udp_socket_.GetBoundAddress().DescribeAsString(true).AsString();
        std::string hostname = host;
        int port = 80;
        size_t pos = host.rfind(':');
        if (pos != std::string::npos) {
            hostname = host.substr(0, pos);
            port = atoi(host.substr(pos+1).c_str());
        }
        std::ostringstream hello;
        hello << "ClientHello " << port << " " << "192.168.50.231" ;
        send_message(hello.str());
        PostMessage(hello.str());
    }

    // 发送信息
    void send_message(const std::string& message) {
        uint32_t size = message.size();
        const char* data = message.c_str();
        pp::CompletionCallback callback =
                callback_factory_.NewCallback(&CoolQServer::send_message_callback);
        int32_t result;
        result = udp_socket_.SendTo(data, size, remote_host_, callback);
        std::ostringstream status;
        if (result < 0) {
            if (result == PP_OK_COMPLETIONPENDING) {
                status << "Sending bytes: " << size;
                PostMessage(status.str());
            } else {
                status << "Send returned error: " << result;
                PostMessage(status.str());
            }
        } else {
            status << "Sent bytes synchronously: " << result;
            PostMessage(status.str());
        }
    }

    // 发送信息的 回调
    void send_message_callback(int32_t result) {
        std::ostringstream status;
        if (result < 0) {
            status << "Send failed with: " << result;
        } else {
            status << "Sent bytes: " << result;
        }
        PostMessage(status.str());
    }

    // 接受信息
    void receive_message() {
        memset(receive_buffer_, 0, kBufferSize);
        pp::CompletionCallbackWithOutput<pp::NetAddress> callback =
                callback_factory_.NewCallbackWithOutput(
                        &CoolQServer::receive_message_callback);
        udp_socket_.RecvFrom(receive_buffer_, kBufferSize, callback);
    }

    // 接受信息的回调
    void receive_message_callback(int32_t result,
                                  pp::NetAddress source) {
        if (result < 0) {
            std::ostringstream status;
            status << "Receive failed with: " << result;
            PostMessage(status.str());
            return;
        }

        PostMessage(std::string(CQ_PERFIX) + ":" + std::string(receive_buffer_, result));
        receive_message();
    }
};


// 导出的 Module
class ExampleModule : public pp::Module {
public:
    ExampleModule() : pp::Module() {}
    virtual ~ExampleModule() {}

    virtual pp::Instance* CreateInstance(PP_Instance instance) {
        return new CoolQServer(instance);
    }
};


// 类似初始换函数, 可以被 Chrome 自动调用
namespace pp {
    Module* CreateModule() { return new ExampleModule(); }
}
