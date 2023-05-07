#pragma once
#include <atomic>

#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/CreateTableRequest.h>
#include <aws/dynamodb/model/DeleteTableRequest.h>
#include <aws/dynamodb/model/GetItemRequest.h>
#include <aws/dynamodb/model/PutItemRequest.h>

using namespace Aws::DynamoDB::Model;

namespace dynamo {
static constexpr char const *region = "ap-northeast-1";
static constexpr char const *endpoint = "http://127.0.0.1:8050";
static constexpr char const *table_name = "test.rand";
static constexpr uint32_t table_size = 1000;
} // namespace dynamo

static constexpr uint16_t run_seconds = 3;

// static constexpr uint16_t num_threads = 2;
// static constexpr uint16_t num_clients = 2;
// static constexpr uint16_t pool_size = 32;

static constexpr uint16_t num_threads = 1;
static constexpr uint16_t num_clients = 1;
static constexpr uint16_t pool_size = 0;

inline std::atomic<bool> killed = false;
inline std::atomic<uint32_t> counter = 0;
inline std::unique_ptr<Aws::DynamoDB::DynamoDBClient> client[num_clients];

std::string random_string(const int len);

void prepare(Aws::DynamoDB::DynamoDBClient *client);

void cleanup(Aws::DynamoDB::DynamoDBClient *client);

template <typename F> void run_test(F worker) {
  std::vector<std::thread> threads;
  for (uint16_t id = 0; id < num_threads; ++id) {
    threads.emplace_back(worker, id, client[id % num_clients].get());
  }
  std::this_thread::sleep_for(std::chrono::seconds(run_seconds));
  killed = true;
  for (uint16_t id = 0; id < num_threads; ++id) {
    threads[id].join();
  }
  uint32_t qps = counter / run_seconds;
  std::cout << "QPS: " << qps << std::endl;
}