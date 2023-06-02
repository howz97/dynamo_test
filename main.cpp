#include <atomic>
#include <iostream>

#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/GetItemRequest.h>

using namespace Aws::DynamoDB::Model;

constexpr uint num_threads = 100;
std::atomic<uint> counter = 0;

GetItemRequest get_rand_item_req() {
  GetItemRequest req;
  req.SetTableName("test.rand");
  req.SetConsistentRead(true);
  req.AddKey("id", AttributeValue().SetN(std::to_string(rand() % 1000)));
  return std::move(req);
}

void worker() {
  Aws::Client::ClientConfiguration config;
  config.region = "ap-northeast-1";
  Aws::DynamoDB::DynamoDBClient client(config);
  for (uint i = 0; i < 5; ++i) {
    GetItemRequest req = get_rand_item_req();
    GetItemOutcome outcome = client.GetItem(req);
    if (!outcome.IsSuccess()) {
      std::cout << outcome.GetError() << std::endl;
      break;
    }
    counter++;
  }
}

int main() {
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  auto start = std::chrono::system_clock::now();
  std::vector<std::thread> workers;
  for (uint i = 0; i < num_threads; ++i) {
    workers.emplace_back(worker);
  }
  for (auto &w : workers) {
    w.join();
  }
  uint elapsed = std::chrono::duration_cast<std::chrono::seconds>(
                     std::chrono::system_clock::now() - start)
                     .count();
  std::cout << "finished counter " << counter << std::endl;
  std::cout << "elapsed seconds " << elapsed << std::endl;

  Aws::ShutdownAPI(options);
  return 0;
}