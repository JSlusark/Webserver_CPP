#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <string>
#include <map>

class Response {
public:
    Response();

    void setStatus(int code, const std::string& message);
    void setHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);
    std::string toString() const;

private:
    int status_code;
    std::string status_message;
    std::map<std::string, std::string> headers;
    std::string body;
};

#endif
