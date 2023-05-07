#include "common.h"

std::string random_string(const int len) {
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
  // create table
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
    std::cout << "CreateTable: " << outcome.GetError() << std::endl;
    assert(false);
  }
  // block until creating table finished
  DescribeTableRequest desc_req;
  desc_req.SetTableName(dynamo::table_name);
  while (true) {
    DescribeTableOutcome outcome = client->DescribeTable(desc_req);
    if (!outcome.IsSuccess()) {
      std::cout << "DescribeTable: " << outcome.GetError().GetExceptionName()
                << std::endl;
      continue;
    }
    if (outcome.GetResult().GetTable().GetTableStatus() ==
        TableStatus::CREATING) {
      std::cout << "DescribeTable: creating..." << std::endl;
      continue;
    }
    std::this_thread::sleep_for(std::chrono::seconds(1));
    break;
  }
  // fill data
  for (uint32_t key = 0; key < dynamo::table_size; ++key) {
    PutItemRequest req;
    req.SetTableName(dynamo::table_name);
    req.AddItem("id", AttributeValue().SetN(std::to_string(key)));
    req.AddItem("c1", AttributeValue().SetS(random_string(24)));
    req.AddItem("c2", AttributeValue().SetS(random_string(100)));
    PutItemOutcome outcome = client->PutItem(req);
    if (!outcome.IsSuccess()) {
      std::cout << "PutItem: " << outcome.GetError() << std::endl;
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
