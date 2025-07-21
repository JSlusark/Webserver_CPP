#include "WebServer.hpp"

extern Config g_config;

// Send a file as a response (with correct Content-Type)
void WebServer::send_file_response(int client_fd, const std::string& path, size_t i) {
    std::string body = read_file(path);
    if (body.empty()) {
        Logger::log(LOG_ERROR, "send_file_response", "File not found or empty: " + path);
        send_error_response(client_fd, 404, "Not Found", i);
        return;
    }
    Logger::log(LOG_INFO, "send_file_response", "Sending file: " + path);
    Response(client_fd, 200, "OK", body, single_header("Content-Type", get_mime_type(path)));
    cleanup_client(client_fd, i);
}

// Send a redirect response
void WebServer::send_redirect_response(int client_fd, int code, const std::string& location, size_t i) {
    std::ostringstream body;
    body << "<html><head><title>" << code << " Redirect</title></head><body>"
         << "<h1>" << code << " Redirect</h1>"
         << "<p>Redirecting to <a href=\"" << location << "\">" << location << "</a></p>"
         << "</body></html>";
    Logger::log(LOG_INFO, "send_redirect_response", "Redirecting to: " + location + " (code " + to_str(code) + ")");
    Response(client_fd, code, "Redirect", body.str(), redirect_headers(location));
    cleanup_client(client_fd, i);
}

void WebServer::send_ok_response(int client_fd, const std::string& body, const std::map<std::string, std::string>& headers, size_t i) {
    Logger::log(LOG_INFO, "send_ok_response", "Sending 200 OK response.");
    Response(client_fd, 200, "OK", body, headers);
    cleanup_client(client_fd, i);
}

void WebServer::send_upload_success_response(int client_fd, const std::string& full_filename, size_t i) {
    std::ostringstream body;
    body << "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
         << "<title>Upload Successful</title></head><body style=\"font-family:sans-serif;text-align:center;margin-top:50px;\">"
         << "<h1>✅ File uploaded successfully!</h1>"
         << "<p>Saved as: <code>" << full_filename << "</code></p>"
         << "<br><br>"
         << "<a href=\"/\" style=\"margin: 0 10px;\"><button>Home</button></a>"
         << "<a href=\"/about\" style=\"margin: 0 10px;\"><button>About</button></a>"
         << "<a href=\"/static/upload\" style=\"margin: 0 10px;\"><button>Upload Another</button></a>"
         << "</body></html>";

    Logger::log(LOG_INFO, "send_upload_success_response", "Upload successful: " + full_filename);
    send_ok_response(client_fd, body.str(), single_header("Content-Type", "text/html; charset=utf-8"), i);
}

// Send a standard error response and cleanup
/*void WebServer::send_error_response(int client_fd, int code, const std::string& msg, size_t i) {
    std::ostringstream oss;
    oss << "<h1>" << code << " " << msg << "</h1>";
    std::string body = oss.str();
    Logger::log(LOG_ERROR, "send_error_response", "Sending error " + to_str(code) + ": " + msg);
    Response(client_fd, code, msg, body, content_type_html());
    cleanup_client(client_fd, i);
}*/

void WebServer::send_error_response(int client_fd, int code, const std::string& msg, size_t i) {
    const std::string* err_page = g_config.getErrorPage(code);
    std::string body;

    if (err_page && !err_page->empty()) {
        std::string resolved_path = resolve_path(*err_page, "GET");
        Logger::log(LOG_DEBUG, "send_error_response", "Trying custom error page: " + resolved_path);

        if (file_exists(resolved_path) && access(resolved_path.c_str(), R_OK) == 0) {
            body = read_file(resolved_path);
            if (!body.empty()) {
                Logger::log(LOG_INFO, "send_error_response", "Using custom error page for code " + to_str(code));
                Response(client_fd, code, msg, body, content_type_html());
                cleanup_client(client_fd, i);
                return;
            }
        } else {
            Logger::log(LOG_ERROR, "send_error_response", "Custom error page not found or not readable: " + resolved_path);
        }
    }

    // Fallback default error HTML
    std::ostringstream oss;
    oss << "<!DOCTYPE html><html><head><title>" << code << " " << msg
        << "</title></head><body><h1>" << code << " " << msg
        << "</h1><p>The server could not fulfill your request.</p></body></html>";
    body = oss.str();

    Logger::log(LOG_INFO, "send_error_response", "Using default error page for code " + to_str(code));
    Response(client_fd, code, msg, body, content_type_html());
    cleanup_client(client_fd, i);
}

