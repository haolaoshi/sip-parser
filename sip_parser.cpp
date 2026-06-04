#include "sip_message.h"
#include "sip_transaction.h"
#include <stdexcept>
#include <iostream>

static SipRequestMethod methodConvertor(const std::string& methodStr)
{
    if (methodStr == "INVITE") return SipRequestMethod::INVITE;
    if (methodStr == "ACK") return SipRequestMethod::ACK;
    if (methodStr == "BYE") return SipRequestMethod::BYE;
    if (methodStr == "CANCEL") return SipRequestMethod::CANCEL;
    if (methodStr == "OPTIONS") return SipRequestMethod::OPTIONS;
    if (methodStr == "MESSAGE") return SipRequestMethod::MESSAGE;
    if (methodStr == "INFO") return SipRequestMethod::INFO;
    if (methodStr == "UPDATE") return SipRequestMethod::UPDATE;
    if (methodStr == "REGISTER") return SipRequestMethod::REGISTER;
    if (methodStr == "REFER") return SipRequestMethod::REFER;
    if (methodStr == "NOTIFY") return SipRequestMethod::NOTIFY;
    if (methodStr == "PUBLISH") return SipRequestMethod::PUBLISH;
    if (methodStr == "SUBSCRIBE") return SipRequestMethod::SUBSCRIBE;
    throw std::invalid_argument("Unknown SIP method: " + methodStr);
}

 

static std::string methodToString(SipRequestMethod method)
{
    switch (method) {
        case SipRequestMethod::INVITE: return "INVITE";
        case SipRequestMethod::ACK: return "ACK";
        case SipRequestMethod::BYE: return "BYE";
        case SipRequestMethod::CANCEL: return "CANCEL";
        case SipRequestMethod::OPTIONS: return "OPTIONS";
        case SipRequestMethod::MESSAGE: return "MESSAGE";
        case SipRequestMethod::INFO: return "INFO";
        case SipRequestMethod::UPDATE: return "UPDATE";
        case SipRequestMethod::REGISTER: return "REGISTER";
        case SipRequestMethod::REFER: return "REFER";
        case SipRequestMethod::NOTIFY: return "NOTIFY";
        case SipRequestMethod::PUBLISH: return "PUBLISH";
        case SipRequestMethod::SUBSCRIBE: return "SUBSCRIBE";
        default: throw std::invalid_argument("Unknown SIP method");
    }
}

static std::string stateOfTransactionToString(SipTransactionState state)
{
    switch (state) {
        case SipTransactionState::Idle: return "Idle";
        case SipTransactionState::Calling: return "Calling";
        case SipTransactionState::Proceeding: return "Proceeding";
        case SipTransactionState::Connected: return "Connected";
        case SipTransactionState::Terminated: return "Terminated";
        default: throw std::invalid_argument("Unknown SIP transaction state");
    }
}

static std::string eventOfTransactionToString(SipTransactionEvent event)
{
    switch (event) {
        case SipTransactionEvent::SendInvite: return "SendInvite";
        case SipTransactionEvent::Received100Or180: return "Received100Or180";
        case SipTransactionEvent::Received183: return "Received183";
        case SipTransactionEvent::Received200OK: return "Received200OK";
        case SipTransactionEvent::Received4xx5xx6xx: return "Received4xx5xx6xx";
        case SipTransactionEvent::ReceivedBYE: return "ReceivedBYE";
        case SipTransactionEvent::SendCancel: return "SendCancel";
        case SipTransactionEvent::SendBYE: return "SendBYE";
        case SipTransactionEvent::TimerBTimeout: return "TimerBTimeout";
        default: throw std::invalid_argument("Unknown SIP transaction event");
    }
}

SipMessage parse(const std::string& raw)
{
    SipMessage message;
 
    std::vector<std::string> lines = [&raw]() {
        std::vector<std::string> result;
        size_t start = 0, end;
        while ((end = raw.find("\r\n", start)) != std::string::npos) {
            result.push_back(raw.substr(start, end - start));
            start = end + 2; // 跳过\r\n
        }
        if (start < raw.size()) {
            result.push_back(raw.substr(start)); // 添加最后一行
        }
       
        return result;
    }();

    std::string requestLine = lines[0];

    // 解析请求行或响应行
    if (requestLine.find("SIP/2.0") == 0) {
        // 响应行
        size_t spacePos = requestLine.find(' ');
        size_t secondSpacePos = requestLine.find(' ', spacePos + 1);
        message.type = Response{static_cast<SipResponseStatusCode>(std::stoi(requestLine.substr(spacePos + 1, 3))), requestLine.substr(secondSpacePos + 1)};
    } else {
        // 请求行
        size_t spacePos = requestLine.find(' ');
        message.type = Request{methodConvertor(requestLine.substr(0, spacePos)), requestLine.substr(spacePos + 1, requestLine.find(' ', spacePos + 1) - spacePos - 1)};
    }
   
    int i = 1;
    for (; i < lines.size(); ++i) {
        if (lines[i].empty()) { 
            break; // 空行，后面是消息体 
        }
        size_t colonPos = lines[i].find(':');
        if (colonPos != std::string::npos) {
            std::string headerName = lines[i].substr(0, colonPos);
            std::string headerValue = lines[i].substr(colonPos + 1); // 注意：这里没有去除headerValue前后的空格
            // 去除headerValue前后的空格
            headerValue.erase(0, headerValue.find_first_not_of(" \t"));
            headerValue.erase(headerValue.find_last_not_of(" \t") + 1);

            message.headers[headerName].push_back(headerValue);
        }
    }

    size_t contentLength = message.headers.count("Content-Length") > 0 ? std::stoul(message.headers["Content-Length"][0]) : 0;
    if (contentLength > 0) {
        size_t bodyStartPos = raw.find("\r\n\r\n");
        if (bodyStartPos != std::string::npos) {
            message.body = raw.substr(bodyStartPos + 4, contentLength); // 跳过\r\n\r\n
        }
    }
    return message;

}


static void printMessage(const std::string& str) 
{
    try {
        SipMessage message = parse(str);
        // 输出解析结果
        if (std::holds_alternative<Request>(message.type)) {
            std::cout << methodToString(std::get<Request>(message.type).method) << " " << std::get<Request>(message.type).uri << std::endl;
        } else {
            std::cout << static_cast<int>(std::get<Response>(message.type).statusCode) << " " << std::get<Response>(message.type).reasonPhrase << std::endl;
        }
        for (const auto& header : message.headers) {
            for (const auto& value : header.second) {
                std::cout << header.first << " Value: " << value << std::endl;
            }
        }
        if (!message.body.empty()) {
            std::cout <<message.body << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing SIP message: " << e.what() << std::endl;
    }
}

/**
 * 
 * Idle → SendInvite → Calling
  Calling → Received180 → Proceeding
  Proceeding → Received200OK → Connected
  Connected → SendBYE → Terminated 
 */
void printTestFlow(SipTransactionState state, SipTransactionEvent event)
{
    std::cout << "Current state: " << stateOfTransactionToString(state) << " -> " << eventOfTransactionToString(event) << " -> " ;
    try {
        SipTransactionState nextState = transition(state, event);
        std::cout << stateOfTransactionToString(nextState) << " [PASS]" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error during state transition: " << e.what() << std::endl;
    }
   
}
void testCallFlow()
{
    printTestFlow(SipTransactionState::Idle, SipTransactionEvent::SendInvite); // Idle -> Calling
    printTestFlow(SipTransactionState::Calling, SipTransactionEvent::Received100Or180); // Calling -> Proceeding
    printTestFlow(SipTransactionState::Proceeding, SipTransactionEvent::Received200OK); // Proceeding -> Connected
    printTestFlow(SipTransactionState::Connected, SipTransactionEvent::SendBYE); // Connected -> Terminated
} 

int main()
{
    std::string rawRequest =  "INVITE sip:1000@192.168.1.1 SIP/2.0";
    //printMessage(rawRequest);
    std::string rawResponse = "SIP/2.0 200 OK";
    //printMessage(rawResponse);
    std::string rawRequestWithHeaders = "INVITE sip:1000@192.168.1.1 SIP/2.0 \r\n\
    Via: SIP/2.0/UDP 192.168.1.2:5060;branch=z9hG4bK776asdhds \r\n\
    Via: SIP/2.0/UDP 192.168.1.3:5060;branch=z9hG4bK123 \r\n\
    From: <sip:alice@192.168.1.2>;tag=1928301774 \r\n\
    To: <sip:1000@192.168.1.1> \r\n\
    Call-ID: a84b4c76e66710@192.168.1.2 \r\n\
    CSeq: 314159 INVITE \r\n\
    Content-Length: 0  \r\n\
    ";
    std::string raw =
      "INVITE sip:1000@192.168.1.1 SIP/2.0\r\n"
      "Content-Length: 4\r\n"
      "\r\n"
      "Test";
    //printMessage(raw);
    testCallFlow();
}
