#include "async.h"
#include "sync.h"

int main(int argc, char *argv[]) {
  assert(argc > 1);
  Aws::SDKOptions options;
  Aws::InitAPI(options);
  for (uint16_t i = 0; i < num_clients; ++i) {
    Aws::Client::ClientConfiguration config;
    config.region = dynamo::region;
    config.endpointOverride = dynamo::endpoint;
    if (pool_size) {
      config.executor =
          Aws::MakeShared<Aws::Utils::Threading::PooledThreadExecutor>(
              "dynamo-executor", pool_size);
    }
    client[i] = std::make_unique<Aws::DynamoDB::DynamoDBClient>(config);
  }

  char *cmd = argv[1];
  if (strcmp(cmd, "prepare") == 0) {
    prepare(client[0].get());
  } else if (strcmp(cmd, "cleanup") == 0) {
    cleanup(client[0].get());
  } else if (strcmp(cmd, "run") == 0) {
    if (argc == 3 && strcmp(argv[2], "sync") == 0) {
      run_test(sync_worker);
    } else {
      run_test(async_worker);
    }
  } else {
    std::cout << "unknown argument" << std::endl;
  }

  for (uint16_t i = 0; i < num_clients; ++i) {
    client[i] = nullptr;
  }
  Aws::ShutdownAPI(options);
  return 0;
}