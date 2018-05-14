# azure gateway

The azure-gateway is a standalone application that transports messages from the device to the azure cloud.
The messages are received via a local AMQP broker.
Sending to the cloud is done with the help of the azure client sdk.

## message format

The message content type, content encoding and content itself are transparently sent from the AMQP broker to the cloud.

The message header is translated from AMQP to the azure message header.
Header properties beginning with *gateway:* are filtered and not sent to the cloud.

Sending of the message can be configured with the header properties.
The following properties are supported.

| property           | value      | description            |
|--------------------|------------|------------------------|
| gateway:type       | properties | send as device twin    |
| gateway:type       | message    | send as normal message |
| gateway:message-id | x          | Use x as message id    |

