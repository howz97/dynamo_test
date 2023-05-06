#include <atomic>

#include <aws/dynamodb/model/GetItemRequest.h>

#include "common.h"

#define CLIENT client[id % num_clients]

// mu protect cv and inflight
std::mutex mu[num_threads];
std::condition_variable cv[num_threads];
uint16_t inflight[num_threads];

std::unique_ptr<Aws::DynamoDB::DynamoDBClient> client[num_clients];
std::atomic<bool> killed = false;
std::atomic<uint32_t> counter = 0;

struct CbCtx : Aws::Client::AsyncCallerContext {
  CbCtx(uint16_t id) : worker_id(id){};
  uint16_t worker_id;
};

void callback(
    const Aws::DynamoDB::DynamoDBClient *client,
    const Aws::DynamoDB::Model::GetItemRequest &req,
    const Aws::DynamoDB::Model::GetItemOutcome &outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext> &ctx) {
  auto cbctx = std::static_pointer_cast<const CbCtx>(ctx);
  counter++;
  mu[cbctx->worker_id].lock();
  inflight[cbctx->worker_id]--;
  mu[cbctx->worker_id].unlock();
  cv[cbctx->worker_id].notify_one();
}

void worker(uint16_t id) {
  auto ctx = std::make_shared<CbCtx>(id);
  while (!killed) {
    mu[id].lock();
    inflight[id]++;
    mu[id].unlock();

    Aws::DynamoDB::Model::GetItemRequest req;
    req.SetTableName(dynamo::table_name);
    req.AddKey("id", AttributeValue().SetN(
                         std::to_string(rand() % dynamo::table_size)));
    CLIENT->GetItemAsync(req, callback, ctx);
    assert(inflight[id] <= coro_per_thd);
    // check without mutex lock
    if (inflight[id] == coro_per_thd) {
      std::unique_lock lk(mu[id]);
      cv[id].wait(lk, [id] { return inflight[id] < coro_per_thd; });
    }
  }
  std::unique_lock lk(mu[id]);
  cv[id].wait(lk, [id] { return inflight[id] == 0; });
}

int main(int argc, char *argv[]) {
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  for (uint16_t i = 0; i < num_clients; ++i) {
    Aws::Client::ClientConfiguration config;
    config.region = dynamo::region;
    config.endpointOverride = dynamo::endpoint;
    config.executor =
        Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>(
            "dynamo-executor", pool_size);
    client[i] = std::make_unique<Aws::DynamoDB::DynamoDBClient>(config);
  }

  char *cmd = argv[1];
  if (argc > 1) {
    if (strcmp(cmd, "prepare") == 0) {
      prepare(client[0].get());
    } else if (strcmp(cmd, "cleanup") == 0) {
      cleanup(client[0].get());
    } else {
      std::cout << "unknown argument" << std::endl;
    }
  } else {
    std::vector<std::thread> threads;
    for (uint16_t id = 0; id < num_threads; ++id) {
      threads.emplace_back(worker, id);
    }
    std::this_thread::sleep_for(std::chrono::seconds(run_seconds));
    killed = true;
    uint32_t qps = counter / run_seconds;
    std::cout << "QPS: " << qps << std::endl;
    for (uint16_t id = 0; id < num_threads; ++id) {
      threads[id].join();
    }
  }
  for (uint16_t i = 0; i < num_clients; ++i) {
    client[i] = nullptr;
  }
  Aws::ShutdownAPI(options);
  return 0;
}