#include "servant/Communicator.h"
#include "StorageService.h"
#include "bcos-storage/Storage.h"
#include "../StorageService/StorageServiceClient.h"
#include "../StorageService/StorageServiceServer.h"
#include "bcos-framework/interfaces/storage/StorageInterface.h"
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>

using namespace bcostars;

void summary(const std::string &desc, clock_t cost, size_t successed,
             size_t failed) {
  std::cout << "===================================================="
            << std::endl;
  std::cout << desc << " request done!" << std::endl;
  std::cout << "Success: " << successed << std::endl;
  std::cout << "Failed: " << failed << std::endl;
  std::cout << "Total time cost: " << cost << " ms" << std::endl;
  std::cout << "TPS: " << (double)successed / (double)cost * 1000 << std::endl;
  std::cout << "===================================================="
            << std::endl;
}

void testPut(const bcos::storage::StorageInterface::Ptr &client,
             const std::vector<std::tuple<std::string, std::string>> &list,
             bool wait) {
  std::atomic<size_t> successed = 0;
  std::atomic<size_t> failed = 0;

  std::mutex lock;
  if (wait) {
    lock.lock();
  }

  clock_t start = clock();
  auto count = list.size();
  for (auto const &it : list) {
    client->asyncPut("default", std::get<0>(it), std::get<1>(it),
                     [&successed, &failed, &lock, count, start,
                      wait](const bcos::Error::Ptr &error) {
                       if (error && error->errorCode()) {
                         std::cout << "Error while put: " << error->errorCode()
                                   << " " << error->errorMessage() << std::endl;
                         ++failed;
                       } else {
                         ++successed;
                       }

                       if (successed + failed == count) {
                         clock_t cost = (clock() - start) / 1000;

                         summary("Put", cost, successed, failed);

                         if (wait) {
                           lock.unlock();
                         }
                       }
                     });
  }

  if (wait) {
    lock.lock();
    lock.unlock();
  }
}

void testGet(const bcos::storage::StorageInterface::Ptr &client,
             const std::vector<std::tuple<std::string, std::string>> &list,
             bool wait) {
  std::atomic<size_t> successed = 0;
  std::atomic<size_t> failed = 0;

  std::mutex lock;
  if (wait) {
    lock.lock();
  }

  clock_t start = clock();
  auto count = list.size();
  size_t index = 0;
  for (auto const &it : list) {
    client->asyncGet(
        "default", std::get<0>(it),
        [&successed, &failed, &lock, count, start, &list, index,
         wait](const bcos::Error::Ptr &error, const std::string &value) {
          if (error && error->errorCode()) {
            std::cout << "Error while put!" << std::endl;
            ++failed;
          } else {
            ++successed;

            auto const &except = std::get<1>(list[index]);
            if (value != except) {
              std::cout << "Value mismatch!" << std::endl;
            }
          }

          if (successed + failed == count) {
            clock_t cost = (clock() - start) / 1000;

            summary("Get", cost, successed, failed);

            if (wait) {
              lock.unlock();
            }
          }
        });

    ++index;
  }

  if (wait) {
    lock.lock();
    lock.unlock();
  }
}

std::string genRandomString(size_t length) {
  std::string str;
  str.reserve(length);

  std::string chars("abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "1234567890"
                    "!@#$%^&*()"
                    "`~-_=+[{]}\\|;:'\",<.>/? ");

  std::random_device rng;
  std::uniform_int_distribution<> index_dist(0, chars.size() - 1);
  for (size_t i = 0; i < length; ++i) {
    str.push_back(chars[index_dist(rng)]);
  }

  return str;
}

void localBasedTest(
    const std::vector<std::tuple<std::string, std::string>> &list) {
  // load the config
  bcos::storage::RocksDBAdapterFactory rocksdbAdapterFactory("./db");
  auto rocksdbAdapter = rocksdbAdapterFactory.createAdapter("bcos_storage");
  auto ret = rocksdbAdapterFactory.createRocksDB("kv_storage");
  auto kvDB = std::make_shared<bcos::storage::KVDBImpl>(ret.first);

  auto storageImpl =
      std::make_shared<bcos::storage::StorageImpl>(rocksdbAdapter, kvDB, 16);

  testPut(storageImpl, list, true);
  testGet(storageImpl, list, true);
}

void tarsBasedTest(
    const std::string &ip, unsigned short port,
    const std::vector<std::tuple<std::string, std::string>> &list) {
  tars::CommunicatorPtr communicator = new tars::Communicator();
  StorageServicePrx proxy = communicator->stringToProxy<StorageServicePrx>(
      std::string(StorageServiceClient::servantName) + "@tcp -h " + ip +
      " -p " + boost::lexical_cast<std::string>(port));

  auto client = std::make_shared<StorageServiceClient>(proxy);
  testPut(client, list, false);
  testGet(client, list, false);
}

int main(int argc, char *argv[]) {
  std::string ip;
  unsigned short port;
  size_t count;
  std::string mode;

  boost::program_options::options_description desc("Storage client options.");
  desc.add_options()("help", "produce help message")(
      "mode",
      boost::program_options::value<std::string>(&mode)->default_value("local"),
      "Test mode, local/tars")(
      "ip",
      boost::program_options::value<std::string>(&ip)->default_value(
          "127.0.0.1"),
      "StorageService server ip")(
      "port",
      boost::program_options::value<unsigned short>(&port)->default_value(0),
      "StorageService server port")(
      "count",
      boost::program_options::value<size_t>(&count)->default_value(1000),
      "Total request count");

  boost::program_options::variables_map vm;
  boost::program_options::store(
      boost::program_options::parse_command_line(argc, argv, desc), vm);

  boost::program_options::notify(vm);

  std::vector<std::tuple<std::string, std::string>> list;
  for (size_t i = 0; i < count; ++i) {
    list.push_back(std::make_tuple(genRandomString(32), genRandomString(256)));
  }

  if (mode == "local") {
    localBasedTest(list);
  } else {
    tarsBasedTest(ip, port, list);
  }
}