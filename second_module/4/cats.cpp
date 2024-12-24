#include "cats.hpp"

#include <boost/filesystem.hpp>

#include <optional>
#include <functional>
#include <iostream>
#include <fstream>

#define CATS_AMOUNT 12

namespace
{

    void trim_end(std::string &str)
    {
        while (!str.empty() && std::iswspace(str.back()))
        {
            str.pop_back();
        }
    }

    void UploadZip(const boost::asio::ip::tcp::endpoint &endpoint, const std::filesystem::path &zip_path, boost::asio::io_context &io_context)
    {
        std::ifstream file_stream(zip_path.c_str(), std::ios::binary);
        std::ostringstream oss;
        oss << file_stream.rdbuf();
        auto raw_file = oss.str();

        boost::asio::ip::tcp::socket sock(io_context);
        sock.connect(endpoint);

        std::string boundary = "----WebKitFormBoundary7MA4YWxkTrZu0gW";
        std::string body;
        body += "--" + boundary + "\r\n";
        body += "Content-Disposition: form-data; name=\"file\"; filename=\"" + std::string(zip_path.filename().c_str()) + "\"\r\n";
        body += "Content-Type: application/zip\r\n\r\n";
        body += raw_file + "\r\n";
        body += "--" + boundary + "--\r\n";

        boost::asio::streambuf request;
        std::ostream(&request) << "POST /cat HTTP/1.1\r\n";
        std::ostream(&request) << "Content-Type: multipart/form-data; boundary=" << boundary << "\r\n";
        std::ostream(&request) << "Content-Length: " << body.size() << "\r\n"; // Add content length
        std::ostream(&request) << "Connection: close\r\n";
        std::ostream(&request) << "\r\n";
        std::ostream(&request) << body;
        boost::asio::write(sock, request);

        // Read the response
        boost::asio::streambuf response;
        boost::asio::read_until(sock, response, "\r\n");

        sock.close();

        std::istream response_stream(&response);
        std::string http_version;
        unsigned int status_code;
        response_stream >> http_version >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        std::cout << "Response: " << http_version << " " << status_code << " " << status_message << "\n";
    }

} // namespace

std::optional<std::filesystem::path> CatsManager::MakeZip()
{
    auto path = std::filesystem::path{"pics/output.zip"};
    zip_t *zip = zip_open(path.c_str(), ZIP_CREATE, NULL);

    if (zip == NULL)
    {
        std::cerr << "Error initializing zip library" << std::endl;
        return std::nullopt;
    }

    for (const auto &file_path : cats_)
    {
        zip_source_t *source = zip_source_file(zip, file_path.c_str(), 0, 0);
        if (source == nullptr)
        {
            std::cerr << "Failed to get source file: " << file_path.c_str() << std::endl;
            zip_source_free(source);
        }
        zip_file_add(zip, file_path.c_str(), source, ZIP_FL_OVERWRITE);
    }

    zip_close(zip);
    return path;
}

void CatsManager::CollectCat(std::filesystem::path &&path)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (!cats_.count(path))
    {
        cats_.insert(std::move(path));
        std::cout << "already fetched: " << cats_.size() << " cats" << std::endl;
    }
}

unsigned int CatsManager::CatsAmount()
{
    return cats_.size();
}

void AsyncDownloadTask::handle()
{
    sock_.async_connect(endpoint_, std::bind(&AsyncDownloadTask::Request,
                                             shared_from_this(),
                                             std::placeholders::_1));
}

void AsyncDownloadTask::Request(boost::system::error_code ec)
{
    assert(!ec);
    std::ostream(&request_) << "GET /cat HTTP/1.1\r\n\r\n";

    using namespace std::placeholders;
    boost::asio::async_write(
        sock_, request_,
        std::bind(&AsyncDownloadTask::OnWritten, shared_from_this(), _2,
                  _1));
}

void AsyncDownloadTask::OnWritten(size_t bytes_written, boost::system::error_code
                                                            ec)
{
    assert(!ec);
    (void)bytes_written;
    using namespace std::placeholders;
    boost::asio::async_read(sock_, response_, boost::asio::transfer_all(),
                            std::bind(&AsyncDownloadTask::OnReadingFinished,
                                      shared_from_this(), _2, _1));
}

void AsyncDownloadTask::OnReadingFinished(size_t bytes_read,
                                          boost::system::error_code read_ec)
{
    (void)bytes_read;

    if (read_ec && read_ec.value() != 2)
    {
        std::cerr << read_ec.message() << " | " << read_ec.value() << std::endl;
        assert(!read_ec);
    }

    std::string line;
    std::istream resp_stream(&response_);
    while (!resp_stream.eof())
    {
        std::getline(resp_stream, line);
        trim_end(line);
        if (line.empty())
            break;
    }
    sock_.close();

    boost::filesystem::create_directory("pics/");
    auto path = "pics/" + std::to_string(rand()) + ".jpg";
    std::ofstream file(path);
    file << resp_stream.rdbuf();
    manager_->CollectCat(std::move(path));
}

int main()
{
    boost::asio::io_context io_context;
    boost::system::error_code ec{};
    const auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("45.138.24.47", ec), 8890);

    const auto cats_manager = std::make_shared<CatsManager>();

    for (int i = 1; i <= 1; ++i)
    {
        auto task = std::make_shared<AsyncDownloadTask>(endpoint, cats_manager, io_context, ec);
        task->handle();
    }

    std::vector<std::thread> threads;
    for (int i = 0; i < std::min(12, 1); ++i)
    {
        threads.emplace_back([&io_context]
                             { io_context.run(); });
    }

    io_context.run();

    for (auto &thread : threads)
    {
        thread.join();
    }
    auto path = cats_manager->MakeZip();
    if (!path.has_value())
    {
        std::cerr << "Failed to make zip file";
        return 1;
    }
    UploadZip(endpoint, path.value(), io_context);

    return 0;
}
