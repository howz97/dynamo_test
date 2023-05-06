#include "sync.h"

void sync_worker(uint16_t id, Aws::DynamoDB::DynamoDBClient *client) {
  while (!killed) {
    Aws::DynamoDB::Model::GetItemRequest req;
    req.SetTableName(dynamo::table_name);
    req.AddKey("id", AttributeValue().SetN(
                         std::to_string(rand() % dynamo::table_size)));
    GetItemOutcome outcome = client->GetItem(req);
    if (!outcome.IsSuccess()) {
      std::cout << outcome.GetError() << std::endl;
      killed = true;
    }
  }
}
