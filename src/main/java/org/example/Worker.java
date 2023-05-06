package org.example;

import software.amazon.awssdk.services.dynamodb.model.AttributeValue;
import software.amazon.awssdk.services.dynamodb.model.GetItemRequest;

import java.util.Random;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicInteger;
import java.net.URI;
import java.util.HashMap;

public class Worker extends Thread {
    protected static Random rand = new Random();
    public static AtomicInteger count = new AtomicInteger(0);
    public static AtomicBoolean killed = new AtomicBoolean(false);
    protected final static URI endpoint = URI.create("http://127.0.0.1:8050");
    protected final static String tableName = "test.rand";

    protected GetItemRequest randRequest() {
        HashMap<String, AttributeValue> keyToGet = new HashMap<String, AttributeValue>();
        keyToGet.put("id", AttributeValue.builder().n(String.valueOf(rand.nextInt(1000))).build());
        GetItemRequest request = GetItemRequest.builder()
                .key(keyToGet)
                .tableName(tableName)
                .build();
        return request;
    }
}
