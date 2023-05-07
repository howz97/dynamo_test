#include "sync.h"

void sync_worker(uint16_t id, Aws::DynamoDB::DynamoDBClient *client) {
  while (!killed) {
    GetItemRequest req = rand_get_item_req();
    GetItemOutcome outcome = client->GetItem(req);
    if (!outcome.IsSuccess()) {
      std::cout << outcome.GetError() << std::endl;
      killed = true;
    }
    counter++;
  }
}
