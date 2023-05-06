package org.example;

import software.amazon.awssdk.services.dynamodb.model.DynamoDbException;
import software.amazon.awssdk.services.dynamodb.model.GetItemRequest;
import software.amazon.awssdk.services.dynamodb.model.GetItemResponse;
import software.amazon.awssdk.services.dynamodb.DynamoDbAsyncClient;
import software.amazon.awssdk.regions.Region;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.locks.ReentrantLock;
import java.util.concurrent.locks.Condition;

public class AsyncWorker extends Worker {
    private final DynamoDbAsyncClient client = DynamoDbAsyncClient.builder()
            .region(Region.AP_NORTHEAST_1).endpointOverride(endpoint)
            .build();
    private ReentrantLock mu = new ReentrantLock();
    private Condition cv = mu.newCondition();
    private int inflight = 0;
    private final int maxInflight;

    AsyncWorker(int maxf) {
        maxInflight = maxf;
    }

    public void run() {
        while (!killed.get()) {
            mu.lock();
            inflight++;
            mu.unlock();

            GetItemRequest request = randRequest();
            CompletableFuture<GetItemResponse> fut = client.getItem(request);
            fut.whenComplete((resp, err) -> {
                try {
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
            });

            mu.lock();
            try {
                while (inflight == maxInflight) {
                    cv.await();
                }
            } catch (InterruptedException excetion) {
                Thread.currentThread().interrupt(); // set interrupted flag
            } finally {
                mu.unlock();
            }
        }
        mu.lock();
        try {
            while (inflight > 0) {
                cv.await();
            }
        } catch (InterruptedException excetion) {
            Thread.currentThread().interrupt();
        } finally {
            mu.unlock();
        }
    }
}
