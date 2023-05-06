#include <aws/core/Aws.h>
#include <aws/dynamodb/DynamoDBClient.h>
#include <aws/dynamodb/model/CreateTableRequest.h>
#include <aws/dynamodb/model/DeleteTableRequest.h>
#include <aws/dynamodb/model/PutItemRequest.h>

using namespace Aws::DynamoDB::Model;

static constexpr uint16_t num_threads = 1;
static constexpr uint32_t num_connection = 512;
static constexpr uint32_t coro_per_thd = num_connection / num_threads;
static constexpr uint16_t run_seconds = 30;
static constexpr uint16_t pool_size = 64;
static constexpr uint16_t num_clients = 2;

namespace dynamo {
static constexpr char const *region = "ap-northeast-1";
static constexpr char const *endpoint = "http://127.0.0.1:8050";
static constexpr char const *table_name = "test.rand";
static constexpr uint32_t table_size = 1000;
} // namespace dynamo

std::string gen_random(const int len) {
  static const char alphanum[] = "0123456789"
                                 "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                 "abcdefghijklmnopqrstuvwxyz";
  std::string tmp_s;
  tmp_s.reserve(len);

  for (int i = 0; i < len; ++i) {
    tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  return tmp_s;
}

void prepare(Aws::DynamoDB::DynamoDBClient *client) {
  CreateTableRequest crt_req;
  crt_req.SetTableName(dynamo::table_name);
  crt_req.SetBillingMode(BillingMode::PAY_PER_REQUEST);
  crt_req.AddAttributeDefinitions(
      AttributeDefinition().WithAttributeName("id").WithAttributeType(
          ScalarAttributeType::N));
  crt_req.AddKeySchema(
      KeySchemaElement().WithAttributeName("id").WithKeyType(KeyType::HASH));
  CreateTableOutcome outcome = client->CreateTable(crt_req);
  if (!outcome.IsSuccess()) {
    std::cout << outcome.GetError() << std::endl;
    assert(false);
  }

  for (uint32_t key = 0; key < dynamo::table_size; ++key) {
    PutItemRequest req;
    req.SetTableName(dynamo::table_name);
    req.AddItem("id", AttributeValue().SetN(std::to_string(key)));
    req.AddItem("c1", AttributeValue().SetS(gen_random(24)));
    req.AddItem("c2", AttributeValue().SetS(gen_random(100)));
    PutItemOutcome outcome = client->PutItem(req);
    if (!outcome.IsSuccess()) {
      std::cout << outcome.GetError() << std::endl;
      assert(false);
    }
  }
}

void cleanup(Aws::DynamoDB::DynamoDBClient *client) {
  DeleteTableRequest req;
  req.SetTableName(dynamo::table_name);
  DeleteTableOutcome outcome = client->DeleteTable(req);
  if (!outcome.IsSuccess()) {
    std::cout << outcome.GetError() << std::endl;
  }
}