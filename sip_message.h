#ifndef SIP_MESSAGE_H
#define SIP_MESSAGE_H

#include <string>
#include <map>
#include <vector>
#include <variant>

enum class SipRequestMethod
{
    INVITE,
    ACK,
    BYE,
    CANCEL,
    OPTIONS,
    MESSAGE,
    INFO,
    UPDATE,
    REGISTER,
    REFER,
    NOTIFY,
    PUBLISH,
    SUBSCRIBE
};

enum class SipResponseStatusCode
{
    Trying = 100,
    Ringing = 180,
    Progress = 183,
    OK = 200,
    Unauthorized = 401,
    NotFound = 404,
    InternalServerError = 500,
    Decline = 603
};

struct Request{
    SipRequestMethod method;
    std::string uri;
};

struct Response{
    SipResponseStatusCode statusCode;
    std::string reasonPhrase;
};

struct SipMessage
{ 
    std::variant<Request, Response> type; //请求行或响应行 
    std::map<std::string, std::vector<std::string>> headers; //header字段
    std::string body; //消息体
};

#endif // SIP_MESSAGE_H
