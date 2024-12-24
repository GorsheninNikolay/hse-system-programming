#include "cats.hpp"

#include <boost/filesystem.hpp>

#include <optional>
#include <functional>
#include <iostream>
#include <fstream>

#define CATS_AMOUNT 12

void trim_end(std::string &str)
{
    while (!str.empty() && std::iswspace(str.back()))
    {
        str.pop_back();
    }
}

// std::optional<std::filesystem::path> CatsManager::MakeZip()
// {
//     auto path = std::filesystem::path{"pics/output.zip"};
//     zip_t *zip = zip_open(path.c_str(), ZIP_CREATE, NULL);

//     if (zip == NULL)
//     {
//         std::cerr << "Error initializing zip library" << std::endl;
//         return std::nullopt;
//     }

//     for (const auto &file_path : cats_)
//     {
//         zip_source_t *source = zip_source_file(zip, file_path.c_str(), 0, 0);
//         if (source == nullptr)
//         {
//             std::cerr << "Failed to get source file: " << file_path.c_str() << std::endl;
//             zip_source_free(source);
//         }
//         zip_file_add(zip, file_path.c_str(), source, ZIP_FL_OVERWRITE);
//     }

//     zip_close(zip);
//     return path;
// }

void CatsManager::CollectCat(std::filesystem::path &&path)
{
    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (!cats_.count(path))
    {
        cats_.insert(std::move(path));
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
    std::ostream(&request_) << "GET /cat HTTP/1.0\r\n\r\n";

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

    std::string message = "Hello, World!";

    boost::filesystem::create_directory("pics/");
    auto path = "pics/" + std::to_string(rand()) + ".jpg";
    std::ofstream file(path);
    file << resp_stream.rdbuf();
    // fetcher_->CollectCat(path);
}

int main()
{
    boost::asio::io_context io_context;
    boost::system::error_code ec{};
    const auto endpoint = boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address("45.138.24.47", ec), 8890);

    auto cats_fetcher = std::make_shared<CatsManager>();

    for (int i = 1; i <= 1; ++i)
    {
        auto task = std::make_shared<AsyncDownloadTask>(endpoint, cats_fetcher, io_context, ec);
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

    std::cout << cats_fetcher->CatsAmount() << std::endl;
    // cats_fetcher->MakeZip();

    return 0;
}
