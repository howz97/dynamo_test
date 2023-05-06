#include "async.h"

constexpr uint32_t coro_per_thd = 128;
// mu protect cv and inflight
std::mutex mu[num_threads];
std::condition_variable cv[num_threads];
uint16_t inflight[num_threads];

void callback(
    const Aws::DynamoDB::DynamoDBClient *client,
    const Aws::DynamoDB::Model::GetItemRequest &req,
    const Aws::DynamoDB::Model::GetItemOutcome &outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext> &ctx) {
  if (!outcome.IsSuccess()) {
    std::cout << outcome.GetError() << std::endl;
    killed = true;
  }
  auto cbctx = std::static_pointer_cast<const CbCtx>(ctx);
  counter++;
  mu[cbctx->worker_id].lock();
  inflight[cbctx->worker_id]--;
  mu[cbctx->worker_id].unlock();
  cv[cbctx->worker_id].notify_one();
}

void async_worker(uint16_t id, Aws::DynamoDB::DynamoDBClient *client) {
  auto ctx = std::make_shared<CbCtx>(id);
  while (!killed) {
    mu[id].lock();
    inflight[id]++;
    mu[id].unlock();

    Aws::DynamoDB::Model::GetItemRequest req;
    req.SetTableName(dynamo::table_name);
    req.AddKey("id", AttributeValue().SetN(
                         std::to_string(rand() % dynamo::table_size)));
    client->GetItemAsync(req, callback, ctx);
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
