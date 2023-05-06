#include "common.h"

struct CbCtx : Aws::Client::AsyncCallerContext {
  CbCtx(uint16_t id) : worker_id(id){};
  uint16_t worker_id;
};

void callback(
    const Aws::DynamoDB::DynamoDBClient *client,
    const Aws::DynamoDB::Model::GetItemRequest &req,
    const Aws::DynamoDB::Model::GetItemOutcome &outcome,
    const std::shared_ptr<const Aws::Client::AsyncCallerContext> &ctx);

void async_worker(uint16_t id, Aws::DynamoDB::DynamoDBClient *client);
