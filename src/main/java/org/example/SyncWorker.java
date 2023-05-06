package org.example;

import software.amazon.awssdk.services.dynamodb.model.DynamoDbException;
import software.amazon.awssdk.services.dynamodb.model.GetItemRequest;
import software.amazon.awssdk.services.dynamodb.model.GetItemResponse;
import software.amazon.awssdk.services.dynamodb.DynamoDbClient;

import software.amazon.awssdk.regions.Region;

public class SyncWorker extends Worker {
    private final DynamoDbClient client = DynamoDbClient.builder()
            .region(Region.AP_NORTHEAST_1).endpointOverride(endpoint)
            .build();

    public void run() {
        while (!killed.get()) {
            getRandItem();
        }
    }

    public void getRandItem() {
        GetItemRequest request = randRequest();
        try {
            GetItemResponse resp = client.getItem(request);
            if (resp.hasItem()) {
                count.incrementAndGet();
            } else {
                System.err.println("item not exists");
                killed.set(true);
            }
        } catch (DynamoDbException e) {
            System.err.println(e.getMessage());
            killed.set(true);
        }
    }
}
