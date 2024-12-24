#include "zip.h"

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <filesystem>
#include <shared_mutex>
#include <string>
#include <unordered_set>

class CatsManager
{
public:
    void CollectCat(std::filesystem::path &&path);
    unsigned int CatsAmount();

private:
    void FetchCat();
    bool IsCatUnique();

    std::unordered_set<std::filesystem::path, std::hash<std::filesystem::path>> cats_;
    std::shared_mutex mutex_;
};

class AsyncDownloadTask : public std::enable_shared_from_this<AsyncDownloadTask>
{

public:
    AsyncDownloadTask(const boost::asio::ip::tcp::endpoint &endpoint, const std::shared_ptr<CatsManager> cats_manager, boost::asio::io_context &io_context, boost::system::error_code ec)
        : manager_(cats_manager), endpoint_(endpoint), sock_(io_context)
    {
    }

    void handle();

private:
    void Request(boost::system::error_code ec);
    void OnWritten(size_t bytes_written, boost::system::error_code ec);
    void OnReadingFinished(size_t bytes_read, boost::system::error_code read_ec);

    std::shared_ptr<CatsManager> manager_;

    boost::asio::ip::tcp::socket sock_;
    boost::asio::ip::tcp::endpoint endpoint_;

    boost::asio::streambuf request_;
    boost::asio::streambuf response_;
};